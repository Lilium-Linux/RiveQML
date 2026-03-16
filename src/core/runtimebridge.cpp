#include "runtimebridge.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>

#include <cstring>

#include <rive/artboard.hpp>
#include <rive/assets/file_asset.hpp>
#include <rive/file_asset_loader.hpp>
#include <rive/simple_array.hpp>
#include <rive/span.hpp>

#if defined(RIVEQML_USE_CG_RENDERER)
#include <cg_factory.hpp>
#elif defined(RIVEQML_USE_SKIA_RENDERER)
#include <skia_factory.hpp>
#else
#include <utils/no_op_factory.hpp>
#endif

namespace
{
QString urlToFileOrQrcPath(const QUrl& url)
{
    if (url.isEmpty())
    {
        return {};
    }

    if (url.isLocalFile())
    {
        return url.toLocalFile();
    }

    const QString text = url.toString();

    if (text.startsWith(QStringLiteral("qrc:/")))
    {
        return QStringLiteral(":") + text.mid(4);
    }

    if (text.startsWith(QStringLiteral(":/")))
    {
        return text;
    }

    if (url.scheme() == QStringLiteral("qrc"))
    {
        return QStringLiteral(":") + url.path();
    }

    return {};
}

QString directoryPathFor(const QUrl& url)
{
    const QString path = urlToFileOrQrcPath(url);
    if (path.isEmpty())
    {
        return {};
    }

    const QFileInfo info(path);
    if (info.isDir())
    {
        return info.absoluteFilePath();
    }

    return info.absolutePath();
}

QString importResultToString(rive::ImportResult result)
{
    switch (result)
    {
        case rive::ImportResult::success:
            return QStringLiteral("success");
        case rive::ImportResult::unsupportedVersion:
            return QStringLiteral("unsupportedVersion");
        case rive::ImportResult::malformed:
            return QStringLiteral("malformed");
    }

    return QStringLiteral("unknown");
}

class QtAssetLoader final : public rive::FileAssetLoader
{
public:
    explicit QtAssetLoader(QString assetRoot) : m_assetRoot(std::move(assetRoot)) {}

    bool loadContents(rive::FileAsset& asset,
                      rive::Span<const uint8_t> inBandBytes,
                      rive::Factory* factory) override
    {
        Q_UNUSED(inBandBytes);

        if (m_assetRoot.isEmpty())
        {
            return false;
        }

        const QString assetPath = QDir(m_assetRoot).filePath(
            QString::fromStdString(asset.uniqueFilename()));
        QFile file(assetPath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        const QByteArray bytes = file.readAll();
        rive::SimpleArray<uint8_t> decoded(static_cast<size_t>(bytes.size()));
        std::memcpy(decoded.data(), bytes.constData(), static_cast<size_t>(bytes.size()));
        return asset.decode(decoded, factory);
    }

private:
    QString m_assetRoot;
};
}

RuntimeBridge::RuntimeBridge(QObject* parent) : QObject(parent) {}

void RuntimeBridge::setSource(const QUrl& source)
{
    m_source = source;
}

void RuntimeBridge::setAssetRoot(const QUrl& assetRoot)
{
    m_assetRoot = assetRoot;
}

void RuntimeBridge::setGraphicsApi(RiveFile::GraphicsApi graphicsApi)
{
    m_graphicsApi = graphicsApi;
}

void RuntimeBridge::load()
{
    emit loadingStarted();
    m_document.reset();

    if (m_source.isEmpty())
    {
        emit loadingFailed(QStringLiteral("Rive source is empty."));
        return;
    }

    const QString sourcePath = urlToFileOrQrcPath(m_source);
    if (sourcePath.isEmpty())
    {
        emit loadingFailed(QStringLiteral("RiveQml supports only local file and qrc sources."));
        return;
    }

    QFile file(sourcePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit loadingFailed(QStringLiteral("Failed to open %1: %2")
                               .arg(sourcePath, file.errorString()));
        return;
    }

    auto document = std::make_shared<RuntimeDocument>();
    document->bytes = file.readAll();
#if defined(RIVEQML_USE_CG_RENDERER)
    document->factory = std::make_shared<rive::CGFactory>();
#elif defined(RIVEQML_USE_SKIA_RENDERER)
    document->factory = std::make_shared<rive::SkiaFactory>();
#else
    document->factory = std::make_shared<rive::NoOpFactory>();
#endif

    const QString assetRoot = m_assetRoot.isEmpty() ? directoryPathFor(m_source)
                                                    : directoryPathFor(m_assetRoot);
    auto assetLoader = std::make_unique<QtAssetLoader>(assetRoot);

    rive::ImportResult result = rive::ImportResult::malformed;
    document->file = rive::File::import(
        rive::Span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(document->bytes.constData()),
            static_cast<size_t>(document->bytes.size())),
        document->factory.get(),
        &result,
        assetLoader.get());

    if (result != rive::ImportResult::success || document->file == nullptr)
    {
        emit loadingFailed(QStringLiteral("Failed to import %1 (%2).")
                               .arg(sourcePath, importResultToString(result)));
        return;
    }

    QSet<QString> animationNames;
    QSet<QString> stateMachineNames;

    const size_t artboardCount = document->file->artboardCount();
    for (size_t artboardIndex = 0; artboardIndex < artboardCount; ++artboardIndex)
    {
        document->artboards.append(
            QString::fromStdString(document->file->artboardNameAt(artboardIndex)));

        auto* artboard = document->file->artboard(artboardIndex);
        if (artboard == nullptr)
        {
            continue;
        }

        for (size_t animationIndex = 0; animationIndex < artboard->animationCount();
             ++animationIndex)
        {
            animationNames.insert(
                QString::fromStdString(artboard->animationNameAt(animationIndex)));
        }

        for (size_t stateMachineIndex = 0;
             stateMachineIndex < artboard->stateMachineCount();
             ++stateMachineIndex)
        {
            stateMachineNames.insert(
                QString::fromStdString(artboard->stateMachineNameAt(stateMachineIndex)));
        }
    }

    document->animations = animationNames.values();
    document->stateMachines = stateMachineNames.values();
    document->animations.sort();
    document->stateMachines.sort();

    m_document = std::move(document);
    emit loadingFinished(m_document->artboards, m_document->animations, m_document->stateMachines);
}

RiveFile::GraphicsApi RuntimeBridge::graphicsApi() const
{
    return m_graphicsApi;
}

std::shared_ptr<RuntimeDocument> RuntimeBridge::document() const
{
    return m_document;
}
