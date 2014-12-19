///////////////////////////////////////////////////////////////////////////////
// $Id: cdemoplugin.cpp 1530 2012-03-02 07:54:39Z tryhuk $
//

#include <QtGui>
#include <QMenu>
#include <QToolBar>
#include "cdemoplugin.h"
#include "cdemopluginpanel.h"

CDemoPlugin::CDemoPlugin() : QObject(), PluginInterface()
{
    m_actionPickValue = NULL;
    m_actionStroke = NULL;
    m_actionPolygon = NULL;
    m_actionShowRegions = NULL;
    m_pMenu = NULL;
    m_pToolBar = NULL;
    m_pPanel = NULL;
    m_colorComboBox = NULL;
}

CDemoPlugin::~CDemoPlugin()
{
    if (NULL!=m_colorComboBox)
        delete m_colorComboBox;
    m_colorComboBox = NULL;
}

void  CDemoPlugin::createActions()
{
    if (!m_actionPickValue)
    {
        m_actionPickValue = new QAction(QIcon(":/icons/icons/1.png"),tr("Pick Value"),NULL);
        m_actionPickValue->setStatusTip(tr("Click the button and then select any point the volume data."));
        QObject::connect(m_actionPickValue, SIGNAL(triggered()), this, SLOT(onActionPickValue()) );
    }
    if (!m_actionStroke)
    {
        m_actionStroke = new QAction(QIcon(":/icons/icons/2.png"),tr("Stroke Drawing Mode"),NULL);
        m_actionStroke->setStatusTip(tr("Click the button, then press Shift and start drawing using mouse."));
        m_actionStroke->setCheckable(true);
        QObject::connect(m_actionStroke, SIGNAL(triggered(bool)), this, SLOT(onActionStroke(bool)) );
    }
    if (!m_actionPolygon)
    {
        m_actionPolygon = new QAction(QIcon(":/icons/icons/3.png"),tr("Polygon Drawing Mode"),NULL);
        m_actionPolygon->setStatusTip(tr("Click the button, then press Shift and start drawing using mouse."));
        m_actionPolygon->setCheckable(true);
        QObject::connect(m_actionPolygon, SIGNAL(triggered(bool)), this, SLOT(onActionPolygon(bool)) );
    }
    if (!m_actionShowRegions)
    {
        m_actionShowRegions = new QAction(QIcon(":/icons/icons/4.png"),tr("Show Regions"),NULL);
        m_actionShowRegions->setStatusTip(tr("Show Regions"));
        m_actionShowRegions->setCheckable(true);
        QObject::connect(m_actionShowRegions, SIGNAL(triggered(bool)), this, SLOT(onActionShowRegions(bool)) );
    }
}

QMenu* CDemoPlugin::getOrCreateMenu()
{
    if (m_pMenu) return m_pMenu;
    createActions();
    m_pMenu = new QMenu(tr("Demo Plugin"));
    if (m_pMenu)
    {
        m_pMenu->addAction(m_actionPickValue);
        m_pMenu->addSeparator();
        m_pMenu->addAction(m_actionPolygon);
        m_pMenu->addAction(m_actionStroke);
        m_pMenu->addSeparator();
        m_pMenu->addAction(m_actionShowRegions);
    }
    return m_pMenu;
}

QToolBar* CDemoPlugin::getOrCreateToolBar()
{
    if (m_pToolBar) return m_pToolBar;
    createActions();
    QToolBar* pToolBar = new QToolBar(tr("Demo Plugin ToolBar"));
    if (pToolBar)
    {
        pToolBar->setObjectName("Demo Plugin ToolBar"); // do not translate - id for settings saving
        pToolBar->addAction(m_actionPickValue);
        pToolBar->addAction(m_actionStroke);
        pToolBar->addAction(m_actionPolygon);
        pToolBar->addAction(m_actionShowRegions);
        {
            QComboBox* pRegionCombo = new QComboBox;
            if (NULL==m_colorComboBox)
                m_colorComboBox = new CColorComboBox;
            m_colorComboBox->setCombo(pRegionCombo);
            pToolBar->addWidget(pRegionCombo);
            QObject::connect(pRegionCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onActionRegionChanged(int)));
        }
        pToolBar->hide();
        m_pToolBar = pToolBar;
    }
    return pToolBar;
}

QWidget*  CDemoPlugin::getOrCreatePanel()
{
    createActions();
    // NOTE: seems like there's a bug in QT https://bugreports.qt.nokia.com/browse/QTBUG-13237
    //       uncheck acceptdrops in lineEdits to avoid it
    if (!m_pPanel)
        m_pPanel = new CDemoPluginPanel(this);
    return m_pPanel;
}

QString  CDemoPlugin::pluginName()
{
    return tr("Demo Plugin");
}

QString  CDemoPlugin::pluginID()
{
    return ("DemoPlugin");
}

