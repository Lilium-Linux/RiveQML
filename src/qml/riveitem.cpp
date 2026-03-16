#include <RiveQml/riveitem.h>

#include "core/runtimebridge.h"
#include "render/qt/qtgraphicsapi.h"

#include <QElapsedTimer>
#include <QHoverEvent>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>

#include <ApplicationServices/ApplicationServices.h>

#include <cg_renderer.hpp>
#include <rive/animation/linear_animation_instance.hpp>
#include <rive/animation/state_machine_input_instance.hpp>
#include <rive/animation/state_machine_instance.hpp>
#include <rive/layout.hpp>
#include <rive/math/mat2d.hpp>
#include <rive/scene.hpp>
#include <rive/static_scene.hpp>
#include <rive/text/text_value_run.hpp>
#include <rive/viewmodel/viewmodel_instance.hpp>
#include <rive/viewmodel/viewmodel_instance_boolean.hpp>
#include <rive/viewmodel/viewmodel_instance_color.hpp>
#include <rive/viewmodel/viewmodel_instance_enum.hpp>
#include <rive/viewmodel/viewmodel_instance_number.hpp>
#include <rive/viewmodel/viewmodel_instance_string.hpp>
#include <rive/viewmodel/viewmodel_instance_trigger.hpp>

#include <memory>

namespace
{
rive::Fit toRiveFit(RiveItem::FitMode fitMode)
{
    switch (fitMode)
    {
        case RiveItem::FitMode::Fill:
            return rive::Fit::fill;
        case RiveItem::FitMode::Contain:
            return rive::Fit::contain;
        case RiveItem::FitMode::Cover:
            return rive::Fit::cover;
        case RiveItem::FitMode::FitWidth:
            return rive::Fit::fitWidth;
        case RiveItem::FitMode::FitHeight:
            return rive::Fit::fitHeight;
        case RiveItem::FitMode::ScaleDown:
            return rive::Fit::scaleDown;
        case RiveItem::FitMode::None:
        default:
            return rive::Fit::none;
    }
}

rive::Alignment toRiveAlignment(RiveItem::AlignmentMode alignmentMode)
{
    switch (alignmentMode)
    {
        case RiveItem::AlignmentMode::TopLeft:
            return rive::Alignment::topLeft;
        case RiveItem::AlignmentMode::Top:
            return rive::Alignment::topCenter;
        case RiveItem::AlignmentMode::TopRight:
            return rive::Alignment::topRight;
        case RiveItem::AlignmentMode::Left:
            return rive::Alignment::centerLeft;
        case RiveItem::AlignmentMode::Right:
            return rive::Alignment::centerRight;
        case RiveItem::AlignmentMode::BottomLeft:
            return rive::Alignment::bottomLeft;
        case RiveItem::AlignmentMode::Bottom:
            return rive::Alignment::bottomCenter;
        case RiveItem::AlignmentMode::BottomRight:
            return rive::Alignment::bottomRight;
        case RiveItem::AlignmentMode::Center:
        default:
            return rive::Alignment::center;
    }
}

QString selectedArtboardName(const RiveFile* document, const QString& requestedArtboard)
{
    if (!requestedArtboard.isEmpty())
    {
        return requestedArtboard;
    }

    const QStringList artboards = document == nullptr ? QStringList() : document->artboards();
    return artboards.isEmpty() ? QString() : artboards.first();
}

class ScopedCGColorSpace
{
public:
    ScopedCGColorSpace() : m_space(CGColorSpaceCreateDeviceRGB()) {}
    ~ScopedCGColorSpace()
    {
        if (m_space != nullptr)
        {
            CGColorSpaceRelease(m_space);
        }
    }

    operator CGColorSpaceRef() const { return m_space; }

private:
    CGColorSpaceRef m_space = nullptr;
};
}

class RiveRuntimeItemState
{
public:
    enum class SceneKind
    {
        Static,
        Animation,
        StateMachine,
    };

