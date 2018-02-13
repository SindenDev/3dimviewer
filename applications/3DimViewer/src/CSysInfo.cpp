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


#include <CSysInfo.h>
#include <QSysInfo>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <app/CProductInfo.h>
#include <QApplication>
#include <C3DimApplication.h>
#include <mainwindow.h>

#if defined(Q_WS_WIN) || defined(Q_OS_WIN)
    #include <windows.h> // to detect wow64 process
#endif

#include <QOpenGLWidget>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

#ifdef _WIN32 // Windows specific
    #include <Windows.h>
    #include <Wbemcli.h>
    #include <comdef.h>
    #pragma comment(lib, "wbemuuid.lib")
    #pragma comment(lib, "Version.lib" )
    #include <dxgi.h>
    #include <ddraw.h>
    #ifndef SAFE_RELEASE
	    #define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
    #endif
#endif

#include <mainwindow.h> // wcs2ACP

#ifdef __APPLE__
#include <unistd.h>
#include <sys/sysctl.h>
#include <ApplicationServices/ApplicationServices.h> // CoreGraphics
#include <OpenGL/OpenGL.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <mach/mach.h>
#include <mach/mach_port.h>
#include <mach-o/dyld_images.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#endif

#ifdef Q_OS_LINUX
#   include <unistd.h>
#   include <linux/sysctl.h>
#endif

#define wcs2ACP MainWindow::wcs2ACP

CSysInfo::CSysInfo()
{
    m_totalRam = 0;
    m_adapterRam = 0;    
    m_cpuCoresCount = 0;
    m_maxTextureSize = 0;
    m_max3DTextureSize = 0;
    m_bOpenGLOk = false;
}

void CSysInfo::getApplicationInfo()
{
    m_sApplicationName = getApplicationName("");
}

QString CSysInfo::getApplicationName(QString subType)
{
    if (!subType.isEmpty())
        subType = " " + subType;
    QString res = QCoreApplication::applicationName();
	app::CProductInfo info=app::getProductInfo();
	res += QString("%5 %1.%2 %3").arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(QString::fromStdString(info.getNote())).arg(subType);

#if defined (_WIN64) || defined (__APPLE__)
    res += " (64 bit)";
#endif
    return res;
}

void CSysInfo::logVideoMemoryUsage()
{
#ifdef _WIN32 // video memory check using direct draw
    {
        HINSTANCE hInstDDraw = LoadLibrary(L"ddraw.dll"); // to prevent linkage to ddraw
        if (nullptr != hInstDDraw)
        {
            typedef HRESULT(WINAPI* LPDIRECTDRAWCREATE)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
            LPDIRECTDRAWCREATE pDDCreate = (LPDIRECTDRAWCREATE)GetProcAddress(hInstDDraw, "DirectDrawCreate");
            if (nullptr != pDDCreate)
            {
                LPDIRECTDRAW mDDraw = nullptr;
                IDirectDraw7* mDDraw2 = nullptr;
                HRESULT  mDDrawResult = pDDCreate(NULL, &mDDraw, NULL);
                if (nullptr != mDDraw)
                {
                    const GUID FAR IID_IDirectDraw7{ 0x15e65ec0, 0x3b9c, 0x11d2,{ 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b } }; // so we don't have to link dxguid
                    mDDrawResult = mDDraw->QueryInterface(IID_IDirectDraw7, (LPVOID *)&mDDraw2);
                    if (nullptr != mDDraw2)
                    {
                        DDSCAPS2 ddscaps = {};
                        DWORD   totalmem = 0, freemem = 0, totalmemloc = 0, freememloc = 0;
                        ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY /*| DDSCAPS_LOCALVIDMEM*/;
                        mDDrawResult = mDDraw2->GetAvailableVidMem(&ddscaps, &totalmem, &freemem);
                        ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
                        mDDrawResult = mDDraw2->GetAvailableVidMem(&ddscaps, &totalmemloc, &freememloc);
                        if (SUCCEEDED(mDDrawResult))
                        {
                            VPL_LOG_INFO("VRAM Check " << formatBytes(freemem) << "/" << formatBytes(totalmem) << " local " << formatBytes(freememloc) << "/" << formatBytes(totalmemloc));
                        }
                        mDDraw2->Release();
                    }
                    mDDraw->Release();
                }
            }
            FreeLibrary(hInstDDraw);
        }
        return;
    }
#endif
#ifdef __APPLE__
    {
        // free vram check
        kern_return_t krc = KERN_SUCCESS;
        mach_port_t masterPort = kIOMasterPortDefault;
        krc = IOMasterPort(bootstrap_port, &masterPort);
        if (krc == KERN_SUCCESS)
        {
            CFMutableDictionaryRef pattern = IOServiceMatching(kIOAcceleratorClassName);
            //CFShow(pattern);

            io_iterator_t deviceIterator;
            krc = IOServiceGetMatchingServices(masterPort, pattern, &deviceIterator);
            if (krc == KERN_SUCCESS)
            {
                io_object_t object;
                while ((object = IOIteratorNext(deviceIterator)))
                {
                    CFMutableDictionaryRef properties = NULL;
                    krc = IORegistryEntryCreateCFProperties(object, &properties, kCFAllocatorDefault, (IOOptionBits)0);
                    if (krc == KERN_SUCCESS)
                    {
                        CFMutableDictionaryRef perf_properties = (CFMutableDictionaryRef)CFDictionaryGetValue(properties, CFSTR("PerformanceStatistics"));
                        //CFShow(perf_properties);
                        if (perf_properties)
                        {
                            // look for a number of keys (this is mostly reverse engineering and best-guess effort)
                            const void* free_vram_number = CFDictionaryGetValue(perf_properties, CFSTR("vramFreeBytes"));
                            if (free_vram_number)
                            {
                                ssize_t vramFreeBytes = 0;
                                CFNumberGetValue((CFNumberRef)free_vram_number, kCFNumberSInt64Type, &vramFreeBytes);
                                VPL_LOG_INFO("Free VRAM " << vramFreeBytes / (1024 * 1024) << "MB");
                            }
                            const void* used_vram_number = CFDictionaryGetValue(perf_properties, CFSTR("vramUsedBytes"));
                            if (used_vram_number)
                            {
                                ssize_t vramUsedBytes = 0;
                                CFNumberGetValue((CFNumberRef)used_vram_number, kCFNumberSInt64Type, &vramUsedBytes);
                                VPL_LOG_INFO("Used VRAM " << vramUsedBytes / (1024 * 1024) << "MB");
                            }
                        }
                    }
                    if (properties)
                        CFRelease(properties);
                    IOObjectRelease(object);
                }
                IOObjectRelease(deviceIterator);
            }
            mach_port_deallocate(mach_task_self(), masterPort);
        }
        return;
    }
#endif
}

