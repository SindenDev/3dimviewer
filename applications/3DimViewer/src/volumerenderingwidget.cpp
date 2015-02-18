///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include "volumerenderingwidget.h"
#include "ui_volumerenderingwidget.h"

#include <render/PSVRrenderer.h>
#include <osg/OSGCanvas.h>
#include <data/CAppSettings.h>
#include <controls/ccollapsiblegroupbox.h>

#include <QSettings>
#include <QGroupBox>

CVolumeRenderingWidget::CVolumeRenderingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeRenderingWidget)
{
    m_pRenderer=NULL;
    ui->setupUi(this);

    ui->labelWindShift->associateWithSlider(ui->sliderWindShift,0);
    ui->labelWindWidth->associateWithSlider(ui->sliderWindWidth,0);
    ui->labelBrightness->associateWithSlider(ui->sliderBrightness,0);
    ui->labelContrast->associateWithSlider(ui->sliderContrast,0);
    ui->labelSurfSharpness->associateWithSlider(ui->sliderSurfSharpness,0);
    ui->labelSurfTolerance->associateWithSlider(ui->sliderSurfTolerance,0);

    VPL_SIGNAL(SigVRDataRemapChange).connect(this, &CVolumeRenderingWidget::vrDataRemapChange);

    // load settings
    QSettings settings;
    settings.beginGroup("VolumeRendering");
    ui->sliderBrightness->setValue(settings.value("Brightness",ui->sliderBrightness->value()).toInt());
    ui->sliderContrast->setValue(settings.value("Contrast",ui->sliderContrast->value()).toInt());
    ui->sliderCuttingPlane->setValue(settings.value("CuttingPlane",ui->sliderCuttingPlane->value()).toInt());
    ui->sliderSurfSharpness->setValue(settings.value("SurfSharpness",ui->sliderSurfSharpness->value()).toInt());
    ui->sliderSurfTolerance->setValue(settings.value("SurfTolerance",ui->sliderSurfTolerance->value()).toInt());
    ui->sliderWindShift->setValue(settings.value("WindShift",ui->sliderWindShift->value()).toInt());
    ui->sliderWindWidth->setValue(settings.value("WindWidth",ui->sliderWindWidth->value()).toInt());

	SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox);
	SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_2);
	SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_3);
	SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_4);
	SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_5);
    if (settings.value("PackGroup1",false).toBool())
    {
        ui->groupBox->setChecked(false);
        packGroupBox(ui->groupBox,false);
    }
    if (settings.value("PackGroup2",false).toBool())
    {
        ui->groupBox_2->setChecked(false);
        packGroupBox(ui->groupBox_2,false);
    }
    if (settings.value("PackGroup3",false).toBool())
    {
        ui->groupBox_3->setChecked(false);
        packGroupBox(ui->groupBox_3,false);
    }
    if (settings.value("PackGroup4",false).toBool())
    {
        ui->groupBox_4->setChecked(false);
        packGroupBox(ui->groupBox_4,false);
    }
    if (settings.value("PackGroup5",false).toBool())
    {
        ui->groupBox_5->setChecked(false);
        packGroupBox(ui->groupBox_5,false);
    }
}

CVolumeRenderingWidget::~CVolumeRenderingWidget()
{
    // save settings
    QSettings settings;
    settings.beginGroup("VolumeRendering");
    settings.setValue("Brightness",ui->sliderBrightness->value());
    settings.setValue("Contrast",ui->sliderContrast->value());
    settings.setValue("CuttingPlane",ui->sliderCuttingPlane->value());
    settings.setValue("SurfSharpness",ui->sliderSurfSharpness->value());
    settings.setValue("SurfTolerance",ui->sliderSurfTolerance->value());
    settings.setValue("WindShift",ui->sliderWindShift->value());
    settings.setValue("WindWidth",ui->sliderWindWidth->value());
	settings.setValue("PackGroup5",!ui->groupBox_5->isChecked());
	settings.setValue("PackGroup4",!ui->groupBox_4->isChecked());
	settings.setValue("PackGroup3",!ui->groupBox_3->isChecked());
	settings.setValue("PackGroup2",!ui->groupBox_2->isChecked());
	settings.setValue("PackGroup1",!ui->groupBox->isChecked());
    delete ui;
}

