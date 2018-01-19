///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#ifndef NOTESPLUGIN_H
#define NOTESPLUGIN_H


#include <QObject>

#include <qtplugin/PluginInterface.h>
#include <data/CRegionData.h>


class CNotesPluginPanel;

//! Plugin has to inherit QObject and PluginInterface
class CNotesPlugin : public QObject,
                    public PluginInterface
{
    Q_OBJECT
//#if QT_VERSION >= 0x050000    
        Q_PLUGIN_METADATA(IID "com.3dim-laboratory.Qt.NotesPlugin")
//#endif
    Q_INTERFACES(PluginInterface)

public:
    //! Constructor
    CNotesPlugin();
    //! Destructor
    ~CNotesPlugin();
protected:
    //these are needed from PluginInterface 

    //! Plugin menu
    QMenu*      m_pMenu;
    //! Plugin toolbar
    QToolBar*   m_pToolBar;
    //! Plugin panel
    CNotesPluginPanel* m_pPanel;



public:
    //! Methods from PluginInterface
    QMenu*      getOrCreateMenu();
    QToolBar*   getOrCreateToolBar();
    QWidget*    getOrCreatePanel();
    QAction*    getAction(const QString &actionName);
    QString     pluginName();
    QString     pluginID();
    void        connectPlugin();
    void        disconnectPlugin();
};

#endif