void CSysInfo::getOpenGLInfo(QOpenGLContext* context)
{
    if (nullptr == context)
        return;

    QOpenGLFunctions fnGL(context);
    // get OpenGL info
    QString glInfo;
    {
        QOpenGLContext::OpenGLModuleType ogltype = QOpenGLContext::openGLModuleType();
        if (QOpenGLContext::LibGL == ogltype)
            glInfo = "LibGL";
        if (QOpenGLContext::LibGLES == ogltype)
            glInfo = "LibGLES";
    }
    {
        const GLubyte* pVendor = fnGL.glGetString(GL_VENDOR);
        if (NULL != pVendor)
        {
            glInfo += "\n" + QString(QLatin1String((const char*)pVendor));
        }
        const GLubyte* pRenderer = fnGL.glGetString(GL_RENDERER);
        if (NULL != pRenderer)
        {
            glInfo += "\n" + QString(QLatin1String((const char*)pRenderer));
            if (m_sCardName.empty())
                m_sCardName = (const char*)pRenderer;
        }
        const GLubyte* pVersion = fnGL.glGetString(GL_VERSION);
        if (NULL != pVersion)
        {
            glInfo += "\n" + QString(QLatin1String((const char*)pVersion));
        }
    }
    m_sOpenGLInfo = glInfo.toStdString();

    GLint maxTextureSize = 0;
    fnGL.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glInfo += "\nGL_MAX_TEXTURE_SIZE " + QString::number(maxTextureSize);
    m_maxTextureSize = maxTextureSize;

    GLint max3DTextureSize = 0;
    fnGL.glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DTextureSize);
    glInfo += "\nGL_MAX_3D_TEXTURE_SIZE " + QString::number(max3DTextureSize);
    m_max3DTextureSize = max3DTextureSize;

    m_bTexturesNPOT = fnGL.hasOpenGLFeature(QOpenGLFunctions::NPOTTextures);
    glInfo += "\nNon power of two textures ";
    if (m_bTexturesNPOT)
        glInfo += "Yes";
    else
        glInfo += "No";

    VPL_LOG_INFO(glInfo.toStdString());

    logVideoMemoryUsage();
}

void   CSysInfo::getComputerInfo()
{
#ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512%28v=vs.85%29.aspx

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;    
    HRESULT hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator,
        (LPVOID *) &pLoc
        );
    if( FAILED(hres) )
    {
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"root\\cimv2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    if( FAILED(hres) )
    {
        pLoc->Release();
        return;
    }
    
    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );
    if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

    {
        // Get serial number of the first physical disk
        hres = pSvc->ExecQuery(
            bstr_t("WQL"), 
            bstr_t("SELECT * FROM Win32_ComputerSystem"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
            );
        
        if( FAILED(hres) )
        {
            pSvc->Release();
            pLoc->Release();
            return;
        }

        // Get the data from the query
        IWbemClassObject *pclsObj = 0;
        ULONG uReturn = 0;

        while( pEnumerator )
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if( uReturn == 0 )
            {
                break;
            }

            VARIANT vtProp;           

            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsManufacturer = vtProp.bstrVal;
            VariantClear(&vtProp);

            hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsModel = vtProp.bstrVal;
            VariantClear(&vtProp);
            
            pclsObj->Release();

            // Write to log
            VPL_LOG_INFO(wcs2ACP(wsManufacturer) << " " << wcs2ACP(wsModel));
        }

        pEnumerator->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    return;
#endif

}