    void reset()
    {
        m_document.reset();
        m_artboard.reset();
        m_scene.reset();
        m_viewModelInstance = nullptr;
        m_sceneKind = SceneKind::Static;
        m_artboardName.clear();
        m_animationName.clear();
        m_stateMachineName.clear();
        m_running = false;
        m_clockStarted = false;
        m_lastFrameNsecs = 0;
    }

    void setRunning(bool running)
    {
        m_running = running;
        if (!m_running)
        {
            m_clockStarted = false;
            m_lastFrameNsecs = 0;
        }
    }

    bool ensureScene(RiveFile* document,
                     const QString& artboardName,
                     const QString& animationName,
                     const QString& stateMachineName,
                     const QVariantMap& viewModelValues,
                     QString* errorString)
    {
        if (document == nullptr || document->status() != RiveFile::Status::Ready)
        {
            reset();
            return false;
        }

        auto runtimeDocument = document->runtimeBridge()->document();
        if (!runtimeDocument || runtimeDocument->file == nullptr)
        {
            reset();
            if (errorString != nullptr)
            {
                *errorString = QStringLiteral("Rive runtime document is not available.");
            }
            return false;
        }

        const bool sceneChanged = runtimeDocument != m_document ||
                                  artboardName != m_artboardName ||
                                  animationName != m_animationName ||
                                  stateMachineName != m_stateMachineName;
        if (!sceneChanged)
        {
            applyViewModelValues(viewModelValues);
            return true;
        }

        reset();
        m_document = std::move(runtimeDocument);
        m_artboardName = artboardName;
        m_animationName = animationName;
        m_stateMachineName = stateMachineName;

        if (!m_artboardName.isEmpty())
        {
            m_artboard = m_document->file->artboardNamed(m_artboardName.toStdString());
        }
        else
        {
            m_artboard = m_document->file->artboardDefault();
        }

        if (!m_artboard)
        {
            if (errorString != nullptr)
            {
                *errorString = m_artboardName.isEmpty()
                                   ? QStringLiteral("The Rive file does not contain a default artboard.")
                                   : QStringLiteral("Artboard \"%1\" was not found.")
                                         .arg(m_artboardName);
            }
            return false;
        }

        if (!m_stateMachineName.isEmpty())
        {
            m_scene = m_artboard->stateMachineNamed(m_stateMachineName.toStdString());
            m_sceneKind = SceneKind::StateMachine;
        }
        else if (!m_animationName.isEmpty())
        {
            m_scene = m_artboard->animationNamed(m_animationName.toStdString());
            m_sceneKind = SceneKind::Animation;
        }
        else
        {
            const int defaultStateMachineIndex = m_artboard->defaultStateMachineIndex();
            if (defaultStateMachineIndex >= 0)
            {
                m_scene = m_artboard->stateMachineAt(static_cast<size_t>(defaultStateMachineIndex));
                m_sceneKind = SceneKind::StateMachine;
            }
            else if (m_artboard->stateMachineCount() > 0)
            {
                m_scene = m_artboard->stateMachineAt(0);
                m_sceneKind = SceneKind::StateMachine;
            }
            else if (m_artboard->animationCount() > 0)
            {
                m_scene = m_artboard->animationAt(0);
                m_sceneKind = SceneKind::Animation;
            }
            else
            {
                m_scene = std::make_unique<rive::StaticScene>(m_artboard.get());
                m_sceneKind = SceneKind::Static;
            }
        }

        if (!m_scene)
        {
            m_scene = std::make_unique<rive::StaticScene>(m_artboard.get());
            m_sceneKind = SceneKind::Static;
        }

        m_viewModelInstance = m_document->file->createDefaultViewModelInstance(m_artboard.get());
        if (m_viewModelInstance != nullptr)
        {
            m_scene->bindViewModelInstance(m_viewModelInstance);
            applyViewModelValues(viewModelValues);
        }

        return true;
    }

