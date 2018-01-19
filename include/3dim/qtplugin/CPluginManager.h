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

#ifndef _QT_PLUGINMANAGER_H
#define _QT_PLUGINMANAGER_H

class QMenu;
class QDockWidget;
class QMainWindow;
class QAction;
class QToolBar;
class QPluginLoader;

#include <QStringList>
#include <QDir>
#include <QNetworkAccessManager>
#include <map>
#include <VPL/Base/Object.h>

class CPluginManager : public QObject
{
    Q_OBJECT
protected:
    //! Pointer to application main window
    QMainWindow*            m_pMain;
    //! Translations base directory
    QDir                    m_localeDir;
    //! DockWidget that the plugins shall tabify to during loadplugins
    QDockWidget*            m_pDockTo;

    //! Plugins base directory
    QDir                    m_pluginsDir;
    //! List of plugins
    QStringList             m_pluginFileNames;
    //! Plugins menu
    QMenu*                  m_pluginsMenu;
	//! Plugins toolbar
	QToolBar*               m_pluginsToolbar;
    //! List of dock widgets associated with panels from plugins - for debugging purpose
    QList<QDockWidget *>    m_panelsWidgets;
    QList<QMenu*>           m_menuWidgets;
    QList<QToolBar*>        m_toolbarWidgets;

    //! network manager for online data access
    QNetworkAccessManager       m_networkManager;

    // Array to store all plugins
    QList<QPluginLoader*>   m_plugins;

	//! Signal mapping table
	std::map<int,vpl::base::CObject*>	m_vplSignals;

	//! Initialize table of vpl signals
	void			initSignalTable();
    //! connects plugin to the main application
    void            initPlugin(QObject* plugin, const QString &fileName);
    //! populates menus with plugin stuff
    void            populateMenus(QObject *plugin);
    //! loads plugins from specified directory and its subdirectories
    void            loadPluginsInDir(QString dirName);
    //! returns panel's parent dockwidget
    QDockWidget*    getParentDockWidget(QWidget* view);
private slots:
    //! Plugin menu handler - shows/hides toolbars and panels
    void            pluginMenuAction(QAction*);
    //! Update plugin menu when being shown
    void            pluginMenuAboutToShow();
    //! Ssl error handler
    void            sslErrorHandler(QNetworkReply* reply, const QList<QSslError> & errlist);
    //! Network request handler
    void            dataDownloaded(QNetworkReply*);
public:
    //! Constructor
    CPluginManager();

    //! Destructor
    ~CPluginManager();

    //! Load plugins
    void    loadPlugins(QMainWindow* pMain, QDir* pLocaleDir, QDockWidget* dockTo);

    //! Call disconnect for all plugins (should be called during app shutdown)
    void    disconnectPlugins();

	void	unloadPlugins(bool bCallUnload);

    //! Returns pointer to a plugin with a given name
    QObject* findPluginByID(QString sPluginName);

    //! Returns plugin base path
    QString getPluginsPath() { return m_pluginsDir.path(); }

    //! Returns pointer to loaded plugin list
    const QStringList* getPluginsList() { return &m_pluginFileNames; }

    //! Tabifies all panels to a dock widget of your choice and hides them
    void    tabifyAndHidePanels(QDockWidget* pDockTo);

    //! Triggers action from a specified plugin, if available
    void    triggerPluginAction(const QString& pluginName, const QString& actionName);

	std::map<int, vpl::base::CObject*>& getVplSignalsTable()
	{
		return m_vplSignals;
	}
};

#endif // _QT_PLUGINMANAGER_H