void   CSysInfo::getProcessorInfo()
{
#ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512%28v=vs.85%29.aspx

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;    
    HRESULT hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator,
        (LPVOID *) &pLoc
        );
    if( FAILED(hres) )
    {
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"root\\cimv2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    if( FAILED(hres) )
    {
        pLoc->Release();
        return;
    }
    
    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );
    if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

    {
        // Get serial number of the first physical disk
        hres = pSvc->ExecQuery(
            bstr_t("WQL"), 
            bstr_t("SELECT * FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
            );
        
        if( FAILED(hres) )
        {
            pSvc->Release();
            pLoc->Release();
            return;
        }

        // Get the data from the query
        IWbemClassObject *pclsObj = 0;
        ULONG uReturn = 0;

        while( pEnumerator )
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if( uReturn == 0 )
            {
                break;
            }

            VARIANT vtProp;           

            // Get the Caption property
            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsCPUName = vtProp.bstrVal;
            VariantClear(&vtProp);

            // Get the cores count property
            hr = pclsObj->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            quint32 nCores = vtProp.uintVal;            
            VariantClear(&vtProp);

            // Get the logical cores count property
            hr = pclsObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            quint32 nCoresLogical = vtProp.uintVal;            
            VariantClear(&vtProp);
            
            pclsObj->Release();

            // Write to log
            VPL_LOG_INFO(wcs2ACP(wsCPUName) << " Cores: " << nCores << "(" << nCoresLogical << ")");

            m_sCPU = wcs2ACP(wsCPUName);
            m_cpuCoresCount = nCores;
        }

        pEnumerator->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    return;
#endif
#ifdef __APPLE__
    size_t len = 0;
    sysctlbyname( "machdep.cpu.brand_string", NULL, &len, NULL, 0 );
    char *rstring = new char[len];
    sysctlbyname( "machdep.cpu.brand_string", rstring, &len, NULL, 0 );
    
    uint coreCount = 0;
    len = sizeof(coreCount);
    sysctlbyname( "machdep.cpu.core_count", &coreCount, &len, NULL, 0 );
    
    uint threadCount = 0;
    len = sizeof(threadCount);
    sysctlbyname( "machdep.cpu.thread_count", &threadCount, &len, NULL, 0 );
    
    VPL_LOG_INFO(rstring << " Cores: " << coreCount << "(" << threadCount << ")");

    m_sCPU = rstring;
    m_cpuCoresCount = coreCount;

    delete [] rstring;
#endif
}

#if defined(Q_WS_WIN) || defined(Q_OS_WIN)
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
#endif

void   CSysInfo::getExtendedOperatingSystemInfo()
{
#ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512%28v=vs.85%29.aspx

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;
    HRESULT hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID *)&pLoc
    );
    if (FAILED(hres))
    {
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"root\\cimv2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (e.g. Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
    );
    if (FAILED(hres))
    {
        pLoc->Release();
        return;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

    {
        // Get serial number of the first physical disk
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_OperatingSystem"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hres))
        {
            pSvc->Release();
            pLoc->Release();
            return;
        }

        // Get the data from the query
        IWbemClassObject *pclsObj = 0;
        ULONG uReturn = 0;

        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (uReturn == 0)
            {
                break;
            }

            VARIANT vtProp;

            hr = pclsObj->Get(L"MaxProcessMemorySize", 0, &vtProp, 0, 0);
            if (FAILED(hr) || vtProp.vt == VT_NULL)
            {
                pclsObj->Release();
                continue;
            }
            std::wstring maxProcessMemory = vtProp.bstrVal;
            VariantClear(&vtProp);
            if (!maxProcessMemory.empty())
            {
                VPL_LOG_INFO("Max Process Memory " << wcs2ACP(maxProcessMemory) + "kB");
            }

            hr = pclsObj->Get(L"FreePhysicalMemory", 0, &vtProp, 0, 0);
            if (FAILED(hr) || vtProp.vt == VT_NULL)
            {
                pclsObj->Release();
                continue;
            }
            std::wstring freePhysical = vtProp.bstrVal;
            VariantClear(&vtProp);
            if (!freePhysical.empty())
            {
                VPL_LOG_INFO("Free Physical Memory " << wcs2ACP(freePhysical) + "kB");
            }

            hr = pclsObj->Get(L"FreeVirtualMemory", 0, &vtProp, 0, 0);
            if (FAILED(hr) || vtProp.vt == VT_NULL)
            {
                pclsObj->Release();
                continue;
            }
            std::wstring freeVirtual = vtProp.bstrVal;
            VariantClear(&vtProp);
            if (!freeVirtual.empty())
            {
                VPL_LOG_INFO("Free Virtual Memory " << wcs2ACP(freeVirtual) + "kB");
            }
            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    return;
#endif

}

void   CSysInfo::getOperatingSystemInfo()
{
    QString operatingSystemString;
#if defined(Q_WS_WIN) || defined(Q_OS_WIN)
    switch ( QSysInfo::WindowsVersion ) {
        case QSysInfo::WV_NT :
            operatingSystemString = "Windows NT";
            break;
        case QSysInfo::WV_2000 :
            operatingSystemString = "Windows 2000";
            break;
        case QSysInfo::WV_XP :
            operatingSystemString = "Windows XP";
            break;
        case QSysInfo::WV_2003 :
            operatingSystemString = "Windows Server 2003, Windows Server 2003 R2, Windows Home Server, Windows XP Professional x64 Edition";
            break;
        case QSysInfo::WV_VISTA :
            operatingSystemString = "Windows Vista, Windows Server 2008";
            break;
        case QSysInfo::WV_WINDOWS7 :
            operatingSystemString = "Windows 7";
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 3))
        case QSysInfo::WV_WINDOWS8 :
            operatingSystemString = "Windows 8";
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
        case QSysInfo::WV_WINDOWS8_1 :
            operatingSystemString = "Windows 8.1";
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 7))
        case QSysInfo::WV_WINDOWS10 :
            operatingSystemString = "Windows 10";
            break;
