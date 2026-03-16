#pragma once

#include <QtCore/qglobal.h>

#if defined(RIVEQML_LIBRARY)
#define RIVEQML_EXPORT Q_DECL_EXPORT
#else
#define RIVEQML_EXPORT Q_DECL_IMPORT
#endif
