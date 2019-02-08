///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#include "cdemopluginpanel.h"
#include "ui_cdemopluginpanel.h"

#include <coremedi/app/Signals.h>
#include <data/CRegionData.h>

#include <drawing/CISEventHandler.h>
#include <drawing/CLineOptimizer.h>

#include <QDebug>
#include <QSettings>

CDemoPluginPanel::CDemoPluginPanel(CAppBindings *pBindings, QWidget *parent) :
    QWidget(parent), CAppBindings(pBindings),
    ui(new Ui::CDemoPluginPanel)
{
    ui->setupUi(this);
    Q_ASSERT(pBindings);
    setAppMode(pBindings->getAppMode());
    setDataStorage(pBindings->getDataStorage());

    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_2);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_3);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_4);

    QSettings settings;
    settings.beginGroup("DemoPanel");

    if (settings.value("PackGroup1", false).toBool())
    {
        ui->groupBox->setChecked(false);
        packGroupBox(ui->groupBox, false);
    }
    if (settings.value("PackGroup2", false).toBool())
    {
        ui->groupBox_2->setChecked(false);
        packGroupBox(ui->groupBox_2, false);
    }
    if (settings.value("PackGroup3", false).toBool())
    {
        ui->groupBox_3->setChecked(false);
        packGroupBox(ui->groupBox_3, false);
    }
    if (settings.value("PackGroup4", false).toBool())
    {
        ui->groupBox_4->setChecked(false);
        packGroupBox(ui->groupBox_4, false);
    }

    settings.endGroup();

    m_colorComboBox.setCombo(ui->comboBoxRegion);

    // Initialize color box first time
    data::CObjectPtr<data::CRegionColoring> spColoring( PLUGIN_APP_STORAGE.getEntry( data::Storage::RegionColoring::Id ) );
    if( spColoring.get() )
    {
        m_colorComboBox.updateFromColoring( spColoring.get() );
    }
    // Connect signal invoked on region change to the combo box
    PLUGIN_APP_STORAGE.connect(data::Storage::RegionColoring::Id, &m_colorComboBox);

    // Retreive the active region
    data::CObjectPtr<data::CRegionData> spData( PLUGIN_APP_STORAGE.getEntry(data::Storage::RegionData::Id) );
    ui->checkBoxShowRegions->setChecked(spData->isColoringEnabled());

    m_haveFocus = false;
    // Connect to the drawing mode changed signal
    m_conDrawingModeChanged = m_sigDrawingModeChanged.connect( this, &CDemoPluginPanel::OnDrawingModeChanged );

}

CDemoPluginPanel::~CDemoPluginPanel()
{
    QSettings settings;
    settings.beginGroup("DemoPanel");

    settings.setValue("PackGroup1", !ui->groupBox->isChecked());
    settings.setValue("PackGroup2", !ui->groupBox_2->isChecked());
    settings.setValue("PackGroup3", !ui->groupBox_3->isChecked());
    settings.setValue("PackGroup4", !ui->groupBox_4->isChecked());

    settings.endGroup();

	PLUGIN_APP_STORAGE.disconnect(data::Storage::RegionColoring::Id, &m_colorComboBox);
	PLUGIN_APP_MODE.disconnectAllDrawingHandlers();
    delete ui;
}

void CDemoPluginPanel::packGroupBox(bool checked)
{
    QGroupBox* pWidget = qobject_cast<QGroupBox*>(sender());
    packGroupBox(pWidget, checked);
}

void CDemoPluginPanel::packGroupBox(QGroupBox* pWidget, bool checked)
{
    if (NULL == pWidget) return;
    int height = pWidget->property("ProperHeight").toInt();
    if (checked)
        pWidget->setMaximumHeight(height);
    else
        pWidget->setMaximumHeight(GROUPBOX_HEIGHT);
}

///////////////////////////////////////////////////////////////////////////////
// Callback for point clicked

