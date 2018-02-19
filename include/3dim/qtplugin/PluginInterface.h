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

#ifndef _QT_PLUGININTERFACE_H
#define _QT_PLUGININTERFACE_H

#include <QtPlugin>
#include <QTranslator>
#include <QTextCodec>
#include <QApplication>
#include <QLocale>
#include <QLibraryInfo>
#include <QFileInfo>

class QStringList;
class QRect;
class QString;
class QPainter;
class QPoint;
class QMenu;
class QToolBar;
class QWidget;
class QAction;

#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>
#include <VPL/Base/Exception.h>

#include <data/CDataStorage.h>
#include <osg/CAppMode.h>
#include <render/CVolumeRenderer.h>

#include "CPluginInfo.h"

#ifndef PLUG_APP_SIGNATURE
	#define PLUG_APP_SIGNATURE	"OpenSource"
#endif

class PluginInterface;

///////////////////////////////////////////////////////////////////////////////
//! Class manages application interface.

class CAppBindings : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CAppBindings);

    //! Exception thrown in case of an error.
    VPL_DECLARE_EXCEPTION(CNullAppBindings, "Trying to access application bindings via NULL pointer")

public:
    //! Default constructor.
    CAppBindings()
        : vpl::base::CObject()
        , m_pAppMode(NULL)
        , m_pStorage(NULL)
        , m_pRenderer(NULL)
        , m_pParent(NULL)
        , m_pPlugin(NULL)
		, m_vplSignals(NULL)
    {
    }

    //! Kind of copy constructor
    CAppBindings(CAppBindings* pBindings)
        : vpl::base::CObject()
        , m_pAppMode(NULL)
        , m_pStorage(NULL)
        , m_pRenderer(NULL)
        , m_pParent(NULL)
        , m_pPlugin(NULL)
		, m_vplSignals(NULL)
    {
        Q_ASSERT(pBindings);
        if (NULL!=pBindings)
        {
            m_pAppMode = pBindings->getAppMode();
            m_pStorage = pBindings->getDataStorage();
            m_pParent = pBindings->getParentWindow();
            m_pPlugin = pBindings->getPluginInstance();
            m_pRenderer = pBindings->getRenderer();
			m_vplSignals = pBindings->getVPLSignals();
        }
    }

    //! Destructor.
    virtual ~CAppBindings()
    {
    }

    //! Initializes binding of the plugin to application.
    CAppBindings& setPluginInstance(PluginInterface *pPlugin)
    {
        m_pPlugin = pPlugin;
        return *this;
    }

    //! Returns pointer to the plugin instance.
    PluginInterface *getPluginInstance()
    {
        if( !m_pPlugin )
        {
            throw CNullAppBindings();
        }

        return m_pPlugin;
    }

    //! Initializes binding of the plugin to application.
    CAppBindings& setAppMode(scene::CAppMode *pAppMode)
    {
        m_pAppMode = pAppMode;
        return *this;
    }

    //! Returns pointer to the object managing mouse mode in the application.
    scene::CAppMode *getAppMode()
    {
        if( !m_pAppMode )
        {
            throw CNullAppBindings();
        }

        return m_pAppMode;
    }


    //! Initializes binding of the plugin to application.
    CAppBindings& setDataStorage(data::CDataStorage *pStorage)
    {
        m_pStorage = pStorage;
        return *this;
    }

    //! Returns pointer to the object managing data in the application.
    data::CDataStorage *getDataStorage()
    {
        if( !m_pStorage )
        {
            throw CNullAppBindings();
        }

        return m_pStorage;
    }

    //! Initializes binding of the plugin to application.
    CAppBindings& setRenderer(CVolumeRenderer *pRenderer)
    {
        if (m_pRenderer != nullptr)
        {
            onRendererDelete();
        }

        m_pRenderer = pRenderer;

        return *this;
    }

    //! Returns pointer to the volume renderer in the application.
    CVolumeRenderer *getRenderer()
    {
        return m_pRenderer;
    }

    //! Sets pointer to application's main window
    CAppBindings& setParentWindow(QWidget* pParent)
    {
        m_pParent = pParent;
        return *this;
    }

    //! Returns pointer to the application's main window
    QWidget *getParentWindow()
    {
        return m_pParent;
    }

    //! Sets pointer to application's main window
    CAppBindings& setVPLSignals(std::map<int,vpl::base::CObject*>* pSignals)
    {
		m_vplSignals = pSignals;
        return *this;
    }

    //! Returns pointer to the application's main window
    std::map<int,vpl::base::CObject*> *getVPLSignals()
    {
        return m_vplSignals;
    }

    //! Sets information on the current language to plugin
    //  localeDir - folder where to search translations
    //  lngFile - app language file (same name as locale name)
    //  fileName - plugin file name
    void    setLanguage(const QString &localeDir, const QString& lngFile, const QString& fileName)
    {
        m_wcsLocale = lngFile;

        if (!fileName.isEmpty())
        {
            if (lngFile.isNull() || lngFile.isEmpty())
            {
                // English - do nothing
            }
            else
            {                
                QFileInfo file(fileName.toLower());
#ifdef QT_DEBUG   // remove d suffix
                QString pluginFileName(file.baseName());
                if (pluginFileName.endsWith('d'))
                {
                    pluginFileName.chop(1);
                    pluginFileName=pluginFileName+"."+file.completeSuffix();
                    file.setFile(pluginFileName);
                }
#endif
#ifdef __APPLE__ // remove lib prefix
                QString pluginFileNameA(file.baseName());
                if (pluginFileNameA.startsWith("lib",Qt::CaseInsensitive))
                {
                    pluginFileNameA = pluginFileNameA.right(pluginFileNameA.length()-3)+"."+file.completeSuffix();
                    file.setFile(pluginFileNameA);
                }                
#endif
                if ((!localeDir.isEmpty() && m_translator.load( localeDir+"/"+file.baseName()+"-"+lngFile )) || // try to load from localeDir
                    m_translator.load( file.baseName()+"-"+lngFile )) // try to load from current working dir
                {
                    QApplication::installTranslator(&m_translator);
                }
                else
                {   // try to load from app dir
                    QString path=QCoreApplication::applicationDirPath();
                    if (m_translator.load( path+"/"+file.baseName()+"-"+lngFile ))
                    {
                        QApplication::installTranslator(&m_translator);
                    }
                }
            }
        }
    }

