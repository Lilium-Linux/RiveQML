#pragma once

#include <RiveQml/riveassetprovider.h>
#include <RiveQml/rivefile.h>
#include <RiveQml/riveinputmap.h>
#include <RiveQml/riveqmlglobal.h>
#include <RiveQml/riveviewmodeladapter.h>

#include <QPointer>
#include <QQuickPaintedItem>
#include <QSet>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <memory>

class QHoverEvent;
class QMouseEvent;
class QPainter;
class RiveRuntimeItemState;

class RIVEQML_EXPORT RiveItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(RiveFile* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString artboard READ artboard WRITE setArtboard NOTIFY artboardChanged)
    Q_PROPERTY(QString animation READ animation WRITE setAnimation NOTIFY animationChanged)
    Q_PROPERTY(QString stateMachine READ stateMachine WRITE setStateMachine NOTIFY stateMachineChanged)
    Q_PROPERTY(FitMode fitMode READ fitMode WRITE setFitMode NOTIFY fitModeChanged)
    Q_PROPERTY(AlignmentMode alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(bool autoplay READ autoplay WRITE setAutoplay NOTIFY autoplayChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY interactiveChanged)
    Q_PROPERTY(RiveAssetProvider* assetProvider READ assetProvider WRITE setAssetProvider NOTIFY assetProviderChanged)
    Q_PROPERTY(RiveInputMap* inputMap READ inputMap WRITE setInputMap NOTIFY inputMapChanged)
    Q_PROPERTY(RiveViewModelAdapter* viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)
    Q_PROPERTY(RiveFile::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    QML_ELEMENT

public:
    enum class FitMode
    {
        Fill,
        Contain,
        Cover,
        FitWidth,
        FitHeight,
        ScaleDown,
        None,
    };
    Q_ENUM(FitMode)

    enum class AlignmentMode
    {
        Center,
        TopLeft,
        Top,
        TopRight,
        Left,
        Right,
        BottomLeft,
        Bottom,
        BottomRight,
    };
    Q_ENUM(AlignmentMode)

    explicit RiveItem(QQuickItem* parent = nullptr);
    ~RiveItem() override;

    RiveFile* document() const;
    void setDocument(RiveFile* document);

    QUrl source() const;
    void setSource(const QUrl& source);

    QString artboard() const;
    void setArtboard(const QString& artboard);

    QString animation() const;
    void setAnimation(const QString& animation);

    QString stateMachine() const;
    void setStateMachine(const QString& stateMachine);

    FitMode fitMode() const;
    void setFitMode(FitMode fitMode);

    AlignmentMode alignment() const;
    void setAlignment(AlignmentMode alignment);

    bool autoplay() const;
    void setAutoplay(bool autoplay);

    bool isPaused() const;
    void setPaused(bool paused);

    qreal speed() const;
    void setSpeed(qreal speed);

    bool interactive() const;
    void setInteractive(bool interactive);

    RiveAssetProvider* assetProvider() const;
    void setAssetProvider(RiveAssetProvider* assetProvider);

    RiveInputMap* inputMap() const;
    void setInputMap(RiveInputMap* inputMap);

    RiveViewModelAdapter* viewModel() const;
    void setViewModel(RiveViewModelAdapter* viewModel);

    RiveFile::Status status() const;
    QString errorString() const;

    Q_INVOKABLE void setNumber(const QString& name, double value);
    Q_INVOKABLE void setBool(const QString& name, bool value);
    Q_INVOKABLE void fireTrigger(const QString& name);
    Q_INVOKABLE void setTextRun(const QString& name, const QString& value);
    Q_INVOKABLE void bindViewModel(RiveViewModelAdapter* adapter);
    Q_INVOKABLE void reload();

signals:
    void documentChanged();
    void sourceChanged();
    void artboardChanged();
    void animationChanged();
    void stateMachineChanged();
    void fitModeChanged();
    void alignmentChanged();
    void autoplayChanged();
    void pausedChanged();
    void speedChanged();
    void interactiveChanged();
    void assetProviderChanged();
    void inputMapChanged();
    void viewModelChanged();
    void statusChanged();
    void errorStringChanged();
    void loaded();
    void loadFailed(const QString& errorString);

protected:
    void paint(QPainter* painter) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void connectDocumentSignals(RiveFile* document);
    void disconnectDocumentSignals(RiveFile* document);
    void connectInputSignals(RiveInputMap* inputMap);
    void disconnectInputSignals(RiveInputMap* inputMap);
    void connectViewModelSignals(RiveViewModelAdapter* viewModel);
    void disconnectViewModelSignals(RiveViewModelAdapter* viewModel);
    void setRuntimeErrorString(const QString& errorString);

    RiveFile* m_internalDocument = nullptr;
    RiveInputMap* m_internalInputMap = nullptr;
    RiveViewModelAdapter* m_internalViewModel = nullptr;
    QPointer<RiveFile> m_document;
    QPointer<RiveAssetProvider> m_assetProvider;
    QPointer<RiveInputMap> m_inputMap;
    QPointer<RiveViewModelAdapter> m_viewModel;
    QString m_artboard;
    QString m_animation;
    QString m_stateMachine;
    FitMode m_fitMode = FitMode::Contain;
    AlignmentMode m_alignment = AlignmentMode::Center;
    bool m_autoplay = true;
    bool m_paused = false;
    qreal m_speed = 1.0;
    bool m_interactive = true;
    QString m_runtimeErrorString;
    QVariantMap m_cachedInputValues;
    QVariantMap m_cachedViewModelValues;
    QSet<QString> m_pendingTriggers;
    bool m_runtimeDirty = true;
    bool m_inputsDirty = true;
    bool m_viewModelDirty = true;
    std::unique_ptr<RiveRuntimeItemState> m_runtimeState;
};
