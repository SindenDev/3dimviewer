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

#include "mainwindow.h"
#include <CPluginManager.h>
#include <PluginInterface.h>

#include <QDir>
#include <QPluginLoader>
#include <QMainWindow>
#include <QSettings>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>
#include <qglobal.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <VPL/Base/Logging.h>
#include <data/CModelManager.h>
#include <actlog/ceventfilter.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <QStandardPaths>
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    #define DATALOCATION() QDesktopServices::storageLocation(QDesktopServices::DataLocation)
#else
    #define DATALOCATION() QStandardPaths::locate(QStandardPaths::DataLocation, QString(), QStandardPaths::LocateDirectory)
#endif


#define ONLINE_HELP_URL     "https://www.3dim-laboratory.cz/manuals/"


// Constructor
CPluginManager::CPluginManager()
{
    m_pMain = NULL;
    m_pluginsMenu = NULL;
	m_pluginsToolbar = NULL;
    m_pDockTo = NULL;
	initSignalTable();
}

CPluginManager::~CPluginManager()
{
    // make a note that we don't want any further online check today
    QSettings settings;
    settings.setValue("LastOnlineHelpCheck",QDate::currentDate().toString(Qt::ISODate));
	unloadPlugins(false);
}

void CPluginManager::unloadPlugins(bool bCallUnload)
{
    // Unload plugins and clear plugin array
    foreach (QPluginLoader* pluginLoader, m_plugins)
    {
		if (bCallUnload)
			pluginLoader->unload();
        delete pluginLoader;
    }   
    m_plugins.clear();
}

// loads plugin from a specified directory and all subdirectories
void CPluginManager::loadPluginsInDir(QString dirName)
{
    QDir pluginsDir = QDir(dirName);
    // load dynamic plugins from dir
#ifdef _WIN32
    foreach (QString fileName, pluginsDir.entryList(QStringList("*.dll"),QDir::Files)) {
#else
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
#endif
		QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        if (NULL!=loader)
        {
            QObject *plugin = loader->instance();
            if (plugin) {
                initPlugin(plugin,fileName);
                m_pluginFileNames += pluginsDir.absolutePath()+"/"+fileName;
                m_plugins.push_back(loader);
            }
            else
                delete loader;
        }
    }
    // load plugins from subdirectories
    foreach (QString subdirName, pluginsDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        loadPluginsInDir(pluginsDir.absolutePath()+"/"+subdirName);
    }
}

// calls disconnectPlugin method from the PluginInterface for every loaded plugin
void CPluginManager::disconnectPlugins()
{
	APP_MODE.disconnectAllDrawingHandlers();
	APP_MODE.disconnectAllSceneHitHandlers();
	// there's a problem with the deallocation of a model allocated in a dll, therefore we set it to null here
	if (!m_plugins.isEmpty() || !QPluginLoader::staticInstances().isEmpty())
	{
		for (int i = 0; i<MAX_MODELS; ++i)
		{
			data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(data::Storage::BonesModel::Id + i));
            geometry::CMesh* pMesh=spModel->getMesh();
			if (NULL != pMesh)
			{
				spModel->setMesh(NULL);
				spModel->clearAllProperties();
				APP_STORAGE.invalidate(spModel.getEntryPtr());
			}
        }
	}
    // call disconnectPlugin for every static plugin
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        if (iPlugin)
            iPlugin->disconnectPlugin();
    } 
    // beware of QTBUG-13237, disable acceptDrops in line edits if you experience crashes
    foreach(QDockWidget* pDW, m_panelsWidgets)
    {
        m_pMain->removeDockWidget(pDW);
    }
    foreach(QToolBar* pTB, m_toolbarWidgets)
    {
        m_pMain->removeToolBar(pTB);
    }
    if (NULL!=m_pluginsMenu)
        foreach(QMenu* pMenu, m_menuWidgets)
            m_pluginsMenu->removeAction(pMenu->menuAction());
	if (NULL!=m_pluginsToolbar)
	{
		m_pMain->removeToolBar(m_pluginsToolbar);
		delete m_pluginsToolbar;
		m_pluginsToolbar = NULL;
	}
    // call disconnectPlugin for every loaded plugin
    foreach (QPluginLoader* pluginLoader, m_plugins)
    {
        QObject *plugin = pluginLoader->instance();
        if (plugin)
        {
            PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
            if (iPlugin)                
            {
                // disconnect renderer
                iPlugin->setRenderer(NULL);                
                // disconnect other stuff
                iPlugin->disconnectPlugin();
                // delete plugin ui, because plugin destructor can come after examination destructor -> problem
                QMenu *pMenu = iPlugin->getOrCreateMenu();
                if (NULL!=pMenu)
                    delete pMenu;
                QToolBar* pToolBar = iPlugin->getOrCreateToolBar();
                if (NULL!=pToolBar)
                    delete pToolBar;
                QWidget* pPanel = iPlugin->getOrCreatePanel();
                if (NULL!=pPanel)
                    delete pPanel;                
            }
        }
        //pluginLoader->unload();
        //delete pluginLoader;
    }   
    //m_plugins.clear();
}

