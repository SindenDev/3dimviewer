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

#ifndef C3DIMAPPLICATION_H
#define C3DIMAPPLICATION_H

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QString>
#include <QDesktopServices>
#include <QUrl>
#include <QDialogButtonBox>
#include <QFileOpenEvent>
#include <QDateTime>
#include <QFileDialog>
//#include <CPreferencesDialog.h> // default values of some settings
#include <cpreferencesdialog.h> // default values of some settings
#include <VPL/Base/Logging.h>
#include <sstream>
#include <qtcompat.h>
#include <zip/zip.h>
#include <zip/unzip.h>

#ifdef _WIN32
#include <time.h>

//! Retrieve application error log entries
static void getRecentEventLogAppEntries()
{
    BYTE* buffer = new BYTE[0x7ffff]; // max buffer size specified in windows api docs
    if (NULL==buffer) return;
	EVENTLOGRECORD* pelr = (EVENTLOGRECORD*) buffer;
	HANDLE hLog = ::OpenEventLog(NULL, L"Application");    
	if (NULL!=hLog)
    {
        VPL_LOG_INFO("Checking Windows Error Log");
        int nLogged = 0;
	    while (nLogged < 4) // log only 4 latest errors
        {
            // perform a false call to get the real structure size
            DWORD dwBytesToRead = 0;
	        DWORD dwBytesRead = 0;
	        DWORD dwBytesNeeded = 0;
		    int ret = ReadEventLog(hLog,
						       EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
						       0,
						       pelr,
						       dwBytesToRead,
						       &dwBytesRead,
						       &dwBytesNeeded);
            // perform call with correct structure size to retrieve just one record
            dwBytesToRead = std::min(dwBytesNeeded,(DWORD)0x7ffff);
	        dwBytesRead = 0;
	        dwBytesNeeded = 0;
            ret = ReadEventLog(hLog,
						       EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
						       0,
						       pelr,
						       dwBytesToRead,
						       &dwBytesRead,
						       &dwBytesNeeded);
		    if (0 == ret)
            {
                int status = GetLastError();
			    break;
            }
            // stop enumeration when the records are too old
            QDateTime dt = QDateTime::fromTime_t(pelr->TimeGenerated);
            if (dt.isValid())
            {
                if (dt.daysTo(QDateTime::currentDateTime())>7)
                    break;
            }
            // events of interest
            if (1001==pelr->EventID || 1000==pelr->EventID)
            {    
                QString all;
                wchar_t* pStr = (wchar_t*)((BYTE*)pelr+pelr->StringOffset);
                for(int i = 0; i < pelr->NumStrings; i++)
                {
                    QString str;
                    #ifdef _MSC_VER
                        str = QString::fromUtf16((const ushort *)pStr);
                    #else
                        std::wstring tmp = pStr;
                        str =  QString::fromStdWString(tmp);
                    #endif
                    pStr+=wcslen(pStr)+1;
                    if (!all.isEmpty())
                        all += "\n";
                    all+=str;
                }
                if (all.contains("3DimViewer",Qt::CaseInsensitive))
                {
                    nLogged++;
                    //qDebug() << "EventID " << pelr->EventID << " Type " << pelr->EventType;
                    VPL_LOG_INFO("EventID " << pelr->EventID << " Type " << pelr->EventType);
                    tm* lt = _localtime32((__time32_t *)&(pelr->TimeGenerated));
                    if (NULL!=lt)
                    {                        
                        std::string strTime = asctime(lt);
                        //qDebug() << "Generated " << QString::fromStdString(strTime);
                        VPL_LOG_INFO("Generated " << strTime);
                    }
                    //qDebug() << all;
                    VPL_LOG_INFO(all.toStdString());
                }
            }
	    }
        ::CloseEventLog(hLog);
    }
    delete[] buffer;
}


#define _DUMP_STACK
#ifdef _DUMP_STACK

#include <DbgHelp.h>
#include <psapi.h>
#include <process.h> // beginthreadex
#pragma comment(lib, "Psapi.lib")

