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

#ifdef _OPENMP
  #include <omp.h>
#endif
#include <QApplication>
#include "mainwindow.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QSettings>
#include <QProxyStyle>
#include <QCheckBox>

#include <app/CProductInfo.h>
#include <C3DimApplication.h>
#include <CSysInfo.h>

#include <AppVersion.h>
DECLARE_PRODUCT_INFO("3DimViewer", VER_MAJOR, VER_MINOR, VER_BUILD, app::EMPTY_NOTE)

#include <data/CDataStorage.h>
#include <VPL/Base/Logging.h>

// Proxy style for custom icon size
class MyProxyStyle : public QProxyStyle
{
public:
    int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const
    {
        int s = QProxyStyle::pixelMetric(metric, option, widget);
        if (metric == QStyle::PM_SmallIconSize) { // used by menu
            s = 20; // looks better than 24 px when icons are downsized from 32 px
        }
        if (metric == QStyle::PM_ToolBarIconSize) {
            s = 32;
        }
        return s;
    }
};


// Windows error handling
#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <signal.h>
#pragma comment(lib, "Psapi.lib")

// crt error handler
void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{

    if (NULL==function) // release mode    
    {
        VPL_LOG_INFO("Invalid parameter detected in C runtime function");
    }
    else
    {
        VPL_LOG_INFO("Invalid parameter detected in function " << C3DimApplication::wcs2ACP(function) << " file " << C3DimApplication::wcs2ACP(file) << " line " << line << " expression " << C3DimApplication::wcs2ACP(expression));
    }

#ifdef _DUMP_STACK
        C3DimApplication::getThreadStackTrace(GetCurrentThread(),NULL);
#endif

#ifndef _DEBUG
        // show error message        
        QMessageBox msgBox(QMessageBox::Critical,QCoreApplication::applicationName(),QObject::tr("Invalid parameter detected in C runtime function"),QMessageBox::Ok|QMessageBox::Abort);
        if (QMessageBox::Abort==msgBox.exec())
            abort();
#endif
}

// signal handler - we attempt to save call stack to log
void SignalHandler(int signal)
{
    switch(signal)
    {
        case SIGSEGV: VPL_LOG_INFO("Access violation"); break;
        case SIGABRT: VPL_LOG_INFO("Abort"); break;
        default:
            VPL_LOG_INFO("Signal " << signal);
    }
#ifdef _DUMP_STACK
    C3DimApplication::getThreadStackTrace(GetCurrentThread(),NULL);
#endif
}

LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionPtrs)
{
#ifdef _DUMP_STACK
    C3DimApplication::writeMiniDump(pExceptionPtrs);
#endif
  return EXCEPTION_CONTINUE_SEARCH; 
} 

void setErrorHandlers() 
{
    // set access violation handler
    typedef void (*SignalHandlerPointer)(int);
    signal(SIGSEGV , SignalHandler);
    signal(SIGABRT , SignalHandler);

    // set crt error handler
    OutputDebugStringA("Setting CRT error handler\n");

    // set handler for process
    //_invalid_parameter_handler oldHandler = 
    _set_invalid_parameter_handler(myInvalidParameterHandler);

    // set handler for process modules
    HANDLE hProcess = GetCurrentProcess();
    DWORD requiredSize = 0, secondRequiredSize = 0;
    if (!EnumProcessModules(hProcess, NULL, 0, &requiredSize))
        return;
    HMODULE* hModules = (HMODULE*)malloc(requiredSize);
    if (EnumProcessModules(hProcess, hModules, requiredSize, &secondRequiredSize)) 
    {
        int loadedModules = std::min(requiredSize, secondRequiredSize) / sizeof(HMODULE);
        for (int i = 0; i < loadedModules; i++) 
        {
            _invalid_parameter_handler *(__cdecl *_set_invalid_parameter_handler_function)(_invalid_parameter_handler) = (_invalid_parameter_handler *(__cdecl *)(_invalid_parameter_handler)) GetProcAddress(hModules[i], "_set_invalid_parameter_handler");
            if (NULL != _set_invalid_parameter_handler_function) 
            {
                _set_invalid_parameter_handler_function(myInvalidParameterHandler);
            }
        }
    }
    free(hModules);

    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
}

