///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

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