    void applyInputValues(const QVariantMap& values, QSet<QString>* pendingTriggers)
    {
        if (m_scene == nullptr)
        {
            return;
        }

        for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        {
            const std::string name = it.key().toStdString();
            const QVariant& value = it.value();

            if (auto* number = m_scene->getNumber(name))
            {
                number->value(static_cast<float>(value.toDouble()));
                continue;
            }

            if (auto* boolean = m_scene->getBool(name))
            {
                boolean->value(value.toBool());
                continue;
            }

            if (value.metaType().id() == QMetaType::QString && m_artboard != nullptr)
            {
                if (auto* textRun = m_artboard->getTextRun(name, std::string()))
                {
                    textRun->text(value.toString().toStdString());
                }
            }
        }

        if (pendingTriggers == nullptr)
        {
            return;
        }

        for (const QString& triggerName : std::as_const(*pendingTriggers))
        {
            if (auto* trigger = m_scene->getTrigger(triggerName.toStdString()))
            {
                trigger->fire();
            }
        }

        pendingTriggers->clear();
    }

    void applyViewModelValues(const QVariantMap& values)
    {
        if (m_viewModelInstance == nullptr)
        {
            return;
        }

        for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        {
            auto* property = m_viewModelInstance->propertyValue(it.key().toStdString());
            if (property == nullptr)
            {
                continue;
            }

            const QVariant& value = it.value();
            if (property->is<rive::ViewModelInstanceNumber>())
            {
                property->as<rive::ViewModelInstanceNumber>()->propertyValue(
                    static_cast<float>(value.toDouble()));
                continue;
            }

            if (property->is<rive::ViewModelInstanceBoolean>())
            {
                property->as<rive::ViewModelInstanceBoolean>()->propertyValue(value.toBool());
                continue;
            }

            if (property->is<rive::ViewModelInstanceString>())
            {
                property->as<rive::ViewModelInstanceString>()->propertyValue(
                    value.toString().toStdString());
                continue;
            }

            if (property->is<rive::ViewModelInstanceTrigger>())
            {
                if (value.toBool())
                {
                    property->as<rive::ViewModelInstanceTrigger>()->trigger();
                }
                continue;
            }

            if (property->is<rive::ViewModelInstanceEnum>())
            {
                property->as<rive::ViewModelInstanceEnum>()->propertyValue(
                    static_cast<uint32_t>(value.toUInt()));
                continue;
            }

            if (property->is<rive::ViewModelInstanceColor>())
            {
                const QColor color = value.value<QColor>();
                property->as<rive::ViewModelInstanceColor>()->propertyValue(color.rgba());
            }
        }
    }

    bool advance(bool autoplay,
                 bool paused,
                 qreal speed,
                 bool forceAdvance,
                 bool* keepAnimating)
    {
        if (m_scene == nullptr)
        {
            if (keepAnimating != nullptr)
            {
                *keepAnimating = false;
            }
            return false;
        }

        float deltaSeconds = 0.0f;
        if ((m_running || autoplay) && !paused)
        {
            const qint64 now = currentNsecs();
            if (m_lastFrameNsecs != 0)
            {
                deltaSeconds = static_cast<float>(now - m_lastFrameNsecs) / 1000000000.0f;
                deltaSeconds *= static_cast<float>(speed);
            }
            m_lastFrameNsecs = now;
            m_clockStarted = true;
        }
        else if (!m_clockStarted)
        {
            m_lastFrameNsecs = currentNsecs();
            m_clockStarted = true;
        }

        bool sceneRequestsMoreFrames = false;
        if (forceAdvance || deltaSeconds > 0.0f)
        {
            sceneRequestsMoreFrames = m_scene->advanceAndApply(deltaSeconds);
        }

        switch (m_sceneKind)
        {
            case SceneKind::Animation:
                m_running = autoplay && !paused;
                if (!sceneRequestsMoreFrames)
                {
                    m_running = false;
                }
                break;
            case SceneKind::StateMachine:
                if (sceneRequestsMoreFrames)
                {
                    m_running = !paused;
                }
                else if (!autoplay)
                {
                    m_running = false;
                }
                break;
            case SceneKind::Static:
            default:
                m_running = false;
                sceneRequestsMoreFrames = false;
                break;
        }

        if (keepAnimating != nullptr)
        {
            *keepAnimating = sceneRequestsMoreFrames && !paused;
        }
        return true;
    }