// Loads plugins
void CPluginManager::loadPlugins(QMainWindow* pMain,    // pointer to main window - needed to add menu, toolbars etc
                                 QDir* pLocaleDir,      // path to translations
                                 QDockWidget *dockTo)   // window to tabify plugin panels with
{
    m_pMain = pMain;
    m_localeDir = *pLocaleDir;
    m_pDockTo = dockTo;
    Q_ASSERT(m_pMain);
    if (NULL==m_pMain)
        return;

    // load static plugins
    foreach (QObject *plugin, QPluginLoader::staticInstances())
        initPlugin(plugin,NULL);

    // set plugin path
    m_pluginsDir = QDir(qApp->applicationDirPath());
    bool ok;
#ifdef _DEBUG
    ok=m_pluginsDir.cd("pluginsd");
#else
    ok=m_pluginsDir.cd("plugins");
#endif
    if (!ok)
    {
        m_pluginsDir.cdUp();
#ifdef _DEBUG
        ok=m_pluginsDir.cd("pluginsd");
#else
        ok=m_pluginsDir.cd("plugins");
#endif
    }
#ifdef __APPLE__
    if (!ok)
    {
        m_pluginsDir.cdUp();
#ifdef _DEBUG
        ok=m_pluginsDir.cd("pluginsd");
#else
        ok=m_pluginsDir.cd("plugins");
#endif
    }
#endif
    if (!ok)
        m_pluginsDir = QDir(qApp->applicationDirPath());	

    // load dynamic plugins
    loadPluginsInDir(m_pluginsDir.absolutePath());

    // we can't be sure that the dockwidget will exist later, set to null
    m_pDockTo = NULL;
}