#endif // _WIN32


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources); // param is filename of resource file

    QCoreApplication::setOrganizationName("3Dim Laboratory s.r.o.");
    QCoreApplication::setOrganizationDomain("3dim-laboratory.cz");
    QCoreApplication::setApplicationName("3DimViewer");
    C3DimApplication app(argc, argv);
    // set application icon - however this affects running application only
    // TODO: add platform dependent application icon
    //       see http://developer.qt.nokia.com/doc/qt-4.8/appicon.html
    app.setWindowIcon(QIcon(":/icons/3dim.ico"));

    // Remove trailing 'bin' from the current working directory
    //QString workDir = QDir::currentPath();
    QString workDir = QCoreApplication::applicationDirPath();
    if( workDir.endsWith("bin",Qt::CaseInsensitive) )
    {
        workDir.truncate(workDir.size() - 4);
        QDir::setCurrent(workDir);
    }
    if( workDir.endsWith("MacOs",Qt::CaseInsensitive) )
    {
        workDir.truncate(workDir.size() - 6);
        QDir::setCurrent(workDir);
    }
#ifdef _WIN32
    QDir::setCurrent(workDir);
#endif
    qDebug() << QDir::currentPath();
    
#ifdef __APPLE__
    std::string osgPluginPath = (QDir::currentPath()+"/PlugIns/").toStdString();
    qDebug() << QString::fromStdString(osgPluginPath);
    osgDB::FilePathList osgPathList=osgDB::Registry::instance()->getLibraryFilePathList();
    osgPathList.push_front(osgPluginPath);
    osgDB::Registry::instance()->setLibraryFilePathList(osgPathList);
#endif
    
    QSettings settings;

    // Initialize the application storage
    // - Simple trick to prevent problems with two different instances of object factory
    //   used within the application and loadable plugins.
    APP_STORAGE.changeFactory(&STORABLE_FACTORY);

    // Initialize MDSTk logs
    bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(true)).toBool();
    if( bLoggingEnabled )
    {
        // Is the current workdir writable? 
        QFile file(workDir + QString("/3DimViewer.log"));
        if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
        {
            // Let's use user's home directory
            file.setFileName(QDir::homePath()  + QString("/3DimViewer.log"));
        }

        // MDSTk logs
        std::string ssLogFile = file.fileName().toStdString();
        VPL_LOG_ADD_FILE_APPENDER(ssLogFile);       
        VPL_LOG_INFO("Logging enabled");
    }

	{	// write app version to log file
		QString software = QCoreApplication::applicationName();
		app::CProductInfo info=app::getProductInfo();
		software += QString(" %1.%2.%3 %4").arg(info.getVersion().getMajorNum()).arg(QString::number(info.getVersion().getMinorNum())).arg(QString::number(info.getVersion().getBuildNum())).arg(QString::fromStdString(info.getNote()));
        #if defined (_WIN64) || defined (__APPLE__)
            software += " 64 bit";
        #endif
		VPL_LOG_INFO(software.toStdString());
    }

    // OpenMP settings
#ifdef _OPENMP    
    int iThreads = vpl::ompDefaultSettings();
    int nWantedThreads = settings.value("NumThreads",0).toInt();
    if (nWantedThreads>0)
    {
        omp_set_num_threads(nWantedThreads);
        iThreads = omp_get_max_threads();
        VPL_LOG_INFO("OpenMP enabled, #threads = " << iThreads << " (forced)");
    }
    else
        VPL_LOG_INFO("OpenMP enabled, #threads = " << iThreads);
#endif // _OPENMP

    // Set error handlers
#ifdef _WIN32
    setErrorHandlers();