    bool render(QImage* image,
                const QSize& pixelSize,
                qreal devicePixelRatio,
                RiveItem::FitMode fitMode,
                RiveItem::AlignmentMode alignmentMode,
                QString* errorString)
    {
        if (m_scene == nullptr || pixelSize.isEmpty() || image == nullptr)
        {
            return false;
        }

        if (image->size() != pixelSize ||
            image->format() != QImage::Format_RGBA8888_Premultiplied)
        {
            *image = QImage(pixelSize, QImage::Format_RGBA8888_Premultiplied);
        }
        image->setDevicePixelRatio(devicePixelRatio);
        image->fill(Qt::transparent);

        ScopedCGColorSpace colorSpace;
        const auto bitmapInfo = static_cast<uint32_t>(kCGBitmapByteOrder32Big) |
                                static_cast<uint32_t>(kCGImageAlphaPremultipliedLast);
        CGContextRef context = CGBitmapContextCreate(image->bits(),
                                                     static_cast<size_t>(pixelSize.width()),
                                                     static_cast<size_t>(pixelSize.height()),
                                                     8,
                                                     static_cast<size_t>(image->bytesPerLine()),
                                                     colorSpace,
                                                     bitmapInfo);
        if (context == nullptr)
        {
            if (errorString != nullptr)
            {
                *errorString = QStringLiteral("Failed to create a CoreGraphics render context.");
            }
            return false;
        }

        rive::CGRenderer renderer(context, pixelSize.width(), pixelSize.height());
        renderer.align(
            toRiveFit(fitMode),
            toRiveAlignment(alignmentMode),
            rive::AABB{0.0f, 0.0f, static_cast<float>(pixelSize.width()), static_cast<float>(pixelSize.height())},
            m_scene->bounds());
        m_scene->draw(&renderer);

        CGContextFlush(context);
        CGContextRelease(context);
        return true;
    }

    bool handlePointerPress(const QPointF& itemPoint,
                            const QSizeF& itemSize,
                            RiveItem::FitMode fitMode,
                            RiveItem::AlignmentMode alignmentMode)
    {
        return dispatchPointer(itemPoint, itemSize, fitMode, alignmentMode,
                               [this](const rive::Vec2D& point) { m_scene->pointerDown(point); });
    }

    bool handlePointerMove(const QPointF& itemPoint,
                           const QSizeF& itemSize,
                           RiveItem::FitMode fitMode,
                           RiveItem::AlignmentMode alignmentMode)
    {
        return dispatchPointer(itemPoint, itemSize, fitMode, alignmentMode,
                               [this](const rive::Vec2D& point) { m_scene->pointerMove(point); });
    }

    bool handlePointerRelease(const QPointF& itemPoint,
                              const QSizeF& itemSize,
                              RiveItem::FitMode fitMode,
                              RiveItem::AlignmentMode alignmentMode)
    {
        return dispatchPointer(itemPoint, itemSize, fitMode, alignmentMode,
                               [this](const rive::Vec2D& point) { m_scene->pointerUp(point); });
    }

    bool handlePointerExit(const QPointF& itemPoint,
                           const QSizeF& itemSize,
                           RiveItem::FitMode fitMode,
                           RiveItem::AlignmentMode alignmentMode)
    {
        return dispatchPointer(itemPoint, itemSize, fitMode, alignmentMode,
                               [this](const rive::Vec2D& point) { m_scene->pointerExit(point); });
    }

private:
    qint64 currentNsecs()
    {
        if (!m_clockStarted)
        {
            m_frameClock.start();
            m_clockStarted = true;
        }

        return m_frameClock.nsecsElapsed();
    }