void CPluginManager::initSignalTable()
{
	if (m_vplSignals.size()>0) return;
	m_vplSignals[VPL_SIGNAL(SigGetModelColor).getId()]=&(VPL_SIGNAL(SigGetModelColor));
	m_vplSignals[VPL_SIGNAL(SigSetModelColor).getId()]=&(VPL_SIGNAL(SigSetModelColor));
	m_vplSignals[VPL_SIGNAL(SigUndoSnapshot).getId()]=&(VPL_SIGNAL(SigUndoSnapshot));
	m_vplSignals[VPL_SIGNAL(SigSetColoring).getId()] = &(VPL_SIGNAL(SigSetColoring));
	m_vplSignals[VPL_SIGNAL(SigSetContoursVisibility).getId()] = &(VPL_SIGNAL(SigSetContoursVisibility));
	m_vplSignals[VPL_SIGNAL(SigGetContoursVisibility).getId()] = &(VPL_SIGNAL(SigGetContoursVisibility));
	m_vplSignals[VPL_SIGNAL(SigVolumeOfInterestChanged).getId()] = &(VPL_SIGNAL(SigVolumeOfInterestChanged));
	m_vplSignals[VPL_SIGNAL(SigEnableRegionColoring).getId()] = &(VPL_SIGNAL(SigEnableRegionColoring));
	m_vplSignals[VPL_SIGNAL(SigShowVolumeOfInterestDialog).getId()] = &(VPL_SIGNAL(SigShowVolumeOfInterestDialog));
	m_vplSignals[VPL_SIGNAL(SigGetModelCutVisibility).getId()] = &(VPL_SIGNAL(SigGetModelCutVisibility));
	m_vplSignals[VPL_SIGNAL(SigSetModelCutVisibility).getId()] = &(VPL_SIGNAL(SigSetModelCutVisibility));
	m_vplSignals[VPL_SIGNAL(SigGetSelectedModelId).getId()] = &(VPL_SIGNAL(SigGetSelectedModelId));
	m_vplSignals[VPL_SIGNAL(SigEstimateDensityWindow).getId()] = &(VPL_SIGNAL(SigEstimateDensityWindow));
	m_vplSignals[VPL_SIGNAL(SigSetDensityWindow).getId()] = &(VPL_SIGNAL(SigSetDensityWindow));
	m_vplSignals[VPL_SIGNAL(SigSaveModel).getId()] = &(VPL_SIGNAL(SigSaveModel));
    m_vplSignals[VPL_SIGNAL(SigNewTransformMatrixFromNote).getId()] = &(VPL_SIGNAL(SigNewTransformMatrixFromNote));
    m_vplSignals[VPL_SIGNAL(SigRemoveModel).getId()] = &(VPL_SIGNAL(SigRemoveModel));
}

void CPluginManager::initPlugin(QObject* plugin, const QString& fileName)
{
    if (!plugin) return;
    PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
    if (iPlugin)
    {
        VPL_LOG_INFO("Plugin: " << fileName.toStdString());
        // set app storage, app mode pointer and pointer to main window
        plugin->setProperty("FileName",fileName);
		plugin->setProperty("AppSignature",PLUG_APP_SIGNATURE);	// "OpenSource"
        iPlugin->setAppMode(&APP_MODE);
        iPlugin->setDataStorage(&APP_STORAGE);
        iPlugin->setRenderer(VPL_SIGNAL(SigGetRenderer).invoke2());
        iPlugin->setParentWindow(m_pMain);        
		iPlugin->setVPLSignals(&m_vplSignals);
        if (!fileName.isEmpty())
        {
            QSettings settings;
            QString lngFile=settings.value("Language","NA").toString();
            if ("NA"==lngFile) // no language selected - set according to system locale
                lngFile = ( QLocale::system().name() );
            if (!lngFile.isEmpty())
            {
                QFileInfo file(lngFile);
                plugin->setProperty("LocaleName",file.baseName());
            }
            iPlugin->setLanguage(m_localeDir.absolutePath(),lngFile, fileName); // load translation file
        }
		//
        PluginLicenseInterface *iPluginLic = qobject_cast<PluginLicenseInterface *>(plugin);
        if (NULL!=iPluginLic)
        {
            app::CProductInfo info=app::getProductInfo();
            QString product=QString("%1 %2.%3").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum());
            iPluginLic->setCurrentHost(product);
        }
        // create menu
        populateMenus(plugin);
        // create toolbar
        QToolBar* pToolbar = iPlugin->getOrCreateToolBar();
        if (pToolbar)
        {
            m_pMain->addToolBar(pToolbar);
            m_toolbarWidgets.append(pToolbar);
        }
        // create panel
        QWidget* pPanel = iPlugin->getOrCreatePanel();
        if (pPanel)
        {
            // create dock widget for the plugin panel
            QDockWidget* pDW = new QDockWidget(pPanel->windowTitle());
            pDW->setWidget(pPanel);
            pDW->setAllowedAreas(Qt::AllDockWidgetAreas);
            pDW->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
            pDW->setObjectName(pPanel->objectName());
			QString icon  = plugin->property("PanelIcon").toString();
			if (icon.isEmpty())
				pDW->setProperty("Icon",":/icons/page.png");
			else
				pDW->setProperty("Icon",icon);
            pDW->hide();
			connect(pDW, SIGNAL(visibilityChanged(bool)), m_pMain, SLOT(dockWidgetVisiblityChanged(bool)));
			connect(pDW,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),m_pMain,SLOT(dockLocationChanged(Qt::DockWidgetArea))); 
            // tabify or add to a dock area
            if (m_pDockTo)
                m_pMain->tabifyDockWidget(m_pDockTo,pDW);
            else
                m_pMain->addDockWidget(Qt::RightDockWidgetArea,pDW);
            m_panelsWidgets.append(pDW);
        }
        // call connectPlugin method to finish plugin initialization
        iPlugin->connectPlugin();
    }
}

