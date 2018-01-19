///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#include <QtGui>
#include <QMenu>
#include <QToolBar>
#include "notesplugin.h"
#include "notespluginpanel.h"


CNotesPlugin::CNotesPlugin() : QObject(), PluginInterface()
{

    m_pMenu = nullptr;
    m_pToolBar = nullptr;
    m_pPanel = nullptr;
    setProperty("Icon", ":/icons/icons/notesplugin.png");
    setProperty("PanelIcon", ":/icons/icons/notesplugin_dock.png");
}

CNotesPlugin::~CNotesPlugin()
{

}


//vyvori polozku v drop down menu pluginu
QMenu* CNotesPlugin::getOrCreateMenu() {

    if (m_pMenu) 
        return m_pMenu;

    m_pMenu = new QMenu(tr("Notes Plugin"));
   
    return m_pMenu;
}


//nikde mi nechybi
QToolBar* CNotesPlugin::getOrCreateToolBar(){
    
    return nullptr;
}


QWidget*  CNotesPlugin::getOrCreatePanel()
{
    // NOTE: seems like there's a bug in QT https://bugreports.qt.nokia.com/browse/QTBUG-13237
    //       uncheck acceptdrops in lineEdits to avoid it
    if (!m_pPanel)
        m_pPanel = new CNotesPluginPanel(this);
    return m_pPanel;
}

QString  CNotesPlugin::pluginName()
{
    return tr("Notes Plugin");
}

QString  CNotesPlugin::pluginID()
{
    return ("NotesPlugin");
}

void CNotesPlugin::connectPlugin() { }


void CNotesPlugin::disconnectPlugin() { }

//! No effect. Always returns nullptr
QAction* CNotesPlugin::getAction(const QString &actionName)
{
    return nullptr;
}

#if QT_VERSION < 0x050000  
    Q_EXPORT_PLUGIN2(pnp_notesplugin, CNotesPlugin)
#endif