    template <typename Handler>
    bool dispatchPointer(const QPointF& itemPoint,
                         const QSizeF& itemSize,
                         RiveItem::FitMode fitMode,
                         RiveItem::AlignmentMode alignmentMode,
                         Handler&& handler)
    {
        if (m_scene == nullptr || m_sceneKind != SceneKind::StateMachine || itemSize.isEmpty())
        {
            return false;
        }

        const rive::Mat2D transform = rive::computeAlignment(
            toRiveFit(fitMode),
            toRiveAlignment(alignmentMode),
            rive::AABB{0.0f, 0.0f, static_cast<float>(itemSize.width()), static_cast<float>(itemSize.height())},
            m_scene->bounds());
        const rive::Mat2D inverse = transform.invertOrIdentity();
        const rive::Vec2D point = inverse * rive::Vec2D{static_cast<float>(itemPoint.x()),
                                                        static_cast<float>(itemPoint.y())};
        handler(point);
        return true;
    }

    std::shared_ptr<RuntimeDocument> m_document;
    std::unique_ptr<rive::ArtboardInstance> m_artboard;
    std::unique_ptr<rive::Scene> m_scene;
    rive::rcp<rive::ViewModelInstance> m_viewModelInstance = nullptr;
    SceneKind m_sceneKind = SceneKind::Static;
    QString m_artboardName;
    QString m_animationName;
    QString m_stateMachineName;
    bool m_running = false;
    bool m_clockStarted = false;
    qint64 m_lastFrameNsecs = 0;
    QElapsedTimer m_frameClock;
};

RiveItem::RiveItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
    setOpaquePainting(false);

    m_internalDocument = new RiveFile(this);
    m_internalInputMap = new RiveInputMap(this);
    m_internalViewModel = new RiveViewModelAdapter(this);

    m_document = m_internalDocument;
    m_inputMap = m_internalInputMap;
    m_viewModel = m_internalViewModel;
    m_runtimeState = std::make_unique<RiveRuntimeItemState>();
    m_cachedInputValues = m_internalInputMap->values();
    m_cachedViewModelValues = m_internalViewModel->values();

    connectDocumentSignals(m_document);
    connectInputSignals(m_inputMap);
    connectViewModelSignals(m_viewModel);
}

RiveItem::~RiveItem() = default;

RiveFile* RiveItem::document() const
{
    return m_document;
}

void RiveItem::setDocument(RiveFile* document)
{
    if (document == nullptr)
    {
        document = m_internalDocument;
    }

    if (m_document == document)
    {
        return;
    }

    disconnectDocumentSignals(m_document);
    m_document = document;
    connectDocumentSignals(m_document);

    if (m_document == m_internalDocument && m_assetProvider != nullptr)
    {
        m_internalDocument->setAssetRoot(m_assetProvider->assetRoot());
    }

    m_runtimeDirty = true;
    setRuntimeErrorString({});
    emit documentChanged();
    emit sourceChanged();
    emit statusChanged();
    emit errorStringChanged();
    update();
}

QUrl RiveItem::source() const
{
    return m_document == nullptr ? QUrl() : m_document->source();
}

void RiveItem::setSource(const QUrl& source)
{
    if (m_document == nullptr || m_document->source() == source)
    {
        return;
    }

    m_runtimeDirty = true;
    setRuntimeErrorString({});
    m_document->setSource(source);
}

QString RiveItem::artboard() const
{
    return m_artboard;
}

void RiveItem::setArtboard(const QString& artboard)
{
    if (m_artboard == artboard)
    {
        return;
    }

    m_artboard = artboard;
    m_runtimeDirty = true;
    setRuntimeErrorString({});
    emit artboardChanged();
    update();
}

QString RiveItem::animation() const
{
    return m_animation;
}

void RiveItem::setAnimation(const QString& animation)
{
    if (m_animation == animation)
    {
        return;
    }

    m_animation = animation;
    m_runtimeDirty = true;
    setRuntimeErrorString({});
    emit animationChanged();
    update();
}