void CDemoPluginPanel::OnPointPicked( float x, float y, float z, int EventType )
{
    // Get pointer to the volume data
    data::CObjectPtr< data::CActiveDataSet > ptrDataset( PLUGIN_APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr< data::CDensityData > pVolume( PLUGIN_APP_STORAGE.getEntry(ptrDataset->getId()) );

    // Get the density value
    int ix = vpl::math::round2Int(x);
    int iy = vpl::math::round2Int(y);
    int iz = vpl::math::round2Int(z);
    vpl::img::tDensityPixel Value = pVolume->at(ix, iy, iz);

    // Show values
    ui->editDensity->setText(QString::number(Value));
    ui->editCoords->setText(QString("%1 %2 %3").arg(ix).arg(iy).arg(iz));
}

///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::on_pushButtonPickValue_clicked()
{
    // Register the scene hit signal handler
    PLUGIN_APP_MODE.getSceneHitSignal().connect(this, &CDemoPluginPanel::OnPointPicked);
    // Change the mouse mode
    PLUGIN_APP_MODE.storeAndSet(scene::CAppMode::COMMAND_SCENE_HIT);
}

///////////////////////////////////////////////////////////////////////////////
// Set drawing options

void CDemoPluginPanel::setDrawingOptions(const data::CDrawingOptions::EDrawingMode Mode,
                                         const osg::Vec4 &Color,
                                         bool bConnect
                                         )
{
    // Get drawing options from data storage
    data::CObjectPtr< data::CDrawingOptions > spOptions( PLUGIN_APP_STORAGE.getEntry(data::Storage::DrawingOptions::Id) );

    // Setup the drawing mode
    spOptions->setDrawingMode( Mode );
    spOptions->setColor( Color );
    spOptions->setWidth(1);

    // Invalidate the modified object
    PLUGIN_APP_STORAGE.invalidate( spOptions.getEntryPtr() );

    // Set panel connected
    if( bConnect )
    {
        connectSignals();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Handle drawing

void CDemoPluginPanel::handleDrawing( const osg::Vec3Array *points, const int handlerType, const int mouseButton )
{
    if( handlerType == data::CDrawingOptions::FOCUS_LOST )
    {
        // not drawing
        m_drawingMode = data::CDrawingOptions::DRAW_NOTHING;
        m_haveFocus = false;
        return;
    }
    else if( handlerType == data::CDrawingOptions::FOCUS_ON )
    {
        m_haveFocus = true;
        return;
    }

    // Check for empty points
    if( !points || points->empty() )
    {
        return;
    }

    // Handle the drawing...
    switch( m_drawingMode )
    {
        case data::CDrawingOptions::DRAW_STROKE:
            handleStroke( points, handlerType );
            break;

        case data::CDrawingOptions::DRAW_LASO:
            handlePolygon( points, handlerType );
            break;

        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::handleStroke( const osg::Vec3Array * points, const int handlerType )
{
    // Get region volume
    data::CObjectPtr< data::CRegionData > pVolume( PLUGIN_APP_STORAGE.getEntry(data::Storage::RegionData::Id) );

    // Get undo manager
//    data::CObjectPtr< data::CUndoManager > undoManager( PLUGIN_APP_STORAGE.getEntry(data::Storage::UndoManager::Id) );

    // Simply rasterize all points into the region/segmentation volume
    int iRegion = getCurrentRegion();
    for( osg::Vec3Array::const_iterator i = points->begin(); i != points->end(); ++i )
    {
        int x = (int)(i->x()); //vpl::math::round2Int(i->x());
        int y = (int)(i->y());
        int z = (int)(i->z()); 

        if( pVolume->checkPosition(x, y, z) )
        {
            pVolume->at(x, y, z) = data::CRegionData::tVoxel(iRegion);
        }
    }

    // Invalidate regions volume
//    PLUGIN_APP_STORAGE.invalidate( pVolume.getEntryPtr(), data::Storage::FORCE_UPDATE );
    PLUGIN_APP_STORAGE.invalidate( pVolume.getEntryPtr() );
}

///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::handlePolygon( const osg::Vec3Array * points, const int handlerType )
{
    // Get region volume
    data::CObjectPtr< data::CRegionData > pVolume( PLUGIN_APP_STORAGE.getEntry( data::Storage::RegionData::Id ) );

    // Get undo manager
//    data::CObjectPtr< data::CUndoManager > undoManager( PLUGIN_APP_STORAGE.getEntry( data::Storage::UndoManager::Id ) );

    // Simply rasterize all points into the region/segmentation volume
    int iRegion = getCurrentRegion();
    for( osg::Vec3Array::const_iterator i = points->begin(); i != points->end(); ++i )
    {
        int x = (int)(i->x()); // vpl::math::round2Int(i->x());
        int y = (int)(i->y()); // vpl::math::round2Int(i->y());
        int z = (int)(i->z()); // vpl::math::round2Int(i->z());

        if( pVolume->checkPosition(x, y, z) )
        {
            pVolume->at(x, y, z) = iRegion;
        }
    }

    // Invalidate regions volume
//    PLUGIN_APP_STORAGE.invalidate( pVolume.getEntryPtr(), data::Storage::FORCE_UPDATE );
    PLUGIN_APP_STORAGE.invalidate( pVolume.getEntryPtr() );
}

///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::connectSignals()
{
    if( !m_haveFocus )
    {
        m_conDrawingDone = PLUGIN_APP_MODE.connectDrawingHandler( this, & CDemoPluginPanel::handleDrawing );
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::OnDrawingModeChanged(int Mode)
{
    // Setup the application drawing mode...
    switch( Mode )
    {
        case data::CDrawingOptions::DRAW_STROKE:
            updateButtons(ui->pushButtonStroke);
            setDrawingOptions( data::CDrawingOptions::DRAW_STROKE, osg::Vec4(0.0, 1.0, 0.0, 1.0) );
            m_drawingMode = data::CDrawingOptions::DRAW_STROKE;
            break;

        case data::CDrawingOptions::DRAW_LASO:
            updateButtons(ui->pushButtonPolygon);
            setDrawingOptions( data::CDrawingOptions::DRAW_LASO, osg::Vec4(0.0, 1.0, 0.0, 1.0) );
            m_drawingMode = data::CDrawingOptions::DRAW_LASO;
            break;

        // Disable all drawings
        default:
            updateButtons(NULL);
            setDrawingOptions( data::CDrawingOptions::DRAW_NOTHING, osg::Vec4(0.0, 1.0, 0.0, 1.0) );
            m_drawingMode = data::CDrawingOptions::DRAW_NOTHING;            
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
//

void CDemoPluginPanel::updateButtons(QWidget *letBe)
{
    // Update GUI
    ui->pushButtonPolygon->blockSignals(true);
    ui->pushButtonStroke->blockSignals(true);
    if (letBe!=ui->pushButtonPolygon)
        ui->pushButtonPolygon->setChecked(false);
    if (letBe!=ui->pushButtonStroke)
        ui->pushButtonStroke->setChecked(false);
    ui->pushButtonPolygon->blockSignals(false);
    ui->pushButtonStroke->blockSignals(false);
}

void CDemoPluginPanel::on_pushButtonPolygon_toggled(bool checked)
{
    // On/Off
    data::CDrawingOptions::EDrawingMode mode( data::CDrawingOptions::DRAW_NOTHING );
    if( checked )
    {
        mode = data::CDrawingOptions::DRAW_LASO;
    }

    // Change the drawing mode
    m_sigDrawingModeChanged.invoke( mode );
}

void CDemoPluginPanel::on_pushButtonStroke_toggled(bool checked)
{
    // On/Off
    data::CDrawingOptions::EDrawingMode mode( data::CDrawingOptions::DRAW_NOTHING );
    if( checked )
    {
        mode = data::CDrawingOptions::DRAW_STROKE;
    }

    // Change the drawing mode
    m_sigDrawingModeChanged.invoke( mode );
}

///////////////////////////////////////////////////////////////////////////////
// Regions

void CDemoPluginPanel::updateShowRegions(bool bEnable)
{
    ui->checkBoxShowRegions->blockSignals(true);
    ui->checkBoxShowRegions->setChecked(bEnable);
    ui->checkBoxShowRegions->blockSignals(false);
}

int CDemoPluginPanel::getCurrentRegion()
{
    // Get currently active region from the storage
    data::CObjectPtr< data::CRegionColoring > ptrColoring( PLUGIN_APP_STORAGE.getEntry( data::Storage::RegionColoring::Id ) );
    return ptrColoring->getActiveRegion();
}

void CDemoPluginPanel::on_comboBoxRegion_currentIndexChanged(int index)
{
    m_colorComboBox.usualIndexChangedHandler(&PLUGIN_APP_STORAGE,index);
}

void CDemoPluginPanel::on_checkBoxShowRegions_toggled(bool checked)
{
    // Enable/disable coloring based on segmentation results
    data::CObjectPtr<data::CRegionData> spData( PLUGIN_APP_STORAGE.getEntry(data::Storage::RegionData::Id) );
    spData->enableColoring(checked);
    PLUGIN_APP_STORAGE.invalidate(spData.getEntryPtr());
}

void CDemoPluginPanel::setActiveData(int id)
{
    data::CObjectPtr< data::CActiveDataSet > ptrDataset(PLUGIN_APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    ptrDataset->setId(id);
    PLUGIN_APP_STORAGE.invalidate(ptrDataset.getEntryPtr());
}

void CDemoPluginPanel::on_pushButtonAuxData_clicked()
{
    setActiveData(data::AUX_DATA);
}

void CDemoPluginPanel::on_pushButtonPatientData_clicked()
{
    setActiveData(data::PATIENT_DATA);
}

void CDemoPluginPanel::on_pushButtonUseModel_clicked()
{
    data::CObjectPtr< data::CDensityData > pVolumeAux(PLUGIN_APP_STORAGE.getEntry(data::AUX_DATA));
    data::CObjectPtr< data::CDensityData > pVolumePatient(PLUGIN_APP_STORAGE.getEntry(data::PATIENT_DATA));

    data::CDensityData densityData;
    pVolumeAux->makeRef(densityData);
    pVolumeAux->copyProps(*pVolumePatient.get());
    pVolumeAux->resize(pVolumePatient->getSize());
    //pVolumeAux->fillMargin(0);

    //data::CDensityData data;
    //data.copy(*pVolumePatient.get());
    //pVolumeAux->makeRef(data);

    for (int x = 0; x < (int)pVolumePatient->getXSize(); x++)
    {
        for (int y = 0; y < (int)pVolumePatient->getYSize(); y++)
        {
            for (int z = 0; z < (int)pVolumePatient->getZSize(); z++)
            {
                pVolumePatient->set(x, y, z, x*y*z * 500);
                pVolumeAux->set(x, y, z, x*y*z * 500);
            }
        }
    }

    //qDebug() << pVolumePatient->at(50, 50, 50) << " : " << pVolumeAux->at(50, 50, 50);
    //qDebug() << pVolumePatient->at(100, 100, 100) << " : " << pVolumeAux->at(100, 100, 100);

    PLUGIN_APP_STORAGE.invalidate(pVolumePatient.getEntryPtr());
    PLUGIN_APP_STORAGE.invalidate(pVolumeAux.getEntryPtr());
    return;
}