// Finds plugin with a given ID
QObject* CPluginManager::findPluginByID(QString sPluginName)
{
    foreach (QString fileName, m_pluginFileNames)
    {
        QPluginLoader loader(m_pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin)
        {
            PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
            if (iPlugin)
                if (iPlugin->pluginID()==sPluginName)
                    return plugin;
        }
    }
    return NULL;
}

void CPluginManager::sslErrorHandler(QNetworkReply* reply, const QList<QSslError> & errlist)
{
    reply->ignoreSslErrors();
}

void CPluginManager::dataDownloaded(QNetworkReply* reply)
{
    QByteArray downloadedData;
    QNetworkReply::NetworkError err = reply->error();
    if(err == QNetworkReply::NoError)
    {
        int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
        if (reply->isReadable())
            downloadedData = reply->readAll();
    }
    QString saveFileName = reply->property("SaveTo").toString();
    reply->deleteLater();
    if (!downloadedData.isEmpty())
    {
        if (!saveFileName.isEmpty())
        {
            QFileInfo inf(saveFileName);
            QDir dir(inf.absolutePath());
            if (!dir.exists())
                dir.mkpath(".");

            QFile file(saveFileName);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(downloadedData);
                file.flush();
                file.close();
            }
        }
    }
}

// Adds entries
void CPluginManager::populateMenus(QObject *plugin)
{
    if (!plugin) return;
    // Create plugins menu in the apps main window
    if (!m_pluginsMenu)
	{
        m_pluginsMenu=m_pMain->menuBar()->addMenu(tr("&Plugins"));
		m_pluginsMenu->setObjectName("menuPlugins");
	}
	if (!m_pluginsToolbar)
	{
		m_pluginsToolbar = m_pMain->addToolBar(tr("Plugins Toolbar"));
		m_pluginsToolbar->setObjectName("toolbarPlugins");
		connect(m_pluginsToolbar,SIGNAL(actionTriggered(QAction*)),this,SLOT(pluginMenuAction(QAction*)));
	}
    if (NULL!=m_pluginsMenu)
    {
        // Basic plugin interface
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        // Licensed plugin interface
        PluginLicenseInterface *iPluginLic = qobject_cast<PluginLicenseInterface *>(plugin);
        if (iPlugin)
        {        
            // show help action
            QAction* pShowDoc = NULL;
            QString fileName = plugin->property("FileName").toString();
            if (!fileName.isEmpty())
            {
                QFileInfo file(fileName.toLower());
#ifdef QT_DEBUG
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
                const QString docPath = QDir::currentPath() + "/doc/";
                const QString localeName = plugin->property("LocaleName").toString();
                const QString wsDataPath = DATALOCATION()+"/Help/";
                QDir dir(docPath);
                QString pdfHelp;
                // name templates
                QString baseName(file.baseName()+".pdf"), localizedName;                                
                if (!localeName.isEmpty())
                    localizedName = file.baseName() + "-" + localeName + ".pdf";
                // look for installed help
                if (dir.exists(baseName))
                    pdfHelp = dir.absoluteFilePath(baseName);
                else
                {                    
                    if (!localizedName.isEmpty() && dir.exists(localizedName))
                        pdfHelp = dir.absoluteFilePath(localizedName);
                }
                // no installed help? see if there is any downloaded
                if (pdfHelp.isEmpty()) 
                {
                    // is there downloaded one?
                    QDir dirCache(wsDataPath);
                    if (dirCache.exists(baseName))
                        pdfHelp = dirCache.absoluteFilePath(baseName);
                    else
                        if (!localizedName.isEmpty() && dirCache.exists(localizedName))
                            pdfHelp = dirCache.absoluteFilePath(localizedName);
                    // check online (once per day only)
                    if (pdfHelp.isEmpty())
                    {
                        QSettings settings;
                        QDate dateLastCheck;
                        QVariant val = settings.value("LastOnlineHelpCheck");
                        if (QVariant::Date == val.type())        
                            dateLastCheck = val.toDate();
                        if (QVariant::String == val.type()) 
                            dateLastCheck = QDate::fromString(val.toString(),Qt::ISODate);
                        QString updateUrlBase = settings.value("OnlineHelpUrl",ONLINE_HELP_URL).toString();
                        if (dateLastCheck<QDate::currentDate())
                        {
                            // we will get the data but we won't add them to the plugin menu because it is async operation, 
                            // user will see the downloaded help next time when he starts the application
                            if (!updateUrlBase.endsWith('/'))
                                updateUrlBase+='/';
                            QString updateUrl = updateUrlBase + baseName;
                            m_networkManager.disconnect(this);
                            connect(&m_networkManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> &)));
                            connect(&m_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(dataDownloaded(QNetworkReply*)));
                            QNetworkRequest request(updateUrl);
                            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
                            request.setRawHeader( "User-Agent" , "Mozilla Firefox" ); // returns 406 when not specified
                            QNetworkReply* pReply = m_networkManager.get(request);                             
                            pReply->setProperty("SaveTo",wsDataPath + baseName);
                        }
                    }         
                }
                if (!pdfHelp.isEmpty())
                {
                    pShowDoc = new QAction(tr("Show Help"),this);
					pShowDoc->setObjectName("show_plugin_help");
                    pShowDoc->setProperty("OfflinePDF",pdfHelp);
                }
            }
            // create plugin menu
            QMenu* pPluginMenu=iPlugin->getOrCreateMenu();
			if (NULL!=pPluginMenu)
			{
				if (pPluginMenu->objectName().isEmpty())
					pPluginMenu->setObjectName(iPlugin->pluginID());
			}
            // check presence of toolbar or panel to add entries to show/hide them
            QToolBar* pToolBar=iPlugin->getOrCreateToolBar();
            QWidget* pPanel=iPlugin->getOrCreatePanel();
            // if there's no plugin menu but there is a toolbar or panel, create our own menu
            if (NULL==pPluginMenu && (pToolBar || pPanel || iPluginLic || pShowDoc))
			{
                pPluginMenu = new QMenu(iPlugin->pluginName());
				pPluginMenu->setObjectName(iPlugin->pluginID());
			}
            if (NULL!=pPluginMenu)
            {
                if (pToolBar || pPanel || iPluginLic || pShowDoc)
                {
                    pPluginMenu->addSeparator();
                    if (pToolBar)
                    {
                        // create new action
                        QAction* pActShowToolbar=new QAction(tr("Show/Hide Plugin Toolbar"),NULL);
						pActShowToolbar->setObjectName("show_hide_plugin_toolbar");
                        // set pointer to object as custom data - used by pluginMenuAction
                        pActShowToolbar->setData(QVariant::fromValue((void*)pToolBar));
                        // add action
                        pPluginMenu->addAction(pActShowToolbar);
                    }
                    if (pPanel)
                    {
                        QAction* pActShowPanel=new QAction(tr("Show/Hide Plugin Panel"),NULL);
						pActShowPanel->setObjectName("show_hide_plugin_panel");
                        pActShowPanel->setData(QVariant::fromValue((void*)pPanel));
						QString icon  = plugin->property("Icon").toString();
						if (!icon.isEmpty())
							pActShowPanel->setIcon(QIcon(icon));
                        pPluginMenu->addAction(pActShowPanel);
                    }
                    if (iPluginLic)
                    {
                        if (pToolBar || pPanel)
                            pPluginMenu->addSeparator();
                        QAction* pActRegistration=new QAction(tr("Plugin Registration..."),NULL);
						pActRegistration->setObjectName("plugin_registration");
                        pActRegistration->setData(QVariant::fromValue((void*)plugin));
                        pPluginMenu->addAction(pActRegistration);
                    }
                    if (pShowDoc)
                    {
                        if (pToolBar || pPanel)
                            pPluginMenu->addSeparator();
                        //QAction* pActRegistration=new QAction(tr("Plugin Registration..."),NULL);
                        //pActRegistration->setData(QVariant::fromValue((void*)plugin));
                        pPluginMenu->addAction(pShowDoc);
                    }
                    // all actions are handled in one menu handler
                    connect(pPluginMenu,SIGNAL(triggered(QAction*)),this,SLOT(pluginMenuAction(QAction*)));

					// connect menu actions to event filter
                    IHasEventFilter *mw = dynamic_cast<IHasEventFilter *>(MainWindow::getInstance());
                    if (mw != NULL)
                    {
                        connect(pPluginMenu, SIGNAL(triggered(QAction*)), &mw->getEventFilter(), SLOT(catchPluginMenuAction(QAction*)));
                    }
                }
                connect(pPluginMenu,SIGNAL(aboutToShow()),this,SLOT(pluginMenuAboutToShow()));
                m_pluginsMenu->addMenu(pPluginMenu);
            }
            if (NULL!=pPluginMenu)
            {
                m_menuWidgets.push_back(pPluginMenu);
            }
			if (NULL!=m_pluginsToolbar && NULL!=pPanel)
			{
                QAction* pActShowPanel=new QAction(iPlugin->pluginName(),NULL);
				pActShowPanel->setObjectName("show_plugin_panel");
                pActShowPanel->setData(QVariant::fromValue((void*)pPanel));
				QString icon  = plugin->property("Icon").toString();
				if (!icon.isEmpty())
					pActShowPanel->setIcon(QIcon(icon));
				else
					pActShowPanel->setIcon(QIcon(":/icons/3dim.ico"));
                m_pluginsToolbar->addAction(pActShowPanel);	

                // connect plugin menu button to event filter
                IHasEventFilter *mw = dynamic_cast<IHasEventFilter *>(MainWindow::getInstance());
                if (mw != NULL)
                {
                    connect(pActShowPanel, SIGNAL(triggered()), &mw->getEventFilter(), SLOT(catchMenuAction()));
                }
            }
        }
    }
}