#endif
        case QSysInfo::WV_NT_based :
            operatingSystemString = "Windows NT Based";
            break;
        default :
            operatingSystemString = "Unknown Windows operating system.";
            break;
    }
    // detect 64 bit OS
  #ifdef _WIN64
    operatingSystemString+=" (64 bit)";
  #else
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if(NULL != fnIsWow64Process)
    {
        BOOL bIsWow64 = FALSE;
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
        if (bIsWow64)
            operatingSystemString+=" (64 bit)";
    }
  #endif
#elif defined(Q_WS_MAC) || defined(Q_OS_MAC)
    switch ( QSysInfo::MacintoshVersion ) {
        case QSysInfo::MV_10_0 :
            operatingSystemString = "Mac OS X 10.0 Cheetah";
            break;
        case QSysInfo::MV_10_1 :
            operatingSystemString = "Mac OS X 10.1 Puma";
            break;
        case QSysInfo::MV_10_2 :
            operatingSystemString = "Mac OS X 10.2 Jaguar";
            break;
        case QSysInfo::MV_10_3 :
            operatingSystemString = "Mac OS X 10.3 Panther";
            break;
        case QSysInfo::MV_10_4 :
            operatingSystemString = "Mac OS X 10.4 Tiger";
            break;
        case QSysInfo::MV_10_5 :
            operatingSystemString = "Mac OS X 10.5 Leopard";
            break;
        case QSysInfo::MV_10_6 :
            operatingSystemString = "Mac OS X 10.6 Snow Leopard";
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 1))
        case QSysInfo::MV_10_7 :
            operatingSystemString = "Mac OS X 10.7 Lion";
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 3))
        case QSysInfo::MV_10_8 :
            operatingSystemString = "Mac OS X 10.8 Mountain Lion";
            break;            
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
        case QSysInfo::MV_10_9 :
            operatingSystemString = "Mac OS X 10.9 Mavericks";
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 7))
        case QSysInfo::MV_10_10 : 
			operatingSystemString = "Mac OS X 10.10 Yosemite";     
			break;
#endif
        case QSysInfo::MV_10_11:
            operatingSystemString = "Mac OS X 10.11 El Capitan";
            break;
        case QSysInfo::MV_10_12:
            operatingSystemString = "Mac OS X 10.12 Sierra";
            break;
        case Q_MV_OSX(10, 13):
            operatingSystemString = "Mac OS X 10.13 High Sierra";
            break;
        case QSysInfo::MV_Unknown :
            operatingSystemString = "An unknown and currently unsupported platform";
            break;
        default :
            operatingSystemString = "Unknown Mac operating system.";
            break;
    }
#elif defined(Q_OS_LINUX)
    // TODO: Get linux version
    operatingSystemString = "Linux";
#else
    operatingSystemString = "Unix";
#endif

    VPL_LOG_INFO(operatingSystemString.toStdString());
	m_sOperatingSystem = operatingSystemString;

#ifdef _WIN32
	OSVERSIONINFOA osVerInfo={};
	osVerInfo.dwOSVersionInfoSize=sizeof OSVERSIONINFOA;
	GetVersionExA(&osVerInfo);
    VPL_LOG_INFO("Version " << osVerInfo.dwMajorVersion << "." << osVerInfo.dwMinorVersion << "." << osVerInfo.dwBuildNumber << " " << osVerInfo.szCSDVersion);

    LONG(WINAPI *pfnRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
    (FARPROC&)pfnRtlGetVersion = GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
    if(pfnRtlGetVersion)
    {
        RTL_OSVERSIONINFOEXW ver = {0};
        ver.dwOSVersionInfoSize = sizeof(ver);
        if(pfnRtlGetVersion(&ver) == 0)
        {
            VPL_LOG_INFO("RtlVersion " << ver.dwMajorVersion << "." << ver.dwMinorVersion << "." << ver.dwBuildNumber << " " << wcs2ACP(ver.szCSDVersion));
        }
    }
#endif

#ifdef __APPLE__
    size_t len = 0;
    sysctlbyname( "kern.osversion", NULL, &len, NULL, 0 );
    char *osversion = new char[len];
    sysctlbyname( "kern.osversion", osversion, &len, NULL, 0 );
    
    len = 0;
    sysctlbyname( "kern.osrelease", NULL, &len, NULL, 0 );
    char *osrelease = new char[len];
    sysctlbyname( "kern.osrelease", osrelease, &len, NULL, 0 );
    
    VPL_LOG_INFO("MacOS " << osrelease << " (" << osversion << ")");
    delete [] osrelease;
    delete [] osversion;
#endif

    getExtendedOperatingSystemInfo();
}

