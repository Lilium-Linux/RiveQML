#include "qtgraphicsapi.h"

#include <QQuickWindow>
#include <QSGRendererInterface>

namespace QtGraphicsApi
{
RiveFile::GraphicsApi fromWindow(const QQuickWindow* window)
{
    if (window == nullptr || window->rendererInterface() == nullptr)
    {
        return RiveFile::GraphicsApi::Unknown;
    }

    switch (window->rendererInterface()->graphicsApi())
    {
        case QSGRendererInterface::OpenGL:
            return RiveFile::GraphicsApi::OpenGL;
        case QSGRendererInterface::Vulkan:
            return RiveFile::GraphicsApi::Vulkan;
        case QSGRendererInterface::Metal:
            return RiveFile::GraphicsApi::Metal;
        case QSGRendererInterface::Direct3D11:
            return RiveFile::GraphicsApi::Direct3D11;
        default:
            return RiveFile::GraphicsApi::Unknown;
    }
}

QString toString(RiveFile::GraphicsApi graphicsApi)
{
    switch (graphicsApi)
    {
        case RiveFile::GraphicsApi::OpenGL:
            return QStringLiteral("OpenGL");
        case RiveFile::GraphicsApi::Vulkan:
            return QStringLiteral("Vulkan");
        case RiveFile::GraphicsApi::Metal:
            return QStringLiteral("Metal");
        case RiveFile::GraphicsApi::Direct3D11:
            return QStringLiteral("Direct3D11");
        default:
            return QStringLiteral("Unknown");
    }
}
}