void CVolumeRenderingWidget::showEvent ( QShowEvent * event )
{
    Q_ASSERT(m_pRenderer);
    const int shader=m_pRenderer->getShader();
	QRadioButton *pRB = NULL;
    switch(shader)
    {
	case PSVR::PSVolumeRendering::MIP: pRB = ui->radioButtonMIP; break;
		break;
    case PSVR::PSVolumeRendering::SHADING: pRB = ui->radioButtonShaded; break;
    case PSVR::PSVolumeRendering::XRAY: pRB = ui->radioButtonXRay; break;
	case PSVR::PSVolumeRendering::CUSTOM: pRB = ui->radioButtonCustom; break;
    default:
		pRB = ui->radioButtonSurface;
    }
	if (NULL!=pRB)
	{
		//pRB->blockSignals(true); 
		pRB->toggle(); 
		//pRB->blockSignals(false); 
	}
    QWidget::showEvent(event);
}


void CVolumeRenderingWidget::setRenderer(PSVR::PSVolumeRendering *pRenderer)
{
    m_pRenderer = pRenderer;
    ui->sliderRenderingQuality->setValue(m_pRenderer->getQuality());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Set background color.
//!
//!\param   color   The color.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CVolumeRenderingWidget::setBackgroundColor(const osg::Vec4 &color)
{
    if (NULL==m_pRenderer) return;
    OSGCanvas * canvas( m_pRenderer->getCanvas() );
    if( canvas )
        canvas->setBackgroundColor( color );
}

void CVolumeRenderingWidget::on_radioButtonMIP_toggled(bool checked)
{
    if (NULL==m_pRenderer) return;
    if (checked)
    {
        int lut = (m_pRenderer->getShader() == PSVR::PSVolumeRendering::MIP ? m_pRenderer->getLut() : PSVR::PSVolumeRendering::MIP_HARD);
        ui->comboBoxColoring->clear();
        ui->comboBoxColoring->addItem(tr("Soft tissues"),SoftTissues);
        ui->comboBoxColoring->addItem(tr("Bone tissues"),BoneTissues);
        ui->comboBoxColoring->setCurrentIndex(vrLutToLutCode(lut) - SoftTissues);
        m_pRenderer->setShader(PSVR::PSVolumeRendering::MIP);
        m_pRenderer->setLut(lut);
        data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
        setBackgroundColor( appSettings->getClearColor() );
        //setBackgroundColor( osg::Vec4( 0.0,0.0,0.0,1.0 ) );
        m_pRenderer->redraw();

        ui->sliderSurfSharpness->setEnabled(false);
        ui->sliderSurfTolerance->setEnabled(false);
        ui->sliderCuttingPlane->setEnabled(false);
    }
}

void CVolumeRenderingWidget::on_radioButtonShaded_toggled(bool checked)
{
    if (NULL==m_pRenderer) return;
    if (checked)
    {
        int lut = (m_pRenderer->getShader() == PSVR::PSVolumeRendering::SHADING ? m_pRenderer->getLut() : PSVR::PSVolumeRendering::SHA_TRAN);
        ui->comboBoxColoring->clear();
        ui->comboBoxColoring->addItem(tr("Transparent"),Transparent);
        ui->comboBoxColoring->addItem(tr("Air pockets"),AirPockets);
        ui->comboBoxColoring->addItem(tr("Bones (skull)"),Skull);
        ui->comboBoxColoring->addItem(tr("Bones (spine)"),Spine);
        ui->comboBoxColoring->addItem(tr("Bones (pelvis)"),Pelvis);
        ui->comboBoxColoring->setCurrentIndex(vrLutToLutCode(lut) - Transparent);
        m_pRenderer->setShader(PSVR::PSVolumeRendering::SHADING);
        m_pRenderer->setLut(lut);
        data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
        setBackgroundColor( appSettings->getClearColor() );
        m_pRenderer->redraw();

        ui->sliderSurfSharpness->setEnabled(false);
        ui->sliderSurfTolerance->setEnabled(false);
        ui->sliderCuttingPlane->setEnabled(true);
    }
}

void CVolumeRenderingWidget::on_radioButtonXRay_toggled(bool checked)
{
    if (NULL==m_pRenderer) return;
    if (checked)
    {
        int lut = (m_pRenderer->getShader() == PSVR::PSVolumeRendering::XRAY ? m_pRenderer->getLut() : PSVR::PSVolumeRendering::XRAY_HARD);
        ui->comboBoxColoring->clear();
        ui->comboBoxColoring->addItem(tr("Enhance soft"),EnhanceSoft);
        ui->comboBoxColoring->addItem(tr("Enhance hard"),EnhanceHard);
        ui->comboBoxColoring->setCurrentIndex(vrLutToLutCode(lut) - EnhanceSoft);
        m_pRenderer->setShader(PSVR::PSVolumeRendering::XRAY);
        m_pRenderer->setLut(lut);
        data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
        setBackgroundColor( appSettings->getClearColor() );
        //setBackgroundColor( osg::Vec4( 0.0f,0.0f,0.0f,1.0 ) );
        m_pRenderer->redraw();

        ui->sliderSurfSharpness->setEnabled(false);
        ui->sliderSurfTolerance->setEnabled(false);
        ui->sliderCuttingPlane->setEnabled(false);
    }
}

void CVolumeRenderingWidget::on_radioButtonCustom_toggled(bool checked)
{
    if (NULL==m_pRenderer) return;
    if (checked)
    {
        ui->comboBoxColoring->clear();
        ui->comboBoxColoring->addItem(tr("Enhance soft"),EnhanceSoft);
        ui->comboBoxColoring->addItem(tr("Enhance hard"),EnhanceHard);
        ui->comboBoxColoring->setCurrentIndex(1);
        m_pRenderer->setShader(PSVR::PSVolumeRendering::CUSTOM);
        m_pRenderer->setLut(PSVR::PSVolumeRendering::XRAY_HARD);
        data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
        setBackgroundColor( appSettings->getClearColor() );
        //setBackgroundColor( osg::Vec4( 0.0f,0.0f,0.0f,1.0 ) );
        m_pRenderer->redraw();

        ui->sliderSurfSharpness->setEnabled(false);
        ui->sliderSurfTolerance->setEnabled(false);
        ui->sliderCuttingPlane->setEnabled(false);
    }
}
void CVolumeRenderingWidget::on_radioButtonSurface_toggled(bool checked)
{
    if (NULL==m_pRenderer) return;
    if (checked)
    {
        int lut = (m_pRenderer->getShader() == PSVR::PSVolumeRendering::SURFACE ? m_pRenderer->getLut() : PSVR::PSVolumeRendering::SURFACE_BONE);
        ui->comboBoxColoring->clear();
        ui->comboBoxColoring->addItem(tr("Bone surface"),BoneSurface);
        ui->comboBoxColoring->addItem(tr("Skin surface"),SkinSurface);
        ui->comboBoxColoring->setCurrentIndex(vrLutToLutCode(lut) - BoneSurface);
        m_pRenderer->setShader(PSVR::PSVolumeRendering::SURFACE);
        m_pRenderer->setLut(lut);
        data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
        setBackgroundColor( appSettings->getClearColor() );
        //setBackgroundColor( osg::Vec4( 0.2f,0.2f,0.4f,1.0 ) );
        m_pRenderer->redraw();

        ui->sliderSurfSharpness->setEnabled(true);
        ui->sliderSurfTolerance->setEnabled(true);
        ui->sliderCuttingPlane->setEnabled(true);
    }
}

void CVolumeRenderingWidget::on_comboBoxColoring_currentIndexChanged(int index)
{
    if (NULL==m_pRenderer) return;

    int lutCode=Default;
    if (index>=0)
        lutCode=ui->comboBoxColoring->itemData(index).toInt();

    int vrLut = lutCodeToVrLut(lutCode);
    m_pRenderer->setLut(vrLut);
    /*switch (lutCode)
    {
    case SoftTissues:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::MIP_SOFT);
        break;
    case BoneTissues:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::MIP_HARD);
        break;
    case Transparent:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SHA_TRAN);
        break;
    case AirPockets:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SHA_AIR);
        break;
    case Skull:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SHA_BONE0);
        break;
    case Spine:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SHA_BONE1);
        break;
    case Pelvis:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SHA_BONE2);
        break;
    case EnhanceSoft:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::XRAY_SOFT);
        break;
    case EnhanceHard:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::XRAY_HARD);
        break;
    case BoneSurface:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SURFACE_BONE);
        break;
    case SkinSurface:
        m_pRenderer->setLut(PSVR::PSVolumeRendering::SURFACE_SKIN);
        break;
    }*/
    m_pRenderer->redraw();
}

int CVolumeRenderingWidget::vrLutToLutCode(int index)
{
    switch (index)
    {
    case PSVR::PSVolumeRendering::MIP_SOFT:     return SoftTissues;
    case PSVR::PSVolumeRendering::MIP_HARD:     return BoneTissues;
    case PSVR::PSVolumeRendering::SHA_TRAN:     return Transparent;
    case PSVR::PSVolumeRendering::SHA_AIR:      return AirPockets;
    case PSVR::PSVolumeRendering::SHA_BONE0:    return Skull;
    case PSVR::PSVolumeRendering::SHA_BONE1:    return Spine;
    case PSVR::PSVolumeRendering::SHA_BONE2:    return Pelvis;
    case PSVR::PSVolumeRendering::XRAY_SOFT:    return EnhanceSoft;
    case PSVR::PSVolumeRendering::XRAY_HARD:    return EnhanceHard;
    case PSVR::PSVolumeRendering::SURFACE_BONE: return BoneSurface;
    case PSVR::PSVolumeRendering::SURFACE_SKIN: return SkinSurface;
    default:                                    return Default;
    }
}

int CVolumeRenderingWidget::lutCodeToVrLut(int index)
{
    switch (index)
    {
    case SoftTissues:   return PSVR::PSVolumeRendering::MIP_SOFT;
    case BoneTissues:   return PSVR::PSVolumeRendering::MIP_HARD;
    case Transparent:   return PSVR::PSVolumeRendering::SHA_TRAN;
    case AirPockets:    return PSVR::PSVolumeRendering::SHA_AIR;
    case Skull:         return PSVR::PSVolumeRendering::SHA_BONE0;
    case Spine:         return PSVR::PSVolumeRendering::SHA_BONE1;
    case Pelvis:        return PSVR::PSVolumeRendering::SHA_BONE2;
    case EnhanceSoft:   return PSVR::PSVolumeRendering::XRAY_SOFT;
    case EnhanceHard:   return PSVR::PSVolumeRendering::XRAY_HARD;
    case BoneSurface:   return PSVR::PSVolumeRendering::SURFACE_BONE;
    case SkinSurface:   return PSVR::PSVolumeRendering::SURFACE_SKIN;
    default:            return PSVR::PSVolumeRendering::LOOKUPS_COUNT;
    }
}

void CVolumeRenderingWidget::on_sliderRenderingQuality_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;
    m_pRenderer->setQuality(value);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderCuttingPlane_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;
    float displacement = float(value) / 100.0f;
    m_pRenderer->setCuttingPlaneDisplacement(displacement);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderWindShift_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;
    float shift = (float)value / 500.0f;
    float expand = ((float)(ui->sliderWindWidth->value() + 100.0f) / 100.0f);
    m_pRenderer->setDataRemap(expand, shift);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderWindWidth_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;
    float shift = (float)ui->sliderWindShift->value() / 500.0f;
    float expand = ((float)(value + 100.0f) / 100.0f);
    m_pRenderer->setDataRemap(expand, shift);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderBrightness_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;

    float brightness = (float)value / 300.0;
    float contrast = pow(((float)(ui->sliderContrast->value() + 100.0) / 100.0), 3);
    m_pRenderer->setPicture(brightness, contrast);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderContrast_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;

    float brightness = (float)ui->sliderBrightness->value() / 300.0;
    float contrast = pow(((float)(value + 100.0) / 100.0), 3);
    m_pRenderer->setPicture(brightness, contrast);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderSurfTolerance_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;

    // defaults:
    //	surfaceNormalMult = 15;
    //  surfaceNormalExp = 2;

    float mult = (((float)(value + 100.0) / 200.0) * 28.0) + 1.0;
    float sharp = (((float)(ui->sliderSurfSharpness->value() + 100.0) / 200.0) * 3.0) + 1.0;
    m_pRenderer->setSurfaceDetection(mult, sharp);
    m_pRenderer->redraw();
}

