#ifndef BUILD_DOWNLOAD_H
#define BUILD_DOWNLOAD_H

#include <QtCore/qglobal.h>

#ifdef BUILD_DOWNLOAD
# define DOWNLOAD_EXPORT Q_DECL_EXPORT
#else
# define DOWNLOAD_EXPORT Q_DECL_IMPORT
#endif

#endif
