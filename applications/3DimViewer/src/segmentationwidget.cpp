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
#ifdef __APPLE__
#   include <glew.h>
#else
#   include <GL/glew.h>
#endif

#include "segmentationwidget.h"
#include "ui_segmentationwidget.h"
#include <QColorDialog>
#include <QStyleFactory>
#include <QSettings>
#include <coremedi/app/Signals.h>
#include <data/CDensityData.h>
#include <data/CRegionData.h>
#include <osg/CAppMode.h>
#include <mainwindow.h>
#include <data/CUndoManager.h>

CSegmentationWidget::CSegmentationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SegmentationWidget)
{
    ui->setupUi(this);

    // connect sliders with spins
    QObject::connect(ui->lowSpinBox, SIGNAL(valueChanged(int)), ui->lowThresholdSlider,  SLOT(setValue(int)));
    QObject::connect(ui->lowThresholdSlider,  SIGNAL(valueChanged(int)), ui->lowSpinBox, SLOT(setValue(int)));

    QObject::connect(ui->highSpinBox, SIGNAL(valueChanged(int)), ui->highThresholdSlider,  SLOT(setValue(int)));
    QObject::connect(ui->highThresholdSlider,  SIGNAL(valueChanged(int)), ui->highSpinBox, SLOT(setValue(int)));

    // set default values
    ui->lowThresholdSlider->setValue(500);
    ui->highThresholdSlider->setValue(2000);
    // because style sheets aren't compatible with QProxyStyle that we use
    // we have to set some default style to the button
    ui->buttonColor->setStyle(QStyleFactory::create("windows"));
    ui->buttonColor->setStyleSheet("* { background-color: #ff0000 }");
    m_color.setRgb(255,0,0);

    QSettings settings;
    settings.beginGroup("Segmentation");
    ui->lowThresholdSlider->setValue(settings.value("LowThreshold",ui->lowThresholdSlider->value()).toInt());
    ui->highThresholdSlider->setValue(settings.value("HighThreshold",ui->highThresholdSlider->value()).toInt());
    ui->checkBoxApplyThresholds->setChecked(settings.value("ApplyThresholds",ui->checkBoxApplyThresholds->isChecked()).toBool());

	ui->pushButtonSetToActiveRegion->setVisible(false);
	QTimer::singleShot(0,this,SLOT(firstEvent()));	
}

CSegmentationWidget::~CSegmentationWidget()
{
    QSettings settings;
    settings.beginGroup("Segmentation");
    settings.setValue("LowThreshold",ui->lowThresholdSlider->value());
    settings.setValue("HighThreshold",ui->highThresholdSlider->value());
    settings.setValue("ApplyThresholds",ui->checkBoxApplyThresholds->isChecked());
    delete ui;
}

void CSegmentationWidget::firstEvent()
{
	ui->pushButtonSetToActiveRegion->setVisible(MainWindow::getInstance()->findPluginByID("RegionControl"));
}

int CSegmentationWidget::getLo()
{
    return ui->lowThresholdSlider->value();
}

int CSegmentationWidget::getHi()
{
    return ui->highThresholdSlider->value();
}

void CSegmentationWidget::on_buttonColor_clicked()
{
    QColorDialog dlg(this);
    m_color=dlg.getColor(m_color,this);
    if (m_color.isValid())
    {
        QString str="* { background-color: "+m_color.name()+" }";
        ui->buttonColor->setStyleSheet(str);
        if (ui->checkBoxApplyThresholds->isChecked())
            SetColoring();
    }
}

void CSegmentationWidget::on_checkBoxApplyThresholds_clicked()
{
    if (ui->checkBoxApplyThresholds->isChecked())
        SetColoring();
    else
        VPL_SIGNAL(SigSetColoring).invoke(new data::CNoColoring()); // version 1
}

void CSegmentationWidget::SetColoring()
{
    static const unsigned char ALPHA = 128;
    data::CColor4b MyColor(m_color.red(), m_color.green(), m_color.blue(), ALPHA);

    // === Version 1 ===
    // Use density window LUT (Look-Up Table) to show the thresholding result
    VPL_SIGNAL(SigSetColoring).invoke(new data::CConstColoring(ui->lowThresholdSlider->value(),
                                                               ui->highThresholdSlider->value(),
                                                               MyColor));

}