void CVolumeRenderingWidget::on_sliderSurfSharpness_valueChanged(int value)
{
    if( NULL==m_pRenderer )
        return;

    // defaults:
    //	surfaceNormalMult = 15;
    //  surfaceNormalExp = 2;

    float mult = (((float)(ui->sliderSurfTolerance->value() + 100.0) / 200.0) * 28.0) + 1.0;
    float sharp = (((float)(value + 100.0) / 200.0) * 3.0) + 1.0;
    m_pRenderer->setSurfaceDetection(mult, sharp);
    m_pRenderer->redraw();
}

#define myround(x) (x<0?ceil((x)-0.5):floor((x)+0.5))

void    CVolumeRenderingWidget::vrDataRemapChange(float expand, float offset)
{
    ui->sliderWindShift->blockSignals(true);
    ui->sliderWindWidth->blockSignals(true);
    ui->sliderWindShift->setValue(myround(offset*500));
    ui->sliderWindWidth->setValue(myround(100*expand - 100));
    ui->sliderWindShift->blockSignals(false);
    ui->sliderWindWidth->blockSignals(false);
}

void CVolumeRenderingWidget::packGroupBox(bool checked)
{
    QGroupBox* pWidget = qobject_cast<QGroupBox*>(sender());    
    packGroupBox(pWidget,checked);
}

void CVolumeRenderingWidget::packGroupBox(QGroupBox* pWidget, bool checked)
{
    if (NULL==pWidget) return;
    int height = pWidget->property("ProperHeight").toInt();
    if (checked)
        pWidget->setMaximumHeight(height);
    else
        pWidget->setMaximumHeight(GROUPBOX_HEIGHT);
}

