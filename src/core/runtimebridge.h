#pragma once

#include <RiveQml/rivefile.h>

#include <QByteArray>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include <memory>

#include <cg_factory.hpp>
#include <rive/file.hpp>

struct RuntimeDocument
{
    QByteArray bytes;
    std::shared_ptr<rive::CGFactory> factory;
    rive::rcp<rive::File> file;
    QStringList artboards;
    QStringList animations;
    QStringList stateMachines;
};

class RuntimeBridge : public QObject
{
    Q_OBJECT

public:
    explicit RuntimeBridge(QObject* parent = nullptr);

    void setSource(const QUrl& source);
    void setAssetRoot(const QUrl& assetRoot);
    void setGraphicsApi(RiveFile::GraphicsApi graphicsApi);
    void load();

    RiveFile::GraphicsApi graphicsApi() const;
    std::shared_ptr<RuntimeDocument> document() const;

signals:
    void loadingStarted();
    void loadingFinished(const QStringList& artboards,
                         const QStringList& animations,
                         const QStringList& stateMachines);
    void loadingFailed(const QString& errorString);

private:
    QUrl m_source;
    QUrl m_assetRoot;
    RiveFile::GraphicsApi m_graphicsApi = RiveFile::GraphicsApi::Unknown;
    std::shared_ptr<RuntimeDocument> m_document;
};