typedef BOOL(__stdcall *STACKWALK64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID,
  PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64,
  PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
typedef BOOL(__stdcall *SYMGETLINEFROMADDR64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
typedef BOOL(__stdcall *SYMGETMODULEINFO64)(HANDLE, DWORD64, PIMAGEHLP_MODULE64);
typedef BOOL(__stdcall *SYMGETSYMFROMADDR64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
typedef BOOL(__stdcall *SYMINITIALIZE)(HANDLE, PSTR, BOOL);
typedef DWORD(__stdcall *SYMSETOPTIONS)(DWORD);
typedef BOOL(__stdcall *MINIDUMPWRITEDUMP)(HANDLE,DWORD,HANDLE,MINIDUMP_TYPE,PMINIDUMP_EXCEPTION_INFORMATION,PMINIDUMP_USER_STREAM_INFORMATION,PMINIDUMP_CALLBACK_INFORMATION);  


static unsigned __stdcall miniDumpThread( void* pArguments )
{
    HMODULE debugDll = ::LoadLibrary(L"dbghelp.dll");
    if (NULL==debugDll) return 0;

    MINIDUMPWRITEDUMP miniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(debugDll,"MiniDumpWriteDump");
    if (NULL!=miniDumpWriteDump)
    {
        HANDLE hProcess = ::GetCurrentProcess();
        DWORD dwProcessId = GetCurrentProcessId();
	
        QString dataLocation = HOMELOCATION();
        dataLocation+="/3DVCrashDump.mdmp";
        
        std::string str = dataLocation.toStdString();

        // create the file
        HANDLE hFile = ::CreateFileA( str.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if (hFile!=INVALID_HANDLE_VALUE)
        {            
		    MINIDUMP_EXCEPTION_INFORMATION mdei; 
		    mdei.ThreadId           = GetCurrentThreadId(); 
		    mdei.ExceptionPointers  = (PEXCEPTION_POINTERS)pArguments; 
		    mdei.ClientPointers     = FALSE; 

            //MINIDUMP_CALLBACK_INFORMATION mci; 
            //mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
            //mci.CallbackParam       = 0; 
            
            MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(//MiniDumpWithPrivateReadWriteMemory | 
                                                        MiniDumpWithIndirectlyReferencedMemory | 
                                                        MiniDumpScanMemory |
                                                        MiniDumpWithDataSegs | 
                                                        MiniDumpWithHandleData |
                                                        //MiniDumpWithFullMemoryInfo | 
                                                        MiniDumpWithThreadInfo | 
                                                        MiniDumpWithProcessThreadData |
                                                        MiniDumpWithUnloadedModules ); 

            // write the dump
            BOOL bOK = miniDumpWriteDump( hProcess, dwProcessId, hFile, mdt, NULL!=pArguments?&mdei:NULL, NULL, NULL/*&mci*/ );
            DWORD lastErr = GetLastError();

            if (bOK)
            {
                VPL_LOG_INFO("Crash dump saved to: " << str);
            }
            ::CloseHandle(hFile);
        }
    }
    FreeLibrary(debugDll);
	return 0;
} 


#endif _DUMP_STACK
#endif _WIN32


//! Application with extended functionality
class C3DimApplication : public QApplication
{
    Q_OBJECT

public:
	//! Constructor
    C3DimApplication(int &argc, char** argv) : QApplication(argc,argv) {}

#if(1)
	//! Unhandled exception handling
    virtual bool notify(QObject* receiver, QEvent * event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch(std::exception& e)
        {
            qDebug() << "Exception: " << e.what();
			VPL_LOG_WARN("Exception" << e.what());
            QString message(e.what());
            if (0==message.compare("bad allocation",Qt::CaseInsensitive))
                message=tr("Out of memory!");
            message="<b>"+message+"</b>";

			// event info
			if (NULL!=event)
			{
				QString eventInfo;
				if (NULL!=receiver && NULL!=receiver->metaObject())
					eventInfo = QLatin1String(receiver->metaObject()->className())+": event "+QString::number(event->type()); 
				else
					eventInfo = QString::number(event->type()); 
				//message+="\n\n"+eventInfo;
				VPL_LOG_INFO(eventInfo.toStdString());
			}

#ifdef _WIN32
            // get recent application log entries (can contain previous crashes)
            getRecentEventLogAppEntries();
#endif

            message.replace('\n',"<br>");

			// show message box
            QMessageBox msgBox(QMessageBox::Critical,QApplication::applicationName(),message,QMessageBox::Ok|QMessageBox::Abort);
			// if logging is enabled, add a button to show log
			{
				QSettings settings;
				bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(true)).toBool();
				if (bLoggingEnabled)
				{
					QPushButton *pButtonLog = new QPushButton(tr("Show Log..."));
					msgBox.addButton(pButtonLog,QMessageBox::HelpRole);
					QObject::disconnect(pButtonLog,0,0,0); // button click would close the message box - disconnect signals
					QObject::connect(pButtonLog,SIGNAL(clicked()),this,SLOT(showLog())); // connect our own
				}
			}
            if (QMessageBox::Abort==msgBox.exec())
            {
                 QCoreApplication::exit(-1);
            }
        }
        return false;
    }
#endif
    
protected:
	QStringList getLogPaths()
	{
		QString dir = QDir::currentPath();
		QList<QString> paths;
		{
			QFileInfo fi(dir + QString("/3DimViewer.log"));
			QFileInfo fi2(QDir::homePath() + QString("/3DimViewer.log"));
			if (fi.exists() && fi2.exists())
			{
				QDateTime dt = fi.lastModified();
				QDateTime dt2 = fi2.lastModified();
				//qDebug() << dt.toString();
				//qDebug() << dt2.toString();
				if (dt2>dt)
				{
					paths.push_back(fi2.filePath());
					paths.push_back(fi.filePath());
				}
				else
				{
					paths.push_back(fi.filePath());
					paths.push_back(fi2.filePath());
				}
			}
			else
			{
				if (fi.exists())
					paths.push_back(fi.filePath());
				if (fi2.exists())
					paths.push_back(fi2.filePath());
			}
		}
		if (!paths.empty())
			return paths;
		return QStringList();
	}
    QString readLog()
    {        
		QString text;
		QStringList paths = getLogPaths();
		if (paths.isEmpty())
			return text;
		// Is the current workdir writable? 
		QFile file(paths[0]);
		if( !file.open(QIODevice::ReadOnly | QIODevice::Text) && paths.size()>1)
		{
			// Let's use user's home directory
			file.setFileName(paths[1]);
			file.open(QIODevice::ReadOnly | QIODevice::Text);
		}
		if (file.isOpen())
		{
			QTextStream in(&file);
            text=in.readAll();			
			file.close();
		}
        return text;
    }

	//! Get local app data path
	QString localAppDataPath()
	{
#ifdef _WIN32
		char* appData = getenv("LOCALAPPDATA");
		if (NULL!=appData && 0!=appData[0])
		{
			std::wstring wName = C3DimApplication::ACP2wcs(appData);
			return QString::fromUtf16( (const ushort*)wName.c_str() );
		}
#endif
		return "";
	}
    
signals:
	//! Signal emitted when style sheet is changed
	void styleSheetChanged();
    
public slots:
	//! Compress all crash dumps into one file
	void saveErrorInfo2Zip()
	{
		QStringList lstCrashDumps;
		QString wsCrashDumpPath = localAppDataPath() + "/CrashDumps";
		QDir dir(wsCrashDumpPath);
		if (!dir.exists())
			return;
		lstCrashDumps = dir.entryList(QStringList("3DimViewer*.*"));		
		if (!lstCrashDumps.empty())
		{			
			QSettings settings;
			QString previousDir = settings.value("DmpDir", HOMELOCATION()).toString();
			if (!previousDir.endsWith('/') && !previousDir.endsWith('\\'))
				previousDir+='/';
			previousDir+="3DVCrash";
			/*if(0)
			{
				char* user = getenv("USER");
				if (NULL==user || 0==user[0])
					user = getenv("USERNAME");
				if (NULL!=user && 0!=user[0])
                {
					std::wstring wName = C3DimApplication::ACP2wcs(user);
					previousDir += QString::fromUtf16( (const ushort*)wName.c_str() );
				}
			}*/
			QString fileName = QFileDialog::getSaveFileName(NULL, tr("Please choose a destination file name."), previousDir, tr("ZIP archive (*.zip)"));
			if (fileName.isEmpty())
				return;
			QFileInfo pathInfo(fileName);
			settings.setValue("DmpDir", pathInfo.dir().absolutePath());

			// save crash dumps
			std::map<QString, int> lstCompress;
			foreach(QString dump, lstCrashDumps)
				lstCompress[dir.absoluteFilePath(dump)] = 1;
			// save logs
			QStringList logs = getLogPaths();
			foreach(QString log, logs)
				lstCompress[log] = 1;
			// save app generated crash dump
			QDir dataDir(HOMELOCATION());
			if (dataDir.exists("3DVCrashDump.mdmp"))
				lstCompress[dataDir.absoluteFilePath("3DVCrashDump.mdmp")] = 1;
			// create zip archive
			createZipFile(fileName,lstCompress,false);
		}
	}
	//! Show dialog with application log contents
	void showLog(const QString& message)
	{
		QStringList lstCrashDumps;
		QString wsCrashDumpPath = localAppDataPath() + "/CrashDumps";
		{
			QDir dir(wsCrashDumpPath);
			if (dir.exists())
				lstCrashDumps = dir.entryList(QStringList("3DimViewer*.*"));
			QDir dataDir(HOMELOCATION());
			if (dataDir.exists("3DVCrashDump.mdmp"))
				lstCrashDumps.push_back("3DVCrashDump.mdmp");
		}
        const double dpiFactor = getDpiFactor();
		QDialog dlg(NULL,Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
        dlg.setWindowTitle(QApplication::applicationName());
        dlg.setMinimumSize(720*dpiFactor,480*dpiFactor);
		dlg.setSizeGripEnabled(true);
        QVBoxLayout* pLayout = new QVBoxLayout();
        dlg.setLayout(pLayout);
        QLabel* pLabelInfo = new QLabel;
        pLabelInfo->setText(message);
        pLayout->addWidget(pLabelInfo);
        QTextEdit* pTextEdit = new QTextEdit();
		pTextEdit->setPlainText(readLog());
        pLayout->addWidget(pTextEdit);
        QDialogButtonBox* pButtonBox = new QDialogButtonBox;
        pButtonBox->setStandardButtons(QDialogButtonBox::Ok);
		if (!lstCrashDumps.empty())
		{
			QPushButton* pButtonSendDump = new QPushButton(tr("Save Crash Report..."));
			pButtonBox->addButton(pButtonSendDump,QDialogButtonBox::HelpRole);		
			connect(pButtonSendDump,SIGNAL(clicked()),this,SLOT(saveErrorInfo2Zip()));
		}
        pButtonBox->setCenterButtons(true);
        connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
        pLayout->addWidget(pButtonBox);
        dlg.exec();
	}
	void showLog()
	{
		showLog(tr("Application Log:"));
	}        
public:
	//! Replaces application style sheet
    void setStyleSheetEx ( const QString & sheet )
    {        
        QApplication::setStyleSheet(sheet);
        emit styleSheetChanged();
    }

	static double getDpiFactor() 
	{
		double sizeFactor = 1;
		QDesktopWidget* pDesktop = QApplication::desktop();
		if (NULL!=pDesktop)
			sizeFactor = std::max(1.0,pDesktop->logicalDpiX()/96.0);
		return sizeFactor;
	}

	static void adjustControlFontSize(QWidget *pWidget, double factor)
	{
		if (NULL!=pWidget)
		{
			pWidget->ensurePolished();		
			QFont f(pWidget->font());
			f.setPointSizeF(pWidget->font().pointSizeF() * factor);
			pWidget->setFont(f);
		}
	}

    //! loads platform specific stylesheet addon
    static QString getPlatformStyleSheetAddon(QString sFileName)
    {
        QString result;
        int indexExt = sFileName.indexOf(".qss",0,Qt::CaseInsensitive);
        if (indexExt>0)
        {
            QString wsLeftName = sFileName.left(indexExt);
    #if defined(Q_WS_WIN) || defined(Q_OS_WIN)
            wsLeftName+="_win.qss";
    #elif defined(Q_WS_MAC) || defined(Q_OS_MAC)
            wsLeftName+="_mac.qss";
    #else // linux
            wsLeftName+="_linux.qss";
    #endif
            QFile file(wsLeftName);
            if (file.open(QIODevice::ReadOnly))
            {
                QTextStream in(&file);
                result=in.readAll();
            }
        }
        return result;
    }

    //! loads hi dpi stylesheet addon
    static QString getHiDpiStyleSheetAddon(QString sFileName)
    {
        QString result;
        int indexExt = sFileName.indexOf(".qss",0,Qt::CaseInsensitive);
        if (indexExt>0)
        {
            QString wsLeftName = sFileName.left(indexExt);
            wsLeftName+="_hidpi.qss";
            QFile file(wsLeftName);
            if (file.open(QIODevice::ReadOnly))
            {
                QTextStream in(&file);
                result=in.readAll();
            }
        }
        return result;
    }

#ifdef WIN32
    //! Windows conversion of wchar string to char string in ACP (uses shortname if conversion is not possible)
    static std::string wcs2ACP(const std::wstring &filename)
    {
        return wcs2ACP(filename.c_str());
    }

    //! Windows conversion of wchar string to char string in ACP (uses shortname if conversion is not possible)
    static std::string wcs2ACP(const wchar_t *filename)
    {
        std::string convFilename;
        if (NULL==filename) 
            return convFilename;

        {
            BOOL bUsedDefaultChar = false; // has to be BOOL!
            // get buffer size
            int size=WideCharToMultiByte(CP_ACP,0,filename,wcslen(filename),0,0,0,&bUsedDefaultChar);
            if (size>0)
            {
                // convert
                char* buffer=new char[size+1];
                int sizeC=WideCharToMultiByte(CP_ACP,0,filename,wcslen(filename),buffer,size,0,&bUsedDefaultChar);
                if (sizeC>0)
                {
                    assert(sizeC<size+1);
                    buffer[sizeC]=0;
                    convFilename=buffer;
                }
                delete[] buffer;
            }
            // conversion wasn't possible, get short path
            if (bUsedDefaultChar)
            {
                wchar_t wcsTmpStr[_MAX_PATH]={};
                unsigned int result = GetShortPathNameW(filename,wcsTmpStr,_MAX_PATH);
                if (result>0 && result<_MAX_PATH)
                {
                    _wcslwr(wcsTmpStr); // because openmesh write checks for lower case extension only
                    std::wstring tmp = wcsTmpStr;
                    return wcs2ACP(wcsTmpStr);
                }
                else
                {
                    #ifdef _MSC_VER
                        qDebug() << "Couldn't get ansi name for " << QString::fromUtf16((const ushort *)filename);
                    #else
                        qDebug() << "Couldn't get ansi name for " << QString::fromStdWString(filename);
                    #endif
                    convFilename.clear();
                }
            }
        }
        return convFilename;
    }
    static std::wstring ACP2wcs(const char *filename)
    {
        std::wstring convFilename;
        if (NULL == filename)
            return convFilename;

        int size = MultiByteToWideChar(CP_ACP, 0, filename, -1, 0, 0);
        if (size>0)
        {
            wchar_t* buffer = new wchar_t[size + 1];
            memset(buffer, 0, sizeof(wchar_t)*(size + 1));
            int sizeC = MultiByteToWideChar(CP_ACP, 0, filename, -1, buffer, size);
            if (sizeC>0)
            {
                assert(sizeC<size + 1);
                buffer[sizeC] = 0;
                convFilename = buffer;
            }
            delete[] buffer;
        }
        return convFilename;
    }
    static std::wstring ACP2wcs(const std::string& filename)
    {
        return ACP2wcs(filename.c_str());
    }
#endif

    static std::string getAnsiName(const QString& filename)
    {
        #ifdef WIN32
            // OpenMesh needs path in ACP
            std::wstring uniName = (const wchar_t*)filename.utf16();
            std::string ansiName = wcs2ACP(uniName);
            if (ansiName.empty()) 
            {
                // couldn't convert to ansi, create an empty file to be able
                // to get its short name
                QFile file( filename );
                if ( file.open( QIODevice::WriteOnly ) )
                {
                    file.close();
                    ansiName = wcs2ACP(uniName);
                }
            }
        #else
            // 2012/03/12: Modified by Majkl (Linux compatible code)
            //std::string ansiName = filename.toUtf8();
            std::string ansiName = filename.toStdString();
        #endif
            return ansiName;
    }

#ifdef __APPLE__
protected:
    //! Reimplements the event() method to handle FileOpen event
    bool event(QEvent * event)
    {
        switch( event->type() )
        {
            case QEvent::FileOpen:
                loadFile(static_cast<QFileOpenEvent *>(event)->file());
                return true;
            default:
                return QApplication::event(event);
        }
    }

private:
    // Handles Apple Open Event
    void loadFile(const QString &fileName)
    {
        if (fileName.endsWith(".vlm",Qt::CaseInsensitive))
        {
            setProperty("OpenVLM",fileName);
        }
        if (fileName.endsWith(".stl",Qt::CaseInsensitive))
        {
            setProperty("OpenSTL",fileName);
        }
    }
#endif // __APPLE__

// windows specific implementation of call stack trace
// https://github.com/DavidKinder/Windows-Inform7/blob/master/Inform7/StackTrace.cpp

#ifdef _WIN32
#ifdef _DUMP_STACK

#ifdef _M_IX86
  // Disable global optimization and ignore /GS warning caused by inline assembly.
  #pragma optimize( "g", off )
  #pragma warning( push )
  #pragma warning( disable : 4748 )
#endif

static bool writeMiniDump(PEXCEPTION_POINTERS pExceptionPtrs)
{
    HMODULE debugDll = ::LoadLibrary(L"dbghelp.dll");
    if (NULL==debugDll) return false;
    MINIDUMPWRITEDUMP miniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(debugDll,"MiniDumpWriteDump");
    // write minidump if possible 
    // http://www.debuginfo.com/articles/effminidumps.html
    // http://msdn.microsoft.com/en-us/library/ms680360%28VS.85%29.aspx
    // http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus
    if (NULL!=miniDumpWriteDump)
    {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL,0,miniDumpThread,pExceptionPtrs,0,0);
        WaitForSingleObject(hThread,INFINITE);
        CloseHandle(hThread);
    }
    FreeLibrary(debugDll);
    return true;
}

static bool getThreadStackTrace(__in HANDLE hThread, PEXCEPTION_POINTERS pExceptionPtrs)
{
    SYSTEM_INFO si={};
    ::GetSystemInfo(&si);
    if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL && si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)
        return false;

    HMODULE debugDll = ::LoadLibrary(L"dbghelp.dll");
    if (NULL==debugDll) return false;

    STACKWALK64 stackWalk64 = (STACKWALK64)::GetProcAddress(debugDll,"StackWalk64");
    PFUNCTION_TABLE_ACCESS_ROUTINE64 symFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64)::GetProcAddress(debugDll,"SymFunctionTableAccess64");
    SYMGETLINEFROMADDR64 symGetLineFromAddr64 = (SYMGETLINEFROMADDR64)::GetProcAddress(debugDll,"SymGetLineFromAddr64");
    PGET_MODULE_BASE_ROUTINE64 symGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64)::GetProcAddress(debugDll,"SymGetModuleBase64");
    SYMGETMODULEINFO64 symGetModuleInfo64 = (SYMGETMODULEINFO64)::GetProcAddress(debugDll,"SymGetModuleInfo64");
    SYMGETSYMFROMADDR64 symGetSymFromAddr64 = (SYMGETSYMFROMADDR64)::GetProcAddress(debugDll,"SymGetSymFromAddr64");
    SYMINITIALIZE symInitialize = (SYMINITIALIZE)::GetProcAddress(debugDll,"SymInitialize");
    SYMSETOPTIONS symSetOptions = (SYMSETOPTIONS)::GetProcAddress(debugDll,"SymSetOptions");
    MINIDUMPWRITEDUMP miniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(debugDll,"MiniDumpWriteDump");
    if (NULL==stackWalk64 ||
        NULL==symFunctionTableAccess64 ||
        NULL==symGetModuleBase64 ||
        NULL==symGetModuleInfo64 ||
        NULL==symInitialize ||
        NULL==symSetOptions ||
        NULL==symGetLineFromAddr64 ||
        NULL==symGetSymFromAddr64)
    {
        FreeLibrary(debugDll);
        return false;
    }

    CONTEXT threadContext = {};
    threadContext.ContextFlags = CONTEXT_FULL;
    if (hThread==::GetCurrentThread())
        RtlCaptureContext(&threadContext);
    else
        if (GetThreadContext(hThread, &threadContext) == 0)
        {
            std::cout << "Error: GetThreadContext() failed with error ID " << GetLastError() << endl;
            FreeLibrary(debugDll);
            return false;
        }

    //initialize stack frame
    DWORD MachineType = 0;
    STACKFRAME64 StackFrame = {};

#ifdef _WIN64
    MachineType                 = IMAGE_FILE_MACHINE_AMD64;
    StackFrame.AddrPC.Offset    = threadContext.Rip;
    StackFrame.AddrPC.Mode      = AddrModeFlat;
    StackFrame.AddrFrame.Offset = threadContext.Rbp;
    StackFrame.AddrFrame.Mode   = AddrModeFlat;
    StackFrame.AddrStack.Offset = threadContext.Rsp;
    StackFrame.AddrStack.Mode   = AddrModeFlat;
#else
    MachineType                 = IMAGE_FILE_MACHINE_I386;
    StackFrame.AddrPC.Offset    = threadContext.Eip;
    StackFrame.AddrPC.Mode      = AddrModeFlat;
    StackFrame.AddrFrame.Offset = threadContext.Ebp;
    StackFrame.AddrFrame.Mode   = AddrModeFlat;
    StackFrame.AddrStack.Offset = threadContext.Esp;
    StackFrame.AddrStack.Mode   = AddrModeFlat;
#endif

    HANDLE hProcess = ::GetCurrentProcess();
    DWORD dwProcessId = GetCurrentProcessId();

    // Set symbol options
    (*symSetOptions)(SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);

    // Load any symbols
    if ((*symInitialize)(hProcess,NULL,TRUE) == 0)
    {

    }    

    VPL_LOG_INFO("Thread " << GetCurrentThreadId());
    // enumerate all the frames in the stack
    for (int i=0; i<20; i++) // limit to N frames
    {
        if (!stackWalk64( MachineType, hProcess, hThread, &StackFrame, &threadContext, NULL, symFunctionTableAccess64, symGetModuleBase64, NULL ))
        {
            // in case it failed or we have finished walking the stack.
            std::cout << "Error: StackWalk64() failed with error ID " << GetLastError() << endl;
            break;
            // return false;
        }

        IMAGEHLP_MODULE64 info={};
        info.SizeOfStruct = sizeof(info);
        BOOL gotModuleInfo = (symGetModuleInfo64)(hProcess,StackFrame.AddrPC.Offset,&info);

        // Set up a symbol buffer
        BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64)+512];
        memset(symbolBuffer,0,sizeof(IMAGEHLP_SYMBOL64)+512);
        PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)symbolBuffer;
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
        symbol->MaxNameLength = 512;

        // Get the symbol
        DWORD64 displacement1 = 0;
        BOOL gotSymbol = (*symGetSymFromAddr64)(hProcess,StackFrame.AddrPC.Offset,&displacement1,symbol);

        // Get the file and line number
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof line;
        DWORD displacement2 = 0;
        BOOL gotLine = (*symGetLineFromAddr64)(hProcess,StackFrame.AddrPC.Offset,&displacement2,&line);

        if ( StackFrame.AddrPC.Offset != 0 )
        {
            std::stringstream ss;
            ss << std::hex << StackFrame.AddrPC.Offset-info.BaseOfImage << " (" << StackFrame.AddrPC.Offset << ")";
            if (gotSymbol)
                ss << " " << symbol->Name;
            if (gotLine)
                ss << " " << std::dec << line.LineNumber;
            VPL_LOG_INFO(i << " " << info.ModuleName << " " << ss.str().c_str());
        }
        else
        {
            // Base reached.
            break;
        }
    }

    // write minidump if possible 
    // http://www.debuginfo.com/articles/effminidumps.html
    // http://msdn.microsoft.com/en-us/library/ms680360%28VS.85%29.aspx
    // http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus
    if (NULL!=miniDumpWriteDump && NULL!=pExceptionPtrs)
    {
        HANDLE hThread = (HANDLE)_beginthreadex(NULL,0,miniDumpThread,pExceptionPtrs,0,0);
        WaitForSingleObject(hThread,INFINITE);
        CloseHandle(hThread);
    }

    FreeLibrary(debugDll);
    return true;
}