QString RiveItem::stateMachine() const
{
    return m_stateMachine;
}

void RiveItem::setStateMachine(const QString& stateMachine)
{
    if (m_stateMachine == stateMachine)
    {
        return;
    }

    m_stateMachine = stateMachine;
    m_runtimeDirty = true;
    setRuntimeErrorString({});
    emit stateMachineChanged();
    update();
}

RiveItem::FitMode RiveItem::fitMode() const
{
    return m_fitMode;
}

void RiveItem::setFitMode(FitMode fitMode)
{
    if (m_fitMode == fitMode)
    {
        return;
    }

    m_fitMode = fitMode;
    emit fitModeChanged();
    update();
}

RiveItem::AlignmentMode RiveItem::alignment() const
{
    return m_alignment;
}

void RiveItem::setAlignment(AlignmentMode alignment)
{
    if (m_alignment == alignment)
    {
        return;
    }

    m_alignment = alignment;
    emit alignmentChanged();
    update();
}

bool RiveItem::autoplay() const
{
    return m_autoplay;
}

void RiveItem::setAutoplay(bool autoplay)
{
    if (m_autoplay == autoplay)
    {
        return;
    }

    m_autoplay = autoplay;
    if (m_runtimeState != nullptr)
    {
        m_runtimeState->setRunning(m_autoplay && !m_paused);
    }
    emit autoplayChanged();
    update();
}

bool RiveItem::isPaused() const
{
    return m_paused;
}

void RiveItem::setPaused(bool paused)
{
    if (m_paused == paused)
    {
        return;
    }

    m_paused = paused;
    if (m_runtimeState != nullptr)
    {
        m_runtimeState->setRunning(!m_paused && m_autoplay);
    }
    emit pausedChanged();
    update();
}

qreal RiveItem::speed() const
{
    return m_speed;
}

void RiveItem::setSpeed(qreal speed)
{
    if (qFuzzyCompare(m_speed, speed))
    {
        return;
    }

    m_speed = speed;
    emit speedChanged();
    update();
}

bool RiveItem::interactive() const
{
    return m_interactive;
}

void RiveItem::setInteractive(bool interactive)
{
    if (m_interactive == interactive)
    {
        return;
    }

    m_interactive = interactive;
    emit interactiveChanged();
}

RiveAssetProvider* RiveItem::assetProvider() const
{
    return m_assetProvider;
}

void RiveItem::setAssetProvider(RiveAssetProvider* assetProvider)
{
    if (m_assetProvider == assetProvider)
    {
        return;
    }

    m_assetProvider = assetProvider;
    if (m_document == m_internalDocument)
    {
        m_internalDocument->setAssetRoot(
            m_assetProvider == nullptr ? QUrl() : m_assetProvider->assetRoot());
        if (!source().isEmpty())
        {
            m_internalDocument->reload();
        }
    }

    m_runtimeDirty = true;
    emit assetProviderChanged();
    update();
}

RiveInputMap* RiveItem::inputMap() const
{
    return m_inputMap;
}

void RiveItem::setInputMap(RiveInputMap* inputMap)
{
    if (inputMap == nullptr)
    {
        inputMap = m_internalInputMap;
    }

    if (m_inputMap == inputMap)
    {
        return;
    }

    disconnectInputSignals(m_inputMap);
    m_inputMap = inputMap;
    connectInputSignals(m_inputMap);
    m_cachedInputValues = m_inputMap == nullptr ? QVariantMap() : m_inputMap->values();
    m_inputsDirty = true;
    emit inputMapChanged();
    update();
}

RiveViewModelAdapter* RiveItem::viewModel() const
{
    return m_viewModel;
}