void   CSysInfo::getGraphicsCardInfo()
{
#ifdef _WIN32
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512%28v=vs.85%29.aspx
	// http://code.msdn.microsoft.com/windowsdesktop/DirectX-Video-Memory-ee7d8319
	
	// DXGI is only available on Windows Vista or later. This method returns the 
	// amount of dedicated video memory, the amount of dedicated system memory, 
	// and the amount of shared system memory.
	HINSTANCE hDXGI = LoadLibrary( L"dxgi.dll" );
    if( hDXGI )
    {
        typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );

        LPCREATEDXGIFACTORY pCreateDXGIFactory = nullptr;
        IDXGIFactory* pDXGIFactory = nullptr;

        // We prefer the use of DXGI 1.1
        pCreateDXGIFactory = ( LPCREATEDXGIFACTORY )GetProcAddress( hDXGI, "CreateDXGIFactory1" );
        if ( !pCreateDXGIFactory )
        {
            pCreateDXGIFactory = ( LPCREATEDXGIFACTORY )GetProcAddress( hDXGI, "CreateDXGIFactory" );
        }
        
		if (pCreateDXGIFactory)
		{
			HRESULT hr = pCreateDXGIFactory( __uuidof( IDXGIFactory ), ( LPVOID* )&pDXGIFactory );
			if ( SUCCEEDED(hr) )
			{
				for( UINT index = 0; ; ++index )
				{
					IDXGIAdapter* pAdapter = nullptr;
					HRESULT hr = pDXGIFactory->EnumAdapters( index, &pAdapter );
					if( FAILED( hr ) ) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
						break;

					DXGI_ADAPTER_DESC desc = {};
					if( SUCCEEDED( pAdapter->GetDesc( &desc ) ) )
					{
						/*for( UINT iOutput = 0; ; ++iOutput )
						{
							IDXGIOutput* pOutput = nullptr;
							hr = pAdapter->EnumOutputs( iOutput, &pOutput );
							if( FAILED( hr ) ) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
								break;

							DXGI_OUTPUT_DESC outputDesc;
							memset( &outputDesc, 0, sizeof( DXGI_OUTPUT_DESC ) );
							if( SUCCEEDED( pOutput->GetDesc( &outputDesc ) ) )
							{
								wprintf( L"hMonitor: 0x%0.8Ix\n", ( DWORD_PTR )outputDesc.Monitor );
								wprintf( L"hMonitor Device Name: %s\n", outputDesc.DeviceName );
							}

							SAFE_RELEASE( pOutput );
						}*/

						if (desc.DedicatedVideoMemory>m_adapterRam)
						{
							m_adapterRam = desc.DedicatedVideoMemory;
						}

						VPL_LOG_INFO("DXGI: " << wcs2ACP(desc.Description) << " Dedicated:" << formatBytes(desc.DedicatedVideoMemory) << " System:" << formatBytes(desc.DedicatedSystemMemory) << " Shared:" << formatBytes(desc.SharedSystemMemory));
					}
					SAFE_RELEASE( pAdapter );
				}
			}
			SAFE_RELEASE(pDXGIFactory);
		}
        FreeLibrary( hDXGI );
    }

    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512%28v=vs.85%29.aspx

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;    
    HRESULT hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator,
        (LPVOID *) &pLoc
        );
    if( FAILED(hres) )
    {
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"root\\cimv2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    if( FAILED(hres) )
    {
        pLoc->Release();
        return;
    }
    
    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );
    if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

    {
        // Get serial number of the first physical disk
        hres = pSvc->ExecQuery(
            bstr_t("WQL"), 
            //bstr_t("SELECT Caption,AdapterRAM,VideoProcessor FROM Win32_VideoController"),
            bstr_t("SELECT * FROM Win32_VideoController"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
            );
        
        if( FAILED(hres) )
        {
            pSvc->Release();
            pLoc->Release();
            return;
        }

        // Get the data from the query
        IWbemClassObject *pclsObj = 0;
        ULONG uReturn = 0;

        while( pEnumerator )
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if( uReturn == 0 )
            {
                break;
            }

            VARIANT vtProp;           

            // Get the card name property
            hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsCardName = vtProp.bstrVal;
            VariantClear(&vtProp);

            // Get the gpu name property
            std::wstring wsGPUName;
            hr = pclsObj->Get(L"VideoProcessor", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                //pclsObj->Release();
                //continue;
            }
            else
                wsGPUName = vtProp.bstrVal;
            VariantClear(&vtProp);

            // Get the driver version property
            hr = pclsObj->Get(L"DriverVersion", 0, &vtProp, 0, 0);
  
            std::wstring wsDriverVersion;
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                //pclsObj->Release();
                //continue;
            }
            else
            {
                if (SUCCEEDED(VariantChangeType(&vtProp,&vtProp,0,VT_BSTR)))
                    wsDriverVersion = vtProp.bstrVal;
            }
            VariantClear(&vtProp);

            // Get the driver version property
            QDate driverDate;
            hr = pclsObj->Get(L"DriverDate", 0, &vtProp, 0, 0);  
            std::wstring wsDriverDate;
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                //pclsObj->Release();
                //continue;
            }
            else
            {
                if (vtProp.vt == VT_DATE)
                {
                    SYSTEMTIME st={};
                    if (VariantTimeToSystemTime(vtProp.date,&st))
                    {
                        std::wstringstream ss;
                        ss << st.wYear << L" " << st.wMonth << L" "  << st.wDay;
                        wsDriverDate = ss.str();
                    }
                }
                if (vtProp.vt == VT_BSTR)
                    wsDriverDate = vtProp.bstrVal;
                if (!wsDriverDate.empty())
                {
                    QString str = QString::fromUtf16((const ushort*)wsDriverDate.c_str());
                    QRegExp rexp("^[0-9]{14}\\.");
                    if (0 == rexp.indexIn(str))
                    {
                        QDate date = QDate::fromString(str.left(8), "yyyyMMdd");
                        if (date.isValid())
                        {
                            driverDate = date;
                            //if (m_gfxDriverDate.isNull() || !m_gfxDriverDate.isValid() || m_gfxDriverDate>date)
                            //   m_gfxDriverDate = date;
                            //qDebug() << m_gfxDriverDate.toString();
                        }
                    }
                }
            }
            VariantClear(&vtProp);

            // Get the adapter ram property
            quint64 nCardRam = 0;
            hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                //pclsObj->Release();
                //continue;
            }
            else
            {
                if (SUCCEEDED(VariantChangeType(&vtProp,&vtProp,0,VT_UI8)))
                    nCardRam = vtProp.ullVal;
                else if (vtProp.vt == VT_I4)
                {
                    nCardRam = *(unsigned int*)(&vtProp.intVal);
                }
            }
            VariantClear(&vtProp);
            
            pclsObj->Release();

            // 
            if( !wsCardName.empty() )
            {
                if (nCardRam>m_adapterRam)
                {
                    m_adapterRam = nCardRam;
                    m_sCardName = wcs2ACP(wsCardName);
                }
                VPL_LOG_INFO(wcs2ACP(wsCardName) << " " << wcs2ACP(wsGPUName) << " " << formatBytes(nCardRam) << " " << wcs2ACP(wsDriverVersion) << " " << wcs2ACP(wsDriverDate));
            }
        }

        pEnumerator->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    return;
