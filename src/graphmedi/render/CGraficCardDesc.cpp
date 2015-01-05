///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

// COM library and WMI are used to get local computer info on Windows
#ifdef _WIN32
#   include <windows.h>
#   define _WIN32_DCOM
#   include <comdef.h>
#   include <wbemidl.h>
#   pragma comment(lib, "wbemuuid.lib")
#endif // _WIN32

#include <render/CGraficCardDesc.h>

#include <string>
#include <sstream>


namespace vr
{

///////////////////////////////////////////////////////////////////////////////
// Converts Unicode string to the ASCII one

void unicode2Ascii(const std::wstring& In, std::string& Out)
{
    Out.resize(In.size());
    for( std::string::size_type i = 0; i < In.size(); ++i )
    {
        Out[i] = char(In[i] & 0x7F);
    }

    std::string::size_type start = Out.find_first_not_of(' ');
    if( start != std::string::npos )
    {
        Out.erase(0, start);
    }

    std::string::size_type end = Out.find_last_not_of(' ');
    if( end != std::string::npos )
    {
        Out.erase(end + 1, std::string::npos);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Windows version
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

CGraficCardDesc::CGraficCardDesc() : m_State(1), m_bNeedUnitialize(false)
{
    // Initialize COM
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if( FAILED(hres) && hres != RPC_E_CHANGED_MODE )
    {
        // Cannot initialize the COM library
        throw CInitFailed();
    }
    m_bNeedUnitialize=SUCCEEDED(hres);
    
    // Set general COM security levels
    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );
/*    if( FAILED(hres) )
    {
        CoUninitialize();
        throw CSearchFailed();
    }*/

    // No error...
    m_State = 0;
}


///////////////////////////////////////////////////////////////////////////////
//

CGraficCardDesc::~CGraficCardDesc()
{
    if( m_State == 0 && m_bNeedUnitialize )
    {
        CoUninitialize();
    }
}


///////////////////////////////////////////////////////////////////////////////
//

unsigned int CGraficCardDesc::getAdapterRAM()
{
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
        throw CSearchFailed();
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
        throw CSearchFailed();
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
        throw CSearchFailed();
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    unsigned int uiRAM = 0;

    // Get MAC address of the first Ethernet network adapter
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
//        bstr_t("SELECT * FROM Win32_VideoController"),
        bstr_t("SELECT AdapterRAM FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
        );
    if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        throw CSearchFailed();
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

        // Get the adapter type
        hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
        if( FAILED(hr) || vtProp.vt == VT_NULL )
        {
            pclsObj->Release();
            continue;
        }
        unsigned int Value = vtProp.uintVal;
        VariantClear(&vtProp);

        // Check that any value was found?
        if( Value == 0 )
        {
            pclsObj->Release();
            continue;
        }

        pclsObj->Release();

        // Return the found value
        uiRAM = Value;
        break;
    }

    pEnumerator->Release();

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    // Anything found?
    if( uiRAM == 0 )
    {
        throw CSearchFailed();
    }
    return (uiRAM / 1024 / 1024);
//    return (uiRAM >> 20);
}


///////////////////////////////////////////////////////////////////////////////
//

std::string CGraficCardDesc::getAdapterName()
{
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
        throw CSearchFailed();
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
        throw CSearchFailed();
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
        throw CSearchFailed();
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    std::string ssDesc;
    
    // Get MAC address of the first Ethernet network adapter
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
//        bstr_t("SELECT * FROM Win32_VideoController"),
        bstr_t("SELECT Description FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
        );
    if( FAILED(hres) )
    {
        pSvc->Release();
        pLoc->Release();
        throw CSearchFailed();
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

        // Get the adapter type
        hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
        if( FAILED(hr) || vtProp.vt == VT_NULL )
        {
            pclsObj->Release();
            continue;
        }
        std::wstring wsDesc = vtProp.bstrVal;
        VariantClear(&vtProp);

        // Check that the any value was found?
        if( wsDesc.empty() )
        {
            pclsObj->Release();
            continue;
        }

        pclsObj->Release();

        // Return the found value
        unicode2Ascii(wsDesc, ssDesc);
        break;
    }

    pEnumerator->Release();

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    // Anything found?
    if( ssDesc.empty() )
    {
        throw CSearchFailed();
    }
    return ssDesc;
}

#endif // _WIN32


///////////////////////////////////////////////////////////////////////////////
// Linux version
// TODO: Implement the following part of code...
///////////////////////////////////////////////////////////////////////////////

#ifdef _LINUX

CGraficCardDesc::CGraficCardDesc()// : m_State(1)
{
    // No error...
    m_State = 0;
    m_bNeedUnitialize = false;
}


///////////////////////////////////////////////////////////////////////////////
//

CGraficCardDesc::~CGraficCardDesc()
{
}


///////////////////////////////////////////////////////////////////////////////
//

unsigned int CGraficCardDesc::getAdapterRAM()
{
    return 512;
}


///////////////////////////////////////////////////////////////////////////////
//

std::string CGraficCardDesc::getAdapterName()
{
    return std::string("");
}

#endif // _LINUX


///////////////////////////////////////////////////////////////////////////////
// Mac OS X version
// TODO: Implement the following part of code...
///////////////////////////////////////////////////////////////////////////////
    
#ifdef __APPLE__
    
CGraficCardDesc::CGraficCardDesc()// : m_State(1)
{
    // No error...
    m_State = 0;
    m_bNeedUnitialize = false;
}
    
    
//////////////////////////////////////////////////////////////////////////////
//
    
CGraficCardDesc::~CGraficCardDesc()
{
}
    
    
///////////////////////////////////////////////////////////////////////////////
//
    
unsigned int CGraficCardDesc::getAdapterRAM()
{
    return 512;
}
    
    
///////////////////////////////////////////////////////////////////////////////
//
    
std::string CGraficCardDesc::getAdapterName()
{
    return std::string("");
}

#endif // __APPLE__

} // namespace vr

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