void RiveItem::setViewModel(RiveViewModelAdapter* viewModel)
{
    if (viewModel == nullptr)
    {
        viewModel = m_internalViewModel;
    }

    if (m_viewModel == viewModel)
    {
        return;
    }

    disconnectViewModelSignals(m_viewModel);
    m_viewModel = viewModel;
    connectViewModelSignals(m_viewModel);
    m_cachedViewModelValues = m_viewModel == nullptr ? QVariantMap() : m_viewModel->values();
    m_viewModelDirty = true;
    emit viewModelChanged();
    update();
}

RiveFile::Status RiveItem::status() const
{
    return m_document == nullptr ? RiveFile::Status::Null : m_document->status();
}

QString RiveItem::errorString() const
{
    if (!m_runtimeErrorString.isEmpty())
    {
        return m_runtimeErrorString;
    }

    return m_document == nullptr ? QString() : m_document->errorString();
}

void RiveItem::setNumber(const QString& name, double value)
{
    if (m_inputMap != nullptr)
    {
        m_inputMap->setNumber(name, value);
    }
}

void RiveItem::setBool(const QString& name, bool value)
{
    if (m_inputMap != nullptr)
    {
        m_inputMap->setBool(name, value);
    }
}

void RiveItem::fireTrigger(const QString& name)
{
    if (m_inputMap != nullptr)
    {
        m_inputMap->fireTrigger(name);
    }
}

void RiveItem::setTextRun(const QString& name, const QString& value)
{
    if (m_inputMap != nullptr)
    {
        m_inputMap->setText(name, value);
    }
}

void RiveItem::bindViewModel(RiveViewModelAdapter* adapter)
{
    setViewModel(adapter);
}

void RiveItem::reload()
{
    m_runtimeDirty = true;
    setRuntimeErrorString({});
    if (m_document != nullptr)
    {
        m_document->reload();
    }
}

void RiveItem::paint(QPainter* painter)
{
    if (painter == nullptr || boundingRect().isEmpty() || m_document == nullptr)
    {
        return;
    }

    if (window() != nullptr)
    {
        m_document->setGraphicsApi(QtGraphicsApi::fromWindow(window()));
    }

    const QString effectiveArtboard = selectedArtboardName(m_document, m_artboard);
    QString runtimeError;
    if (!m_runtimeState->ensureScene(
            m_document, effectiveArtboard, m_animation, m_stateMachine, m_cachedViewModelValues, &runtimeError))
    {
        setRuntimeErrorString(runtimeError);
        return;
    }

    if (m_runtimeDirty)
    {
        m_runtimeState->setRunning(m_autoplay && !m_paused);
    }

    if (m_inputsDirty || m_runtimeDirty)
    {
        m_runtimeState->applyInputValues(m_cachedInputValues, &m_pendingTriggers);
    }

    if (m_viewModelDirty || m_runtimeDirty)
    {
        m_runtimeState->applyViewModelValues(m_cachedViewModelValues);
    }

    bool keepAnimating = false;
    m_runtimeState->advance(m_autoplay,
                            m_paused,
                            m_speed,
                            m_runtimeDirty || m_inputsDirty || m_viewModelDirty,
                            &keepAnimating);

    const qreal devicePixelRatio =
        painter->device() == nullptr ? 1.0 : painter->device()->devicePixelRatioF();
    const QSize pixelSize = (boundingRect().size() * devicePixelRatio).toSize();

    QImage frame;
    if (!m_runtimeState->render(
            &frame, pixelSize, devicePixelRatio, m_fitMode, m_alignment, &runtimeError))
    {
        setRuntimeErrorString(runtimeError);
        return;
    }

    setRuntimeErrorString({});
    painter->drawImage(boundingRect(), frame);

    m_runtimeDirty = false;
    m_inputsDirty = false;
    m_viewModelDirty = false;

    if (keepAnimating && !m_paused)
    {
        QMetaObject::invokeMethod(
            this, [this]() { update(); }, Qt::QueuedConnection);
    }
}

void RiveItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    update();
}

void RiveItem::hoverMoveEvent(QHoverEvent* event)
{
    if (m_interactive && m_runtimeState != nullptr &&
        m_runtimeState->handlePointerMove(event->position(), size(), m_fitMode, m_alignment))
    {
        update();
    }

    QQuickPaintedItem::hoverMoveEvent(event);
}

