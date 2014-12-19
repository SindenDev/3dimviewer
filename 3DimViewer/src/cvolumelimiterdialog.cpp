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

#include "cvolumelimiterdialog.h"
#include "ui_cvolumelimiterdialog.h"
#include <QGridLayout>
#include <QSettings>

CVolumeLimiterDialog::CVolumeLimiterDialog(QWidget *parent) :
    QDialog(parent,Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    ui(new Ui::CVolumeLimiterDialog)
{
    ui->setupUi(this);
    m_OrthoXYSlice = NULL;
    m_OrthoXZSlice = NULL;
    m_OrthoYZSlice = NULL;

    // create OSG scenes and setup connections
    createControls();

    // restore last known size
    QSettings settings;
    settings.beginGroup("VolumeLimiterWindow");
    resize(settings.value("size").toSize());
    settings.endGroup();

    // set app mode to slice manipulation
    APP_MODE.set(scene::CAppMode::MODE_SLICE_MOVE);
}

CVolumeLimiterDialog::~CVolumeLimiterDialog()
{
    // disconnect signals
    VPL_SIGNAL(SigSetMinX).disconnect(m_SigConnection[0]);
    VPL_SIGNAL(SigSetMaxX).disconnect(m_SigConnection[1]);
    VPL_SIGNAL(SigSetMinY).disconnect(m_SigConnection[2]);
    VPL_SIGNAL(SigSetMaxY).disconnect(m_SigConnection[3]);
    VPL_SIGNAL(SigSetMinZ).disconnect(m_SigConnection[4]);
    VPL_SIGNAL(SigSetMaxZ).disconnect(m_SigConnection[5]);
    // restore app mode
    APP_MODE.setDefault();
    // save window size
    QSettings settings;
    settings.beginGroup("VolumeLimiterWindow");
    settings.setValue("size",size());
    settings.endGroup();
    delete ui;
}

void   CVolumeLimiterDialog::createControls()
{
    // XY
    m_OrthoXYSlice = new OSGOrtho2DCanvas(this);
    m_SceneXY = new scene::CLimiterXY(m_OrthoXYSlice);
    m_OrthoXYSlice->setScene(m_SceneXY.get());
    m_OrthoXYSlice->centerAndScale();

    // XZ
    m_OrthoXZSlice = new OSGOrtho2DCanvas(this);
    m_SceneXZ = new scene::CLimiterXZ(m_OrthoXZSlice);
    m_OrthoXZSlice->setScene(m_SceneXZ.get());
    m_OrthoXZSlice->centerAndScale();

    // YZ
    m_OrthoYZSlice = new OSGOrtho2DCanvas(this);
    m_SceneYZ = new scene::CLimiterYZ(m_OrthoYZSlice);
    m_OrthoYZSlice->setScene(m_SceneYZ.get());
    m_OrthoYZSlice->centerAndScale();

    {
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(m_OrthoXYSlice,0,0);
        ui->groupBoxXY->setLayout(layout);
    }
    {
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(m_OrthoXZSlice,0,0);
        ui->groupBoxXZ->setLayout(layout);
    }
    {
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(m_OrthoYZSlice,0,0);
        ui->groupBoxYZ->setLayout(layout);
    }

    m_SigConnection[0] = VPL_SIGNAL(SigSetMinX).connect(this, &CVolumeLimiterDialog::updateMinX);
    m_SigConnection[1] = VPL_SIGNAL(SigSetMaxX).connect(this, &CVolumeLimiterDialog::updateMaxX);
    m_SigConnection[2] = VPL_SIGNAL(SigSetMinY).connect(this, &CVolumeLimiterDialog::updateMinY);
    m_SigConnection[3] = VPL_SIGNAL(SigSetMaxY).connect(this, &CVolumeLimiterDialog::updateMaxY);
    m_SigConnection[4] = VPL_SIGNAL(SigSetMinZ).connect(this, &CVolumeLimiterDialog::updateMinZ);
    m_SigConnection[5] = VPL_SIGNAL(SigSetMaxZ).connect(this, &CVolumeLimiterDialog::updateMaxZ);
}

void CVolumeLimiterDialog::setVolume(data::CDensityData *pData, bool bInitLimits)
{

    m_spVolume = pData;
    if( m_spVolume.get() )
    {
        m_SceneXY.get()->setupScene(*m_spVolume);
        m_SceneXZ.get()->setupScene(*m_spVolume);
        m_SceneYZ.get()->setupScene(*m_spVolume);

        m_SliceXY.updateRTG(*m_spVolume);
        m_SliceXZ.updateRTG(*m_spVolume);
        m_SliceYZ.updateRTG(*m_spVolume);
        ui->comboBoxMode->setCurrentIndex(0);

        m_SceneXY->setTextureAndCoordinates(m_SliceXY.getTexturePtr(), 0.0f, m_SliceXY.getTextureWidth(), 0.0f, m_SliceXY.getTextureHeight());
        m_SceneXZ->setTextureAndCoordinates(m_SliceXZ.getTexturePtr(), 0.0f, m_SliceXZ.getTextureWidth(), 0.0f, m_SliceXZ.getTextureHeight());
        m_SceneYZ->setTextureAndCoordinates(m_SliceYZ.getTexturePtr(), 0.0f, m_SliceYZ.getTextureWidth(), 0.0f, m_SliceYZ.getTextureHeight());

        if( bInitLimits )
        {
            data::SVolumeOfInterest Limits;
            Limits.m_MinX = Limits.m_MinY = Limits.m_MinZ = 0;
            Limits.m_MaxX = m_spVolume->getXSize() - 1;
            Limits.m_MaxY = m_spVolume->getYSize() - 1;
            Limits.m_MaxZ = m_spVolume->getZSize() - 1;
            setLimits(Limits);
        }
    }
}

void CVolumeLimiterDialog::setLimits(const data::SVolumeOfInterest& Limits)
{
    m_SceneXY->setLimits(Limits);
    m_SceneXZ->setLimits(Limits);
    m_SceneYZ->setLimits(Limits);

    m_Limits = Limits;

    ui->editMinX->setText(QString::number(m_Limits.m_MinX));
    ui->editMinY->setText(QString::number(m_Limits.m_MinY));
    ui->editMinZ->setText(QString::number(m_Limits.m_MinZ));
    ui->editMaxX->setText(QString::number(m_Limits.m_MaxX));
    ui->editMaxY->setText(QString::number(m_Limits.m_MaxY));
    ui->editMaxZ->setText(QString::number(m_Limits.m_MaxZ));
}


void CVolumeLimiterDialog::on_comboBoxMode_currentIndexChanged(int index)
{
    if (1==index)
    {
        m_SliceXY.updateMIP(*m_spVolume);
        m_SliceXZ.updateMIP(*m_spVolume);
        m_SliceYZ.updateMIP(*m_spVolume);
    }
    else
    {
        m_SliceXY.updateRTG(*m_spVolume);
        m_SliceXZ.updateRTG(*m_spVolume);
        m_SliceYZ.updateRTG(*m_spVolume);
    }
    m_SceneXY->setTextureAndCoordinates(m_SliceXY.getTexturePtr(), 0.0f, m_SliceXY.getTextureWidth(), 0.0f, m_SliceXY.getTextureHeight());
    m_SceneXZ->setTextureAndCoordinates(m_SliceXZ.getTexturePtr(), 0.0f, m_SliceXZ.getTextureWidth(), 0.0f, m_SliceXZ.getTextureHeight());
    m_SceneYZ->setTextureAndCoordinates(m_SliceYZ.getTexturePtr(), 0.0f, m_SliceYZ.getTextureWidth(), 0.0f, m_SliceYZ.getTextureHeight());
}

void CVolumeLimiterDialog::on_buttonResetZoom_clicked()
{
    m_OrthoXYSlice->centerAndScale();
    m_OrthoXZSlice->centerAndScale();
    m_OrthoYZSlice->centerAndScale();
}

void CVolumeLimiterDialog::updateMinX(int iValue)
{
    m_Limits.m_MinX = iValue;
    ui->editMinX->setText(QString::number( vpl::math::getMin(m_Limits.m_MinX, m_Limits.m_MaxX) ));
    ui->editMaxX->setText(QString::number( vpl::math::getMax(m_Limits.m_MinX, m_Limits.m_MaxX) ));
}

void CVolumeLimiterDialog::updateMaxX(int iValue)
{
    m_Limits.m_MaxX = iValue;
    ui->editMinX->setText(QString::number( vpl::math::getMin(m_Limits.m_MinX, m_Limits.m_MaxX) ));
    ui->editMaxX->setText(QString::number( vpl::math::getMax(m_Limits.m_MinX, m_Limits.m_MaxX) ));
}

void CVolumeLimiterDialog::updateMinY(int iValue)
{
    m_Limits.m_MinY = iValue;
    ui->editMinY->setText(QString::number( vpl::math::getMin(m_Limits.m_MinY, m_Limits.m_MaxY) ));
    ui->editMaxY->setText(QString::number( vpl::math::getMax(m_Limits.m_MinY, m_Limits.m_MaxY) ));
}

void CVolumeLimiterDialog::updateMaxY(int iValue)
{
    m_Limits.m_MaxY = iValue;
    ui->editMinY->setText(QString::number( vpl::math::getMin(m_Limits.m_MinY, m_Limits.m_MaxY) ));
    ui->editMaxY->setText(QString::number( vpl::math::getMax(m_Limits.m_MinY, m_Limits.m_MaxY) ));
}

void CVolumeLimiterDialog::updateMinZ(int iValue)
{
    m_Limits.m_MinZ = iValue;
    ui->editMinZ->setText(QString::number( vpl::math::getMin(m_Limits.m_MinZ, m_Limits.m_MaxZ) ));
    ui->editMaxZ->setText(QString::number( vpl::math::getMax(m_Limits.m_MinZ, m_Limits.m_MaxZ) ));
}

void CVolumeLimiterDialog::updateMaxZ(int iValue)
{
    m_Limits.m_MaxZ = iValue;
    ui->editMinZ->setText(QString::number( vpl::math::getMin(m_Limits.m_MinZ, m_Limits.m_MaxZ) ));
    ui->editMaxZ->setText(QString::number( vpl::math::getMax(m_Limits.m_MinZ, m_Limits.m_MaxZ) ));
}
