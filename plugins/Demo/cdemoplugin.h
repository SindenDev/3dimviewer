///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2015 3Dim Laboratory s.r.o.
//

#ifndef CDEMOPLUGIN_H
#define CDEMOPLUGIN_H

#include <QRect>
#include <QObject>
#include <QStringList>
#include <QPainterPath>
#include <QImage>
#include <QAction>

#include <qtplugin/PluginInterface.h>
#include <controls/ccolorcombobox.h>
#include <data/CRegionData.h>

class CDemoPluginPanel;

//! Demo Plugin has to inherit QObject and PluginInterface
class CDemoPlugin : public QObject,
                    public PluginInterface,
                    public data::CObjectObserver< data::CRegionData >
{
    Q_OBJECT
#if QT_VERSION >= 0x050000    
    Q_PLUGIN_METADATA(IID "com.3dim-laboratory.Qt.DemoPlugin")
#endif
    Q_INTERFACES(PluginInterface)
public:
    //! Constructor
    CDemoPlugin();
    //! Destructor
    ~CDemoPlugin();
protected:
    //! Plugin menu
    QMenu*      m_pMenu;
    //! Plugin toolbar
    QToolBar*   m_pToolBar;
    //! Plugin panel
    CDemoPluginPanel* m_pPanel;
    //! Plugin action
    QAction*    m_actionPickValue;
    QAction*    m_actionStroke;
    QAction*    m_actionPolygon;
    QAction*    m_actionShowRegions;
    //! ComboBox with regions
    CColorComboBox* m_colorComboBox;
    //! Helper method for action creation
    void        createActions();
    //! Uncheck actions
    void        updateActions(QAction *letBe);
private slots:
    //! action handler
    void        onActionPickValue();
    void        onActionStroke(bool);
    void        onActionPolygon(bool);
    void        onActionShowRegions(bool);
    void        onActionRegionChanged(int);
protected:
    //! Signal connection for mouse mode change monitoring
    vpl::mod::tSignalConnection m_ConnectionModeChanged;
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
protected:
    //! Signal handlers
    void        sigModeChanged( scene::CAppMode::tMode mode );
    //! Region data observer method
    void        objectChanged(data::CRegionData *pData);
};

#endif