#ifdef _M_IX86
  #pragma warning( pop )
  #pragma optimize( "g", on )
#endif

#endif _DUMP_STACK
#endif _WIN32

static bool createZipFile(QString destFile, std::map<QString, int> paths, bool bDecompressZip)
	{
	#ifdef WIN32
		std::wstring uniPath = (const wchar_t*)destFile.utf16();
		std::string destinationPath = wcs2ACP(uniPath);
	#else
		std::string destinationPath = destFile.toStdString();
	#endif
		zipFile zf = zipOpen(destinationPath.c_str(), APPEND_STATUS_CREATE);
		if (NULL == zf)
		{
			return false;
		}

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		bool ok = true;
		for (auto it = paths.begin(), eit = paths.end(); it != eit && ok; ++it)
		{
			QFile f(it->first);
			QFileInfo inf(it->first);
			const int count = it->second;
			Q_ASSERT(count>0);
			bool bUZOk = false;
			if (bDecompressZip && 0 == inf.suffix().compare("zip", Qt::CaseInsensitive))
			{
	#ifdef WIN32
				std::wstring uniPath = (const wchar_t*)(it->first).utf16();	
				std::string zipPath = wcs2ACP(uniPath);
	#else
				std::string zipPath = it->first.toStdString();
	#endif
				unzFile hZIP = unzOpen(zipPath.c_str());
				if (hZIP)
				{
	#define CUSTOM_MAX_PATH 4096
					char pFileName[CUSTOM_MAX_PATH] = { };
					bool bFile = bUZOk = (UNZ_OK == unzGoToFirstFile(hZIP));
					while (bUZOk && bFile)
					{
						unz_file_info fi = { };
						memset(pFileName, 0, CUSTOM_MAX_PATH*sizeof(char));
						if (unzGetCurrentFileInfo(hZIP, &fi, pFileName, CUSTOM_MAX_PATH, NULL, 0, NULL, 0) == UNZ_OK)
						{
	#undef CUSTOM_MAX_PATH
							std::string s = pFileName;
							QString file = QString::fromStdString(s);
							if (file.endsWith('/') || file.endsWith('\\'))
							{
								// ignore directories
							}
							else
							{
								if (fi.uncompressed_size>0)
								{
									bUZOk = false;
									char* pDecompressed = new char[fi.uncompressed_size];
									if (NULL != pDecompressed)
									{
										int nRead = 0;
										if (unzOpenCurrentFile(hZIP) == UNZ_OK)
										{
											nRead = unzReadCurrentFile(hZIP, pDecompressed, fi.uncompressed_size);
											if (UNZ_CRCERROR == unzCloseCurrentFile(hZIP))
											{
												nRead = 0;
											}
										}
										if (nRead>0)
										{
											zip_fileinfo zfi = { };
											QDateTime t = inf.created();
											zfi.tmz_date.tm_sec = t.time().second();
											zfi.tmz_date.tm_min = t.time().minute();
											zfi.tmz_date.tm_hour = t.time().hour();
											zfi.tmz_date.tm_mday = t.date().day();
											zfi.tmz_date.tm_mon = t.date().month() - 1;
											zfi.tmz_date.tm_year = t.date().year();
											for (int m = 0; m < count; m++)
											{
												std::string fnX = pFileName;
												if (count>1)
												{
													int idxSlash = (int)std::max(fnX.find_last_of("/"), fnX.find_last_of("\\"));
													int idxDot = fnX.find_last_of(".");
													if (idxDot>idxSlash)
													{
														std::stringstream ss;
														ss << "_" << m + 1;
														fnX.insert(idxDot, ss.str());
													}
													else
													{
														m = count;
													}
												}
												if (ZIP_OK == zipOpenNewFileInZip(zf, fnX.c_str(), &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, fi.flag))
												{
													bUZOk = true;
													if (zipWriteInFileInZip(zf, fi.uncompressed_size == 0 ? "" : pDecompressed, fi.uncompressed_size))
													{
														bUZOk = false;
													}

													if (zipCloseFileInZip(zf))
													{
														bUZOk = false;
													}
												}
											}
										}
										delete[] pDecompressed;
									}
								}
							}
						}
						bFile = (UNZ_OK == unzGoToNextFile(hZIP));
					}
					unzClose(hZIP);
				}
			}
			if (!bUZOk)
			{
				if (f.open(QIODevice::ReadOnly))
				{
					QByteArray arr = f.readAll();
					long size = arr.size();

					QString wsFileName = inf.fileName();
					zip_fileinfo zfi = { };
					QDateTime t = inf.created();
					zfi.tmz_date.tm_sec = t.time().second();
					zfi.tmz_date.tm_min = t.time().minute();
					zfi.tmz_date.tm_hour = t.time().hour();
					zfi.tmz_date.tm_mday = t.date().day();
					zfi.tmz_date.tm_mon = t.date().month() - 1;
					zfi.tmz_date.tm_year = t.date().year();
					std::string utf8Name = wsFileName.toStdString();
					for (int m = 0; m < count; m++)
					{
						std::string fnX = utf8Name;
						if (count > 1)
						{
							int idxSlash = (int)std::max(fnX.find_last_of("/"), fnX.find_last_of("\\"));
							int idxDot = fnX.find_last_of(".");
							if (idxDot > idxSlash)
							{
								std::stringstream ss;
								ss << "_" << m + 1;
								fnX.insert(idxDot, ss.str());
							}
							else
							{
								m = count;
							}
						}
						if (ZIP_OK == zipOpenNewFileInZip(zf, fnX.c_str(), &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 1 << 11 /* UTF8 filename */))
						{
							if (zipWriteInFileInZip(zf, size == 0 ? "" : arr.data(), size))
							{
								ok = false;
							}

							if (zipCloseFileInZip(zf))
							{
								ok = false;
							}
						}
						else
						{
							ok = false;
						}
					}
				}
				else
				{
					ok = false;
				}
			}
		}

		if (zipClose(zf, NULL))
		{
			ok = false;
		}

		QApplication::restoreOverrideCursor();
		return ok;
	}
};

#define APP3DIM  (qobject_cast<C3DimApplication*>(qApp))

#endif // C3DIMAPPLICATION_H
