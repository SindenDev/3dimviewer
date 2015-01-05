///////////////////////////////////////////////////////////////////////////////
// $Id$
//

#ifndef GAUGEPLUGIN_H
#define GAUGEPLUGIN_H

#include <QRect>
#include <QObject>
#include <QStringList>
#include <QPainterPath>
#include <QImage>
#include <QAction>

#include <qtplugin/PluginInterface.h>

class CGaugePanel;

class GaugePlugin : public QObject,
                          public PluginInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000    
    Q_PLUGIN_METADATA(IID "com.3dim-laboratory.Qt.GaugePlugin")
#endif
    Q_INTERFACES(PluginInterface)
public:
    GaugePlugin();
protected:
    QMenu*      m_pMenu;
    QToolBar*   m_pToolBar;
    CGaugePanel* m_pPanel;

    QAction*    m_actionMeasureDensity;
    QAction*    m_actionMeasureDistance;
    QAction*    m_actionClearMeasurements;
    void        createActions();
private slots:
    void        measureDensity(bool);
    void        measureDistance(bool);
    void        clearMeasurements();
protected:
    //! Signal connection for mouse mode change monitoring
    vpl::mod::tSignalConnection m_ConnectionModeChanged,
                                m_ConnectionDensityMeasure,
                                m_ConnectionDistanceMeasure;
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
    void        sigDensityMeasured(int nValue);
    void        sigDistanceMeasured(double fValue);
    void        sigModeChanged( scene::CAppMode::tMode mode );
};

#endif
