#pragma once

#include <RiveQml/riveqmlglobal.h>

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

#include <memory>

class RuntimeBridge;
class RiveItem;
class RiveRuntimeItemState;

class RIVEQML_EXPORT RiveFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(GraphicsApi graphicsApi READ graphicsApi NOTIFY graphicsApiChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QStringList artboards READ artboards NOTIFY artboardsChanged)
    Q_PROPERTY(QStringList animations READ animations NOTIFY animationsChanged)
    Q_PROPERTY(QStringList stateMachines READ stateMachines NOTIFY stateMachinesChanged)
    QML_ELEMENT

public:
    enum class Status
    {
        Null,
        Loading,
        Ready,
        Error,
    };
    Q_ENUM(Status)

    enum class GraphicsApi
    {
        Unknown,
        OpenGL,
        Vulkan,
        Metal,
        Direct3D11,
    };
    Q_ENUM(GraphicsApi)

    explicit RiveFile(QObject* parent = nullptr);
    ~RiveFile() override;

    QUrl source() const;
    void setSource(const QUrl& source);

    Status status() const;
    GraphicsApi graphicsApi() const;
    QString errorString() const;
    QStringList artboards() const;
    QStringList animations() const;
    QStringList stateMachines() const;

    void setGraphicsApi(GraphicsApi api);
    Q_INVOKABLE void reload();
    QString graphicsApiName() const;

signals:
    void sourceChanged();
    void statusChanged();
    void graphicsApiChanged();
    void errorStringChanged();
    void artboardsChanged();
    void animationsChanged();
    void stateMachinesChanged();

private:
    friend class RiveItem;
    friend class RiveRuntimeItemState;

    void resetMetadata();
    void updateStatus(Status status, QString errorString = {});
    RuntimeBridge* runtimeBridge() const;
    void setAssetRoot(const QUrl& assetRoot);

    QUrl m_source;
    Status m_status = Status::Null;
    GraphicsApi m_graphicsApi = GraphicsApi::Unknown;
    QString m_errorString;
    QStringList m_artboards;
    QStringList m_animations;
    QStringList m_stateMachines;
    std::unique_ptr<RuntimeBridge> m_runtimeBridge;
};