void CDemoPlugin::connectPlugin()
{
    // Initialize color box first time
    if (NULL==m_colorComboBox)
        m_colorComboBox = new CColorComboBox;
    data::CObjectPtr<data::CRegionColoring> spColoring( PLUGIN_APP_STORAGE.getEntry( data::Storage::RegionColoring::Id ) );
    if( spColoring.get() )
    {
        m_colorComboBox->objectChanged( spColoring.get() );
    }
    // Connect signal invoked on region change to the combo box
    PLUGIN_APP_STORAGE.connect(data::Storage::RegionColoring::Id, m_colorComboBox);

    // Register the plugin as region data observer to update m_actionShowRegions
    PLUGIN_APP_STORAGE.connect(data::Storage::RegionData::Id, this);

    // Connect to mode change signal for state update of m_actionStroke and m_actionPolygon
    m_ConnectionModeChanged = getAppMode()->getModeChangedSignal().connect( this, &CDemoPlugin::sigModeChanged );
}

void CDemoPlugin::disconnectPlugin()
{
    getAppMode()->getModeChangedSignal().disconnect( m_ConnectionModeChanged );
    PLUGIN_APP_STORAGE.disconnect(data::Storage::RegionData::Id, this);
    if (m_colorComboBox)
    {
        PLUGIN_APP_STORAGE.disconnect(data::Storage::RegionColoring::Id, m_colorComboBox);    
        delete m_colorComboBox;
        m_colorComboBox = NULL;
    }
}

QAction* CDemoPlugin::getAction(const QString &actionName)
{
    createActions();
    if (actionName=="PickValue")
        return m_actionPickValue;
    return NULL;
}

void CDemoPlugin::sigModeChanged( scene::CAppMode::tMode mode )
{
    data::CObjectPtr< data::CDrawingOptions > spOptions( PLUGIN_APP_STORAGE.getEntry(data::Storage::DrawingOptions::Id) );
    data::CDrawingOptions::EDrawingMode drMode=spOptions->getDrawingMode();
    spOptions.release();
    if (m_actionStroke)
    {
        m_actionStroke->blockSignals(true);
        m_actionStroke->setChecked(scene::CAppMode::COMMAND_DRAW_GEOMETRY==mode && data::CDrawingOptions::DRAW_STROKE==drMode);
        m_actionStroke->blockSignals(false);
    }
    if (m_actionPolygon)
    {
        m_actionPolygon->blockSignals(true);
        m_actionPolygon->setChecked(scene::CAppMode::COMMAND_DRAW_GEOMETRY==mode && data::CDrawingOptions::DRAW_LASO==drMode);
        m_actionPolygon->blockSignals(false);
    }
}

void CDemoPlugin::objectChanged(data::CRegionData *pData)
{
    m_actionShowRegions->blockSignals(true);
    m_actionShowRegions->setChecked(pData->isColoringEnabled());
    m_actionShowRegions->blockSignals(false);
    CDemoPluginPanel * pPanel = dynamic_cast<CDemoPluginPanel*>( getOrCreatePanel() );
    if (NULL!=pPanel)
        pPanel->updateShowRegions(pData->isColoringEnabled());
}

void CDemoPlugin::updateActions(QAction *letBe)
{
    // Update GUI
    m_actionStroke->blockSignals(true);
    m_actionPolygon->blockSignals(true);
    if (letBe!=m_actionStroke)
        m_actionStroke->setChecked(false);
    if (letBe!=m_actionPolygon)
        m_actionPolygon->setChecked(false);
    m_actionStroke->blockSignals(false);
    m_actionPolygon->blockSignals(false);
}

void CDemoPlugin::onActionPickValue()
{
    CDemoPluginPanel * pPanel = dynamic_cast<CDemoPluginPanel*>( getOrCreatePanel() );
    if( pPanel )
        pPanel->on_pushButtonPickValue_clicked();
}

void CDemoPlugin::onActionStroke(bool checked)
{
    updateActions(m_actionStroke);
    CDemoPluginPanel * pPanel = dynamic_cast<CDemoPluginPanel*>( getOrCreatePanel() );
    if( pPanel )
        pPanel->on_pushButtonStroke_toggled(checked);
}

void CDemoPlugin::onActionPolygon(bool checked)
{
    updateActions(m_actionPolygon);
    CDemoPluginPanel * pPanel = dynamic_cast<CDemoPluginPanel*>( getOrCreatePanel() );
    if( pPanel )
        pPanel->on_pushButtonPolygon_toggled(checked);
}

void CDemoPlugin::onActionShowRegions(bool checked)
{
    CDemoPluginPanel * pPanel = dynamic_cast<CDemoPluginPanel*>( getOrCreatePanel() );
    if( pPanel )
        pPanel->on_checkBoxShowRegions_toggled(checked);
}

void CDemoPlugin::onActionRegionChanged(int index)
{
    if (NULL==m_colorComboBox)
        m_colorComboBox = new CColorComboBox;
    m_colorComboBox->usualIndexChangedHandler(&PLUGIN_APP_STORAGE,index);
}

#if QT_VERSION < 0x050000  
    Q_EXPORT_PLUGIN2(pnp_demoplugin, CDemoPlugin)
#endif