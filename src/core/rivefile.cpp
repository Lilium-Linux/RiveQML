#include <RiveQml/rivefile.h>

#include "runtimebridge.h"

#include "render/qt/qtgraphicsapi.h"

#include <utility>

RiveFile::RiveFile(QObject* parent) : QObject(parent), m_runtimeBridge(std::make_unique<RuntimeBridge>())
{
    connect(m_runtimeBridge.get(), &RuntimeBridge::loadingStarted, this, [this]() {
        updateStatus(Status::Loading);
    });

    connect(m_runtimeBridge.get(),
            &RuntimeBridge::loadingFinished,
            this,
            [this](const QStringList& artboards,
                   const QStringList& animations,
                   const QStringList& stateMachines) {
                m_artboards = artboards;
                m_animations = animations;
                m_stateMachines = stateMachines;
                emit artboardsChanged();
                emit animationsChanged();
                emit stateMachinesChanged();
                updateStatus(Status::Ready);
            });

    connect(m_runtimeBridge.get(), &RuntimeBridge::loadingFailed, this, [this](const QString& errorString) {
        resetMetadata();
        updateStatus(Status::Error, errorString);
    });
}

RiveFile::~RiveFile() = default;

QUrl RiveFile::source() const
{
    return m_source;
}

void RiveFile::setSource(const QUrl& source)
{
    if (m_source == source)
    {
        return;
    }

    m_source = source;
    emit sourceChanged();

    m_runtimeBridge->setSource(source);

    if (m_source.isEmpty())
    {
        resetMetadata();
        updateStatus(Status::Null);
        return;
    }

    m_runtimeBridge->load();
}

RiveFile::Status RiveFile::status() const
{
    return m_status;
}

RiveFile::GraphicsApi RiveFile::graphicsApi() const
{
    return m_graphicsApi;
}

QString RiveFile::errorString() const
{
    return m_errorString;
}

QStringList RiveFile::artboards() const
{
    return m_artboards;
}

QStringList RiveFile::animations() const
{
    return m_animations;
}

QStringList RiveFile::stateMachines() const
{
    return m_stateMachines;
}

void RiveFile::setGraphicsApi(GraphicsApi api)
{
    if (m_graphicsApi == api)
    {
        return;
    }

    m_graphicsApi = api;
    m_runtimeBridge->setGraphicsApi(api);
    emit graphicsApiChanged();
}

void RiveFile::reload()
{
    if (m_source.isEmpty())
    {
        resetMetadata();
        updateStatus(Status::Null);
        return;
    }

    m_runtimeBridge->setSource(m_source);
    m_runtimeBridge->load();
}

QString RiveFile::graphicsApiName() const
{
    return QtGraphicsApi::toString(m_graphicsApi);
}

void RiveFile::resetMetadata()
{
    const bool hadArtboards = !m_artboards.isEmpty();
    const bool hadAnimations = !m_animations.isEmpty();
    const bool hadStateMachines = !m_stateMachines.isEmpty();

    m_artboards.clear();
    m_animations.clear();
    m_stateMachines.clear();

    if (hadArtboards)
    {
        emit artboardsChanged();
    }

    if (hadAnimations)
    {
        emit animationsChanged();
    }

    if (hadStateMachines)
    {
        emit stateMachinesChanged();
    }
}

void RiveFile::updateStatus(Status status, QString errorString)
{
    const bool statusDidChange = m_status != status;
    const bool errorChanged = m_errorString != errorString;

    m_status = status;
    m_errorString = std::move(errorString);

    if (statusDidChange)
    {
        emit statusChanged();
    }

    if (errorChanged)
    {
        emit errorStringChanged();
    }
}

RuntimeBridge* RiveFile::runtimeBridge() const
{
    return m_runtimeBridge.get();
}

void RiveFile::setAssetRoot(const QUrl& assetRoot)
{
    m_runtimeBridge->setAssetRoot(assetRoot);
}