// returns plugin panel's parent dockwidget
QDockWidget* CPluginManager::getParentDockWidget(QWidget* view)
{
    if (NULL==view) return NULL;
    QWidget* pParent=view->parentWidget();
    if (NULL==pParent) return NULL;
    QDockWidget* pDockW=qobject_cast<QDockWidget*>(pParent);
    return pDockW;
}

// resets all plugin panels to a "default" layout
void CPluginManager::tabifyAndHidePanels(QDockWidget* pDock1)
{
    Q_ASSERT(pDock1);

    // run through static plugin panels
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        if (iPlugin)
        {
            QWidget* pPanel=iPlugin->getOrCreatePanel();
            if (pPanel)
            {
                QDockWidget* pDock=getParentDockWidget(pPanel);
                if (pDock)
                {
                    m_pMain->tabifyDockWidget(pDock1,pDock);
                    pDock->hide();
                }
            }
        }
    }
    // run through dynamic plugin panels
    foreach (QString fileName, m_pluginFileNames)
    {
        QPluginLoader loader(m_pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin)
        {
            PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
            if (iPlugin)
            {
                QWidget* pPanel=iPlugin->getOrCreatePanel();
                if (pPanel)
                {
                    QDockWidget* pDock=getParentDockWidget(pPanel);
                    if (pDock)
                    {
                        m_pMain->tabifyDockWidget(pDock1,pDock);
                        pDock->hide();
                    }
                }
            }
        }
    }
}