#endif

    // find dir with translations
    QString localePath = settings.value("LocaleDir").toString();
    //if (localePath.isEmpty())
    {
        QDir localeDir=QDir::currentPath();
        if ((localeDir.entryList(QStringList("*.qm")).isEmpty()))
        {
            if (!localeDir.cd("locale"))
                localeDir.cd("translations");
        }
        localePath=localeDir.absolutePath();
        settings.setValue("LocaleDir",localePath); // save the choice
        settings.sync();
    }

    // application style sheet
    QString defaultStyleName = qApp->style()->objectName();
    settings.setValue("DefaultStyle",defaultStyleName);
    QString styleSheet = settings.value("StyleSheet",DEFAULT_STYLESHEET).toString();
    if (!styleSheet.isEmpty() && 0!=styleSheet.compare(defaultStyleName,Qt::CaseInsensitive))
    {   // load style sheet if present
        QFile file(workDir+"/styles/"+styleSheet);
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream in(&file);
            QString style=in.readAll();
            style += C3DimApplication::getPlatformStyleSheetAddon(workDir+"/styles/"+styleSheet);
            if (!style.isEmpty())
                app.setStyleSheet(style);
        }
    }

    // set custom icon size
    bool bigIcons=settings.value("BigIcons",true).toBool();
    if (bigIcons)
        app.setStyle(new MyProxyStyle);

    // load translations
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QTranslator qt, t;
    QString lngFile=settings.value("Language","NA").toString();
    if ("NA"==lngFile) // no language selected - set according to system locale
    {
        if (t.load( localePath+"/"+QLocale::system().name() ))
        {
            settings.setValue("Language",QLocale::system().name()+".qm"); // save the choice
            settings.sync();
            app.installTranslator(&t);
            if (qt.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
                app.installTranslator(&qt);
        }
    }
    else
    {
        if (lngFile.isNull() || lngFile.isEmpty())
        {
            // English - do nothing
        }
        else
        {
            if (t.load( localePath+"/"+lngFile ))
            {
                app.installTranslator(&t);
                if (qt.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
                    app.installTranslator(&qt);
            }
        }
    }

    // check program arguments
    QStringList arguments = QCoreApplication::arguments();
#ifdef _WIN32
    if (arguments.size()>1)
    {
        // there is an issue with windows/qt argument parsing where for input "c:\something\" qt gives us c:\something"
        // and breaks parsing of other arguments, therefore we get the original command line again and replace all
        // backslashes with forward slashes and generate a new argument list using windows api function
        LPWSTR wCmdLine = GetCommandLineW();
        if (NULL!=wCmdLine)
        {
            wchar_t* wCmdLineCustom = _wcsdup(wCmdLine);
            if (NULL!=wCmdLineCustom)
            {
                const size_t len = wcslen(wCmdLineCustom);
                for(int i=0;i<len;i++)
                    if (wCmdLineCustom[i]==L'\\')
                        wCmdLineCustom[i]=L'/';
                arguments.clear();
                //
                int nArgs = 0;
                LPWSTR *szArglist = CommandLineToArgvW(wCmdLineCustom, &nArgs);
                if (NULL!=szArglist)
                {
                    for(int i=0; i<nArgs; i++)
                    {
                        QString argument;
                        #ifdef _MSC_VER
                            argument = QString::fromUtf16((const ushort *)szArglist[i]);
                        #else
                            std::wstring tmp = szArglist[i];
                            argument =  QString::fromStdWString(tmp);
                        #endif                
                            arguments.push_back(argument);
                    }
                    LocalFree(szArglist);
                }
                free(wCmdLineCustom);
            }
        }
    }
#endif
    foreach(QString argument, arguments)
    {
        if (argument.endsWith(".vlm",Qt::CaseInsensitive))
        {
            app.setProperty("OpenVLM",argument);
        }
        else
        {
            if (argument.endsWith(".stl",Qt::CaseInsensitive))
            {
                app.setProperty("OpenSTL",argument);
            }
            else
            {
                QString appName = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
                // directory or file? look for dicom
                QFileInfo info(argument);
                if (info.isDir() || (info.isFile() && !argument.contains(appName,Qt::CaseInsensitive)))
                {
                    app.setProperty("OpenDICOM",argument);
                }
            }
        }
    }

	// init system info (and write to log)
    CSysInfo* pSysInfo = CSysInfo::instance();
    pSysInfo->init(); 

    // check configuration and show warning if below recommended
    bool bShowHWWarning = settings.value("HWConfigurationWarning",true).toBool();
    if (bShowHWWarning)
    {
        quint64 ram = pSysInfo->getTotalRam();
        quint64 vram = pSysInfo->getAdapterRam();
        quint64 textSize = pSysInfo->m_max3DTextureSize;
        if ((vram>0 && vram<512*1024*1024) ||   // VRAM less then 512MB
            (ram>0 && ram<896*1024*1024) ||     // RAM below 1GB (well, 896MB)
            (textSize<512))                     // 3D texture size at least 512
        {
            QMessageBox msgBox(QMessageBox::Information,QCoreApplication::applicationName(),QObject::tr("Your hardware doesn't meet the recommended configuration. You may experience reduced performance or the application won't run at all."),QMessageBox::Ok);
            QCheckBox *pCB = new QCheckBox(QObject::tr("Don't show again"));
            pCB->blockSignals(true); // this will ensure that the checkbox won't close the dialog
            msgBox.addButton(pCB,QMessageBox::ResetRole);
            msgBox.exec();
            if (Qt::Checked==pCB->checkState())
                settings.setValue("HWConfigurationWarning",false);
        }
    }
	int res = 0;
	{
		CPluginManager plugins;		
		{
			MainWindow w(NULL,&plugins);
			w.show();   
			res = app.exec();
		}
		// we can't deallocate plugins until MainWindow is deleted but we have to unload them before storage objects destruction
		plugins.unloadPlugins(true);
	}
	return res;
}
