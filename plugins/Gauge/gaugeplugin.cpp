///////////////////////////////////////////////////////////////////////////////
// $Id$
//

#include <QtGui>
#include <QMenu>
#include <QToolBar>

#include "gaugeplugin.h"
#include <data/CSceneManipulatorDummy.h>
#include "cgaugepanel.h"

GaugePlugin::GaugePlugin() : QObject(), PluginInterface()
{
    m_actionMeasureDensity = NULL;
    m_actionMeasureDistance = NULL;
    m_actionClearMeasurements = NULL;
    m_pMenu = NULL;
    m_pToolBar = NULL;
    m_pPanel = NULL;
}

void  GaugePlugin::createActions()
{
    if (!m_actionMeasureDensity)
    {
        m_actionMeasureDensity = new QAction(QIcon(":/icons/measure_density.png"),tr("Measure Density Value [Hu]"),NULL);
        m_actionMeasureDensity->setCheckable(true);
        m_actionMeasureDensity->setStatusTip(tr("To measure local density specify a point using the mouse cursor and click the left button."));
        connect(m_actionMeasureDensity, SIGNAL(triggered(bool)), this, SLOT(measureDensity(bool)) );
    }
    if (!m_actionMeasureDistance)
    {
        m_actionMeasureDistance = new QAction(QIcon(":/icons/measure_distance.png"),tr("Measure Distance [mm]"),NULL);
        m_actionMeasureDistance->setCheckable(true);
        m_actionMeasureDistance->setStatusTip(tr("Measure distance by clicking the left mouse button and dragging."));
        connect(m_actionMeasureDistance, SIGNAL(triggered(bool)), this, SLOT(measureDistance(bool)) );
    }
    if (!m_actionClearMeasurements)
    {
        m_actionClearMeasurements = new QAction(QIcon(":/icons/delete.png"),tr("Clear Measurements"),NULL);
        m_actionClearMeasurements->setStatusTip(tr("Clear all results of measurements visible in all scenes."));
        connect(m_actionClearMeasurements, SIGNAL(triggered()), this, SLOT(clearMeasurements()) );
    }
}

QMenu* GaugePlugin::getOrCreateMenu()
{
    if (m_pMenu) return m_pMenu;
    createActions();
    m_pMenu = new QMenu(tr("Gauge Plugin"));
    if (m_pMenu)
    {
        m_pMenu->addAction(m_actionMeasureDensity);
        m_pMenu->addAction(m_actionMeasureDistance);
        m_pMenu->addAction(m_actionClearMeasurements);
    }
    return m_pMenu;
}

QToolBar* GaugePlugin::getOrCreateToolBar()
{
    if (m_pToolBar) return m_pToolBar;
    createActions();
    QToolBar* pToolBar = new QToolBar(tr("Gauge Plugin ToolBar"));
    if (pToolBar)
    {
        pToolBar->setObjectName("Gauge Plugin ToolBar"); // do not translate
        pToolBar->addAction(m_actionMeasureDensity);
        pToolBar->addAction(m_actionMeasureDistance);
        pToolBar->hide();
        m_pToolBar = pToolBar;
    }
    return pToolBar;
}

QWidget*  GaugePlugin::getOrCreatePanel()
{
    createActions();
    // NOTE: seems like there's a bug in QT https://bugreports.qt.nokia.com/browse/QTBUG-13237
    //       uncheck acceptdrops in lineEdits to avoid it
    if (!m_pPanel)
        m_pPanel = new CGaugePanel(this);
    return m_pPanel;
}

QString  GaugePlugin::pluginName()
{
    return tr("Gauge Plugin");
}

QString  GaugePlugin::pluginID()
{
    return ("Gauge");
}

void GaugePlugin::connectPlugin()
{
    m_ConnectionModeChanged = getAppMode()->getModeChangedSignal().connect( this, &GaugePlugin::sigModeChanged );
    m_ConnectionDensityMeasure = PLUGIN_APP_MODE.getDensityMeasureSignal().connect(this, &GaugePlugin::sigDensityMeasured);
    m_ConnectionDistanceMeasure = PLUGIN_APP_MODE.getDistanceMeasureSignal().connect(this, &GaugePlugin::sigDistanceMeasured);
}

void GaugePlugin::disconnectPlugin()
{
    getAppMode()->getModeChangedSignal().disconnect( m_ConnectionModeChanged );
    PLUGIN_APP_MODE.getDensityMeasureSignal().disconnect(m_ConnectionDensityMeasure);
    PLUGIN_APP_MODE.getDistanceMeasureSignal().disconnect(m_ConnectionDistanceMeasure);
}

QAction* GaugePlugin::getAction(const QString &actionName)
{
    createActions();
    if (actionName=="MeasureDensity")
        return m_actionMeasureDensity;
    if (actionName=="MeasureDistance")
        return m_actionMeasureDistance;
    if (actionName=="ClearMeasurements")
        return m_actionClearMeasurements;
    return NULL;
}

void GaugePlugin::sigModeChanged( scene::CAppMode::tMode mode )
{
    Q_ASSERT(m_actionMeasureDensity && m_actionMeasureDistance);
    m_actionMeasureDensity->setChecked(scene::CAppMode::COMMAND_DENSITY_MEASURE==mode);
    m_actionMeasureDistance->setChecked(scene::CAppMode::COMMAND_DISTANCE_MEASURE==mode);
}

void GaugePlugin::measureDensity(bool on)
{
    if (on)
        getAppMode()->storeAndSet(scene::CAppMode::COMMAND_DENSITY_MEASURE);
    else
        getAppMode()->restore();
}

void GaugePlugin::measureDistance(bool on)
{
    if (on)
        getAppMode()->storeAndSet(scene::CAppMode::COMMAND_DISTANCE_MEASURE);
    else
        getAppMode()->restore();

}

void GaugePlugin::clearMeasurements()
{
    if (NULL==getDataStorage()) return;
    getDataStorage()->invalidate(getDataStorage()->getEntry(data::Storage::SceneManipulatorDummy::Id, data::Storage::NO_UPDATE).get());
}

void GaugePlugin::sigDensityMeasured(int nValue)
{
    if (m_pPanel)
        m_pPanel->setDensity(nValue);
}

void GaugePlugin::sigDistanceMeasured(double fValue)
{
    if (m_pPanel)
        m_pPanel->setDistance(fValue);
}

#if QT_VERSION < 0x050000    
    Q_EXPORT_PLUGIN2(pnp_gaugeplugin, GaugePlugin)
#endif