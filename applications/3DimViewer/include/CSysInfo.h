///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
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

#ifndef CSYSINFO_H
#define CSYSINFO_H

#include <QString>

class QOpenGLContext;

//! Basic system information
class CSysInfo
{
public:
    std::string m_sCardName;
    quint64 m_adapterRam;
    quint64 m_totalRam;
    quint64 m_cpuCoresCount;
    std::string m_sOpenGLInfo;
    QString m_sApplicationName;
	QString m_sOperatingSystem;
    std::string m_sCPU;
    quint64 m_maxTextureSize;
    quint64 m_max3DTextureSize;
    bool    m_bOpenGLOk;
    bool    m_bTexturesNPOT;

protected:
    // helper methods
    void   getTotalSystemMemory();
    void   getGraphicsCardInfo();
    void   getProcessorInfo();
    void   getOperatingSystemInfo();
    void   getExtendedOperatingSystemInfo();
    void   getEnviromentInfo();
    void   getOpenGLInfo(QOpenGLContext* context);
    void   getApplicationInfo();
    void   getDrivesInfo();
    void   getComputerInfo();
    void   logLoadedModules();
    void   logVideoMemoryUsage();
public:
    quint64 getAdapterRam() const { return m_adapterRam; };
    quint64 getTotalRam() const { return m_totalRam; };
public:
    //! Constructor
	CSysInfo();    
    //! Initialization
    void   init(); 
    //! Is OpenGL support good enough?
    bool   isOpenGLOk() const { return m_bOpenGLOk; }
    //! Writes current memory usage to log file
    static void    logMemoryStatus();
    //! Formats number to string with appropriate suffix
    static std::string formatBytes(quint64 size);
    //! Returns instance of sysinfo
    static CSysInfo* instance()
    {
        static CSysInfo sysInfo;
        return &sysInfo;
    }
    QString getApplicationName(QString subType);
};

#endif