protected:
    virtual void onRendererDelete()
    { }

protected:
    //! Pointer to the class managing mouse mode in the application.
    scene::CAppMode *m_pAppMode;

    //! Pointer to the class managing data in the application.
    data::CDataStorage *m_pStorage;

    //! Pointer to renderer
    CVolumeRenderer *m_pRenderer;

    //! Pointer to parent window
    QWidget*        m_pParent;

    //! Info about preferred language
    QString         m_wcsLocale;

    //! Plugin translator object
    QTranslator     m_translator;

    //! Pointer to the plugin instance
    PluginInterface* m_pPlugin;

	//! Mapping of vpl signals
	std::map<int,vpl::base::CObject*>* m_vplSignals;
};


///////////////////////////////////////////////////////////////////////////////
//! 3Dim plugin interface

class PluginInterface : public CAppBindings
{
public:
    //! Constructor with some basic initialization
    PluginInterface() { setPluginInstance(this); }
public:
    //! Empty virtual destructor to avoid some compiler warnings
    virtual ~PluginInterface() {}

    //! Create Plugin's Menu, can be NULL
    //! Please note that the Menu, ToolBar and Panel are erased by the host application
    virtual QMenu*      getOrCreateMenu() = 0;

    //! Create Plugin's Toolbar, can be NULL
    virtual QToolBar*   getOrCreateToolBar() = 0;

    //! Create Plugin's Panel, can be NULL
    //! - note that windowTitle and objectName of the Panels has to be valid
    //!   and in case of objectName it should be also unique
    virtual QWidget*    getOrCreatePanel() = 0;

    //! Return pointer to action so the host application can trigger it
    virtual QAction*    getAction(const QString &actionName) = 0;

    //! Return localized plugin name
    virtual QString     pluginName() = 0;

    //! Return unique non localized plugin name eg "DataExpress"
    virtual QString     pluginID() = 0;

    //! Method called by the main application when the plugin is connected
    virtual void        connectPlugin() = 0;

    //! Method called by the main application when the plugin is disconnected
    virtual void        disconnectPlugin() = 0;

protected:
    //! Method called on renderer delete
    virtual void onRendererDelete()
    { }
};

Q_DECLARE_INTERFACE(PluginInterface, "com.3dim-laboratory.PluginInterface/1.0")

// License handling interface

#define PLUGIN_LICENSE_CHECK_OCCASIONALLY   1
#define PLUGIN_LICENSE_SHOW_DIALOG          2

class PluginLicenseInterface : public plug::CPluginInfo
{
public:
    //! Default constructor with plugin info
    PluginLicenseInterface(const std::string& ssHost,
                const std::string& ssName,
                const std::string& ssActName,
                int MajorNum,
                int MinorNum = 0
                )    :
        plug::CPluginInfo(ssHost,ssName,ssActName,MajorNum,MinorNum)
    {}

    //! Checks if the product license is valid.
    //! - Returns an error code, see the CActivationPolicy class.
    //! - Implement this method according to your needs.
    //! - Default version of the method does no license checking!
    //! - Handle of the plugin (i.e. pointer to the parent wxWindow object must be set)!
    virtual int validateLicense(int Flags = 0) = 0;

	//! Checks available exports in the product license
	virtual bool canExport(const QString& seriesID) = 0;

	//! Notifies that a series was exported (->inc used exports count)
	virtual void wasExported(const QString& seriesID) = 0;
};

Q_DECLARE_INTERFACE(PluginLicenseInterface, "com.3dim-laboratory.PluginLicenseInterface/1.1")

//
///////////////////////////////////////////////////////////////////////////////

//! Macro returns reference to the application mode.
//! - This macro may be used within any class derived from the CAppBindings class.
#define PLUGIN_APP_MODE     (*getAppMode())

//! Returns reference to the application data storage.
//! - This macro may be used within any class derived from the CAppBindings class.
#define PLUGIN_APP_STORAGE  (*getDataStorage())

//! Macro returns reference to volume renderer usable for rendering of custom volume data.
//! - This macro may be used within any class derived from the CAppBindings class.
#define PLUGIN_RENDERER  (*getRenderer())

//! Returns reference to the plugin.
//! - This macro may be used within any class derived from the CAppBindings class.
#define PLUGIN_INSTANCE      (*getPluginInstance())

#define PLUGIN_LICENSE_INSTANCE      (*(dynamic_cast<PluginLicenseInterface*>(getPluginInstance())))

//! Plugin VPL signals access - beware that only a subset of all signals is available!
#define PLUGIN_VPL_SIGNAL_AVAILABLE(x)	(NULL!=(dynamic_cast<decltype(&(VPL_SIGNAL(x)))>((*m_vplSignals)[VPL_SIGNAL(x).getId()])))
#define PLUGIN_VPL_SIGNAL(x)			(*(dynamic_cast<decltype(&(VPL_SIGNAL(x)))>((*m_vplSignals)[VPL_SIGNAL(x).getId()])))


#endif // _QT_PLUGININTERFACE_H
