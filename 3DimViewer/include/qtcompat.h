#ifndef __QTCOMPAT_H
#define __QTCOMPAT_H

#include <qglobal.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <QStandardPaths>
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    #define SETRESIZEMODE(x,y) setResizeMode(x,y)
    #define HOMELOCATION() QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
    #define DESKTOPLOCATION() QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)
    #define DATALOCATION() QDesktopServices::storageLocation(QDesktopServices::DataLocation)
    #define DOCUMENTSLOCATION() QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
#else
    #define SETRESIZEMODE(x,y) setSectionResizeMode(x,y)
    #define HOMELOCATION() QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)
    #define DESKTOPLOCATION() QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory)
    #define DATALOCATION() QStandardPaths::locate(QStandardPaths::DataLocation, QString(), QStandardPaths::LocateDirectory)
    #define DOCUMENTSLOCATION() QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory)
#endif

#endif