void CSegmentationWidget::on_highThresholdSlider_valueChanged(int value)
{
    if (ui->checkBoxApplyThresholds->isChecked())
        SetColoring();
}

void CSegmentationWidget::on_lowThresholdSlider_valueChanged(int value)
{
    if (ui->checkBoxApplyThresholds->isChecked())
        SetColoring();
}

void CSegmentationWidget::on_buttonPickLo_clicked()
{
    // Register the scene hit signal handler
    APP_MODE.getSceneHitSignal().connect(this, &CSegmentationWidget::setLowerThreshold);

    // Change the mouse mode
    APP_MODE.storeAndSet(scene::CAppMode::COMMAND_SCENE_HIT);
}

void CSegmentationWidget::on_buttonPickHi_clicked()
{
    // Register the scene hit signal handler
    APP_MODE.getSceneHitSignal().connect(this, &CSegmentationWidget::setHigherThreshold);

    // Change the mouse mode
    APP_MODE.storeAndSet(scene::CAppMode::COMMAND_SCENE_HIT);

}

////////////////////////////////////////////////////////////
/**
 * Function called on scene hit...
 */

void CSegmentationWidget::setLowerThreshold(float x, float y, float z, int EventType)
{
    // Get pointer to the volume data
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()) );

    // Get the density value
    vpl::img::tDensityPixel Value = spVolume->at(vpl::math::round2Int(x),
                                                  vpl::math::round2Int(y),
                                                  vpl::math::round2Int(z)
                                                  );

    // Set the threshold
    ui->lowThresholdSlider->setValue(Value);

    if (ui->checkBoxApplyThresholds->isChecked())
        SetColoring();
}

void CSegmentationWidget::setHigherThreshold(float x, float y, float z, int EventType)
{
    // Get pointer to the volume data
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()) );

    // Get the density value
    vpl::img::tDensityPixel Value = spVolume->at(vpl::math::round2Int(x),
                                                  vpl::math::round2Int(y),
                                                  vpl::math::round2Int(z)
                                                  );

    // Set the threshold
    ui->highThresholdSlider->setValue(Value);

    if (ui->checkBoxApplyThresholds->isChecked())
        SetColoring();
}

void CSegmentationWidget::on_pushButtonCreateSurfaceModel_clicked()
{
	MainWindow::getInstance()->createSurfaceModel();
}

void CSegmentationWidget::on_pushButtonSetToActiveRegion_clicked()
{
	const int low = ui->lowThresholdSlider->value();
	const int high = ui->highThresholdSlider->value();
	data::CObjectPtr< data::CRegionColoring > ptrColoring( APP_STORAGE.getEntry( data::Storage::RegionColoring::Id ) );
    int region( ptrColoring->getActiveRegion() );

	data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );    
	data::CObjectPtr< data::CRegionData  > pRegion( APP_STORAGE.getEntry( data::Storage::RegionData::Id ) );

    data::CObjectPtr< data::CUndoManager > undoManager( APP_STORAGE.getEntry( data::Storage::UndoManager::Id ) );
    undoManager->insert( pRegion->getVolumeSnapshot() );

    pRegion->enableColoring();
	int xSize = pRegion->getXSize();
	int ySize = pRegion->getYSize();
	int zSize = pRegion->getZSize();
#pragma omp parallel for
	for(int z=0; z<zSize; z++)
	{
		for(int y=0; y<ySize; y++)
		{
			for(int x=0; x<xSize; x++)
			{
				vpl::img::tDensityPixel val = spData->at(x,y,z);
				if (pRegion->at(x,y,z) == 0 || pRegion->at(x,y,z) == region)
				{
					if (val>=low && val<=high)
						pRegion->at(x,y,z) = region;
					else
						if (pRegion->at(x,y,z) == region)
							pRegion->at(x,y,z) = 0;
				}
			}
		}
	}
	APP_STORAGE.invalidate( pRegion.getEntryPtr() );
}
