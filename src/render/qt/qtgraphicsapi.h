#pragma once

#include <RiveQml/rivefile.h>

#include <QString>

class QQuickWindow;

namespace QtGraphicsApi
{
RiveFile::GraphicsApi fromWindow(const QQuickWindow* window);
QString toString(RiveFile::GraphicsApi graphicsApi);
}