#endif
#ifdef __APPLE__
    {
        CGError                 err = CGDisplayNoErr;
        
        CGDirectDisplayID       *displays = NULL;
        CGDisplayCount          dspCount = 0;
        
        // How many active displays do we have?
        err = CGGetActiveDisplayList(0, NULL, &dspCount);
        
        // Allocate enough memory to hold all the display IDs we have
        displays = (CGDirectDisplayID*)calloc((size_t)dspCount, sizeof(CGDirectDisplayID));
        
        // Get the list of active displays
        err = CGGetActiveDisplayList(dspCount, displays, &dspCount);
        
        // Now we iterate through them
        for(unsigned int i = 0; i < dspCount; i++)
        {
            quint64 nVRamSize = 0;
            
            CGLRendererInfoObj rend;
            GLint rendCount = 0;
            CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(displays[i]),&rend,&rendCount);
            for(int j = 0; j<rendCount; j++)
            {
                // get video memory
                GLint value = 0;
                CGLDescribeRenderer(rend,j,kCGLRPVideoMemoryMegabytes, &value);
                nVRamSize = 1024*1024*(quint64)value;
                if (0==nVRamSize)
                {
                    CGLDescribeRenderer(rend,j,kCGLRPVideoMemory, &value);
                    nVRamSize = value;
                }
                // get renderer id
                value = 0;
                CGLDescribeRenderer(rend,j,kCGLRPRendererID, &value);
                
                if (nVRamSize>m_adapterRam)
                {
                    m_adapterRam = nVRamSize;
                }
                VPL_LOG_INFO("RendererID " << std::hex << value << std::dec << " VRAM " << nVRamSize/(1024*1024) << "MB");
            }
            CGLDestroyRendererInfo(rend);

            // information extraction using registry, won't find the second card
            /*
            CFTypeRef               typeCode;
             
            // Get the service port for the display
            io_service_t dspPort = CGDisplayIOServicePort(displays[i]);
            
            std::string sModel;
            CFDataRef model = (CFDataRef)IORegistryEntrySearchCFProperty(dspPort,kIOServicePlane, CFSTR("model"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
            if (NULL!=model)
            {
                const UInt8* pData = CFDataGetBytePtr(model);
                int len = CFDataGetLength(model);
                sModel.append((char*)pData, len);
                CFRelease(model);
            }
            // Ask IOKit for the VRAM size property
            typeCode = IORegistryEntrySearchCFProperty(dspPort, kIOServicePlane, CFSTR("VRAM,totalsize"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);

            if(typeCode && CFGetTypeID(typeCode) == CFDataGetTypeID())
            {
                const UInt8* v = CFDataGetBytePtr(static_cast<CFDataRef>(typeCode));
                nVRamSize = *(reinterpret_cast<const unsigned int*>(v));
            }
            if(typeCode)
                CFRelease(typeCode);
            
            if (0==nVRamSize)
            {
                typeCode = IORegistryEntryCreateCFProperty(dspPort,
                                                       CFSTR(kIOFBMemorySizeKey),
                                                       kCFAllocatorDefault,
                                                       kNilOptions);
            
            
                // Ensure we have valid data from IOKit
                if(typeCode && CFGetTypeID(typeCode) == CFNumberGetTypeID())
                {
                    // If so, convert the CFNumber into a plain unsigned long
                    CFNumberGetValue((CFNumberRef)typeCode, kCFNumberSInt32Type, &nVRamSize);
                }
                if(typeCode)
                    CFRelease(typeCode);
            }
            if (nVRamSize>m_adapterRam)
            {
                m_adapterRam = nVRamSize;
                //m_sCardName = wcs2ACP(wsCardName);
            }
            VPL_LOG_INFO(sModel);
            VPL_LOG_INFO("VRAM " << nVRamSize/(1024*1024) << "MB");
             */
        }
        free(displays);
        return;
    }