void RiveItem::hoverLeaveEvent(QHoverEvent* event)
{
    if (m_interactive && m_runtimeState != nullptr &&
        m_runtimeState->handlePointerExit(QPointF(-1.0, -1.0), size(), m_fitMode, m_alignment))
    {
        update();
    }

    QQuickPaintedItem::hoverLeaveEvent(event);
}

void RiveItem::mousePressEvent(QMouseEvent* event)
{
    if (m_interactive && m_runtimeState != nullptr &&
        m_runtimeState->handlePointerPress(event->position(), size(), m_fitMode, m_alignment))
    {
        update();
    }

    QQuickPaintedItem::mousePressEvent(event);
}

void RiveItem::mouseMoveEvent(QMouseEvent* event)
{
    if (m_interactive && m_runtimeState != nullptr &&
        m_runtimeState->handlePointerMove(event->position(), size(), m_fitMode, m_alignment))
    {
        update();
    }

    QQuickPaintedItem::mouseMoveEvent(event);
}

void RiveItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_interactive && m_runtimeState != nullptr &&
        m_runtimeState->handlePointerRelease(event->position(), size(), m_fitMode, m_alignment))
    {
        update();
    }

    QQuickPaintedItem::mouseReleaseEvent(event);
}

void RiveItem::connectDocumentSignals(RiveFile* document)
{
    if (document == nullptr)
    {
        return;
    }

    connect(document, &RiveFile::sourceChanged, this, [this]() {
        m_runtimeDirty = true;
        emit sourceChanged();
        update();
    });
    connect(document, &RiveFile::statusChanged, this, [this]() {
        m_runtimeDirty = true;
        emit statusChanged();
        if (status() == RiveFile::Status::Ready)
        {
            emit loaded();
        }
        if (status() == RiveFile::Status::Error)
        {
            emit loadFailed(errorString());
        }
        update();
    });
    connect(document, &RiveFile::errorStringChanged, this, [this]() {
        emit errorStringChanged();
        update();
    });
}

void RiveItem::disconnectDocumentSignals(RiveFile* document)
{
    if (document != nullptr)
    {
        disconnect(document, nullptr, this, nullptr);
    }
}

void RiveItem::connectInputSignals(RiveInputMap* inputMap)
{
    if (inputMap == nullptr)
    {
        return;
    }

    connect(inputMap, &RiveInputMap::valuesChanged, this, [this, inputMap]() {
        m_cachedInputValues = inputMap->values();
        m_inputsDirty = true;
        update();
    });
    connect(inputMap, &RiveInputMap::triggerFired, this, [this](const QString& name) {
        m_pendingTriggers.insert(name);
        m_inputsDirty = true;
        update();
    });
}

void RiveItem::disconnectInputSignals(RiveInputMap* inputMap)
{
    if (inputMap != nullptr)
    {
        disconnect(inputMap, nullptr, this, nullptr);
    }
}

void RiveItem::connectViewModelSignals(RiveViewModelAdapter* viewModel)
{
    if (viewModel == nullptr)
    {
        return;
    }

    connect(viewModel, &RiveViewModelAdapter::valuesChanged, this, [this, viewModel]() {
        m_cachedViewModelValues = viewModel->values();
        m_viewModelDirty = true;
        update();
    });
}

void RiveItem::disconnectViewModelSignals(RiveViewModelAdapter* viewModel)
{
    if (viewModel != nullptr)
    {
        disconnect(viewModel, nullptr, this, nullptr);
    }
}

void RiveItem::setRuntimeErrorString(const QString& errorString)
{
    if (m_runtimeErrorString == errorString)
    {
        return;
    }

    m_runtimeErrorString = errorString;
    emit errorStringChanged();

    if (!m_runtimeErrorString.isEmpty())
    {
        emit loadFailed(m_runtimeErrorString);
    }
}