// this method is called when some "general" action (show/hide plugin panel, toolbar, register)
// in a plugin menu is triggered
void CPluginManager::pluginMenuAction(QAction* action)
{
    if (!action) return;
    // get custom data of that action
    QWidget* pObj=(QWidget*)action->data().value<void*>();
    if (NULL!=pObj)
    {
        // if custom data contain a toolbar pointer, toggle its visibility
        QToolBar* pToolBar=qobject_cast<QToolBar*>(pObj);
        if (pToolBar)
        {
            if (pToolBar->isVisible())
                pToolBar->hide();
            else
                pToolBar->show();
        }
        else
        {
            // for others assume that it's the panel, toggle its visibility
            QWidget* pPanel=qobject_cast<QWidget*>(pObj);
            if (pPanel)
            {
				bool bForceShow = (0==action->objectName().compare("show_plugin_panel",Qt::CaseInsensitive));
				QDockWidget* pParentDock = qobject_cast<QDockWidget*>(pPanel->parentWidget());
				if (pParentDock)
				{
					if (pParentDock->isVisible() && !bForceShow)
					{
						pParentDock->hide();
					}
					else
					{
						pParentDock->show();
						pParentDock->raise();
					}
				}
				else
				{
					if (pPanel->isVisible() && !bForceShow)
						pPanel->hide();
					else
						pPanel->show();
				}
            }
            else
            {
                PluginLicenseInterface *iPluginLic = qobject_cast<PluginLicenseInterface *>(pObj);
                if (iPluginLic)
                {
                    iPluginLic->validateLicense(PLUGIN_LICENSE_SHOW_DIALOG);
                }
            }
        }
    }
    else
    {
        QString pdfPath = action->property("OfflinePDF").toString();
        if (!pdfPath.isEmpty())
        {
            QFile file(pdfPath);
            if (file.exists())
            {
                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(file.fileName())))
                    QMessageBox::critical(NULL,QCoreApplication::applicationName(),tr("A PDF viewer is required to view help!"));            
            }    
            else
                QMessageBox::critical(NULL,QCoreApplication::applicationName(),tr("Help file is missing!"));
        }
        QString pdfUrl = action->property("OnlinePDF").toString();
        if (!pdfUrl.isEmpty())
        {
            if (!QDesktopServices::openUrl(QUrl(pdfUrl)))
                QMessageBox::critical(NULL,QCoreApplication::applicationName(),tr("Couldn't open online help! Please verify your internet connection."));            
        }
    }
}