#endif
}

void CSysInfo::getTotalSystemMemory()
{
#ifdef _WIN32
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    m_totalRam = status.ullTotalPhys;
#endif
#ifdef __APPLE__
    int mib[2]={CTL_HW,HW_MEMSIZE};
    size_t len = sizeof(m_totalRam);
    sysctl(mib,2,&m_totalRam,&len,NULL,0);
#endif
#ifdef Q_OS_LINUX
    // TODO: This does not work on Linux!!!
    //int mib[2]={CTL_HW,HW_MEMSIZE};
    //size_t len = sizeof(m_totalRam);
    //sysctl(mib,2,&m_totalRam,&len,NULL,0);
    m_totalRam = 0;
#endif
    VPL_LOG_INFO("RAM: " << m_totalRam/(1024*1024) << "MB ");
}

void CSysInfo::logMemoryStatus()
{
#ifdef _WIN32
    // get memory status
    MEMORYSTATUSEX statex = {};
	statex.dwLength = sizeof statex;
	GlobalMemoryStatusEx(&statex);

#define DIV (1024*1024)
	VPL_LOG_INFO("Memory In Use: " << statex.dwMemoryLoad << "%");
    VPL_LOG_INFO("Total Physical Memory: " << statex.ullTotalPhys/DIV << "MB");
    VPL_LOG_INFO("Free Physical Memory: " << statex.ullAvailPhys/DIV << "MB");
    VPL_LOG_INFO("Total Paging File: " << statex.ullTotalPageFile/DIV << "MB");
    VPL_LOG_INFO("Free Paging File: " << statex.ullAvailPageFile/DIV << "MB");
    VPL_LOG_INFO("Total Virtual Memory: " << statex.ullTotalVirtual/DIV << "MB");
    VPL_LOG_INFO("Free Virtual Memory: " << statex.ullAvailVirtual/DIV << "MB");
#undef DIV

#endif // WIN32
}

void CSysInfo::getEnviromentInfo()
{
    // monitor count and resolution
    QDesktopWidget* pDesktop=QApplication::desktop();
    if (NULL!=pDesktop)
    {
        int nScreens = pDesktop->screenCount();
        VPL_LOG_INFO("Monitor Count: " << nScreens);
        for(int i = 0; i < nScreens; i++)
        {
            QRect geom = pDesktop->screenGeometry(i);
            VPL_LOG_INFO(i << ": " << geom.width() << "x" << geom.height());
        }
    }

    // language id
#ifdef _WIN32
    VPL_LOG_INFO("System Default Language ID: " << GetSystemDefaultLangID() << " User Default Language ID: " << GetUserDefaultLangID());
#endif
}

std::string CSysInfo::formatBytes(quint64 size)
{
    std::stringstream ss;
    if (size<10*1024)
    {
        ss << size << "B";
        return ss.str();
    }    
    if (size<10*1024*1024)
    {
        ss << size/1024 << "kB";
        return ss.str();
    }
    if (size<10*(quint64)1024*1024*1024)
    {
        ss << size/(1024*1024) << "MB";
        return ss.str();
    }
    ss << size/(1024*1024*1024) << "GB";
    return ss.str();
}