// Using this method a desired action from a plugin can be triggered
void CPluginManager::triggerPluginAction(const QString& pluginName, const QString& actionName)
{
    QObject* pPlugin=findPluginByID(pluginName);
    if (NULL!=pPlugin)
    {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(pPlugin);
        if (iPlugin)
        {
            QAction* pAct=iPlugin->getAction(actionName);
            if (pAct)
                pAct->trigger();
        }
    }
}

// set visibility checkboxes for toolbar and panel in plugin menu
void CPluginManager::pluginMenuAboutToShow()
{
    // sender should be always a menu
    QMenu* pMenu=qobject_cast<QMenu*>(sender());
    if (NULL!=pMenu)
    {
        // for every action in that menu
        QList<QAction *> actions = pMenu->actions();
        for(QList<QAction*>::const_iterator it = actions.begin(); it != actions.end(); ++it)
        {
            QAction* pAct=(*it);
            // get custom data of that action
            QWidget* pObj=(QWidget*)pAct->data().value<void*>();
            if (pObj) // if any than it should be a pointer to a widget
            {
                // a toolbar
                QToolBar* pToolBar=qobject_cast<QToolBar*>(pObj);
                if (pToolBar)
                {
                    pAct->setCheckable(true);
                    pAct->setChecked(pToolBar->isVisible());
                }
                else
                {
                    // or a dock widget containing the plugin's panel
                    QDockWidget* pParentDock = qobject_cast<QDockWidget*>(pObj->parentWidget());
                    if (pParentDock)
                    {
                        pAct->setCheckable(true);
                        pAct->setChecked(pParentDock->isVisible());
                    }
                }
            }
        }
    }
}