void CSysInfo::getDrivesInfo()
{
#ifdef _WIN32
    char drives[1024]={},*pDrives=drives;
    DWORD logicalDriveStrings=GetLogicalDriveStringsA(sizeof(drives),drives);	
    while(*pDrives)
    {
        unsigned int driveType = GetDriveTypeA(pDrives);
        if (DRIVE_FIXED == driveType)
		{
			ULARGE_INTEGER  freeBytesAvailable = {},
							totalNumberOfBytes = {},
							totalNumberOfFreeBytes = {};
			GetDiskFreeSpaceExA(pDrives,&freeBytesAvailable,&totalNumberOfBytes,&totalNumberOfFreeBytes);				
            VPL_LOG_INFO(pDrives << " " << "Total: " << formatBytes(totalNumberOfBytes.QuadPart) << " Free: " << formatBytes(totalNumberOfFreeBytes.QuadPart) << " Available: " << formatBytes(freeBytesAvailable.QuadPart));
		}
	    pDrives+=strlen(pDrives)+1;
    }
#endif
#ifdef __APPLE__
    struct statfs *mntbufp = NULL;
    unsigned count = getmntinfo(&mntbufp, 0);
    for(unsigned i = 0; i<count; i++)
    {
        char* volName = mntbufp[i].f_mntonname;
        if (NULL!=volName)
            VPL_LOG_INFO(volName << " " << "Total: " << formatBytes(mntbufp[i].f_bsize*mntbufp[i].f_blocks) << " Free: " << formatBytes(mntbufp[i].f_bsize*mntbufp[i].f_bfree) << " Available: " << formatBytes(mntbufp[i].f_bsize*mntbufp[i].f_bavail));
    }
#endif
}

#ifdef _WIN32
    #include <Psapi.h>
#endif

void CSysInfo::logLoadedModules()
{
#ifdef _WIN32    
    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
    if (NULL != hProcess)
    {
        VPL_LOG_INFO("Loaded modules:");
        // Get a list of all the modules in this process.
        HMODULE hMods[1024] = {};
        DWORD cbNeeded = 0;
        if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                // Get the full path to the module's file.
                wchar_t szModName[MAX_PATH*2]={};            
                if ( GetModuleFileNameExW( hProcess, hMods[i], szModName,
                                          sizeof(szModName) / sizeof(wchar_t)))
                {
                    int MajorVersion = 0,
                        MinorVersion = 0,
                        BuildNumber = 0,
                        RevisionNumber = 0;
                    DWORD dwVersionInfoSize = GetFileVersionInfoSize(szModName,NULL);
                    if (dwVersionInfoSize>0)
                    {
                        BYTE* lpData = new BYTE[dwVersionInfoSize];
                        if (NULL!=lpData)
                        {
                            if( GetFileVersionInfo( szModName, NULL, dwVersionInfoSize, lpData ) )
                            {
                                VS_FIXEDFILEINFO *pFileInfo;
                                UINT BufLen = 0;
                                if( VerQueryValue( lpData, LPCWSTR("\\"), (LPVOID *) &pFileInfo, &BufLen ) ) 
                                {
                                    MajorVersion = HIWORD(pFileInfo->dwFileVersionMS); 
                                    MinorVersion = LOWORD(pFileInfo->dwFileVersionMS); 
                                    BuildNumber = HIWORD(pFileInfo->dwFileVersionLS); 
                                    RevisionNumber = LOWORD(pFileInfo->dwFileVersionLS); 
                                }
                            }
                            delete [] lpData;
                        }
                    }

                    std::stringstream ss;                    
				    MODULEINFO mofo={};
				    if(GetModuleInformation(hProcess,hMods[i],&mofo,sizeof(mofo)))
				    {
                        ss << std::hex << "Base: " << mofo.lpBaseOfDll << " Entry: " << mofo.EntryPoint << " Size: " << std::dec << mofo.SizeOfImage;					
				    }
                    
                    std::wstring wstrModule = szModName;
					std::string sss = ss.str();
                    VPL_LOG_INFO(wcs2ACP(szModName) << "\t" << MajorVersion << "." << MinorVersion << "." << BuildNumber << "." << RevisionNumber <<"\t" << sss);
                }
            }
        }    
        CloseHandle( hProcess );
    }
#endif // WIN32
}


void CSysInfo::init()
{
    getApplicationInfo();
    getComputerInfo();
    getProcessorInfo();
    getTotalSystemMemory();
    getGraphicsCardInfo();
    if (0==m_adapterRam)
    {
        m_adapterRam = 512*1024*1024;
        VPL_LOG_INFO( "Couldn't detect GPU memory! Assume " << m_adapterRam/(1024*1024) << "MB" );
    }
    getOperatingSystemInfo();
    VPL_LOG_INFO(QSslSocket::sslLibraryVersionString().toStdString());
    getEnviromentInfo();
    getDrivesInfo();
    {     
		QOpenGLContext cx;
		QOpenGLWindow sf;
		sf.create();
		cx.create();
		cx.makeCurrent(&sf);

        // ask opengl info
        getOpenGLInfo(&cx);

        // check version
        m_bOpenGLOk = cx.format().majorVersion()>3 || (cx.format().majorVersion() == 3 && cx.format().minorVersion() >= 3);
        VPL_LOG_INFO("OpenGL 3.3 support: " << (m_bOpenGLOk ? "true" : "false"));
    }
    logLoadedModules();
}
