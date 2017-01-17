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

#include "orthosliceswidget.h"
#include "ui_orthosliceswidget.h"
#include <coremedi/app/Signals.h>
#include <data/COrthoSlice.h>
#include <data/CActiveDataSet.h>

COrthoSlicesWidget::COrthoSlicesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrthoSlicesWidget)
{
    m_bDontNotify=false;

    ui->setupUi(this);

    // Register signal handlers
    m_Connection = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &COrthoSlicesWidget::onNewDensityData);
    m_ConnectionXY = APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).connect(this, &COrthoSlicesWidget::onNewSliceXY);
    m_ConnectionXZ = APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).connect(this, &COrthoSlicesWidget::onNewSliceXZ);
    m_ConnectionYZ = APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).connect(this, &COrthoSlicesWidget::onNewSliceYZ);

    // connect sliders with spins
    QObject::connect(ui->xySpinBox, SIGNAL(valueChanged(int)), ui->xySliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->xySliceSlider,  SIGNAL(valueChanged(int)), ui->xySpinBox, SLOT(setValue(int)));

    QObject::connect(ui->xzSpinBox, SIGNAL(valueChanged(int)), ui->xzSliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->xzSliceSlider,  SIGNAL(valueChanged(int)), ui->xzSpinBox, SLOT(setValue(int)));

    QObject::connect(ui->yzSpinBox, SIGNAL(valueChanged(int)), ui->yzSliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->yzSliceSlider,  SIGNAL(valueChanged(int)), ui->yzSpinBox, SLOT(setValue(int)));

    // Default values
    int position = data::CSlice::INIT_SIZE / 2;
    m_LastXYPosition = m_LastXZPosition = m_LastYZPosition = position;
    ui->xySliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xzSliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->yzSliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xySpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xzSpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->yzSpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xySliceSlider->setValue(position);
    ui->xzSliceSlider->setValue(position);
    ui->yzSliceSlider->setValue(position);
}

COrthoSlicesWidget::~COrthoSlicesWidget()
{
    // De-register signal handlers
    APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_Connection);
    APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).disconnect(m_ConnectionXY);
    APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).disconnect(m_ConnectionXZ);
    APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).disconnect(m_ConnectionYZ);
    delete ui;
}

void COrthoSlicesWidget::onNewDensityData(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::CActiveDataSet> spDataSet( pEntry );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );

    m_bDontNotify=true;
    const vpl::tSize zSize=spVolume->getZSize();
    ui->xySliceSlider->setMaximum( zSize - 1);
    ui->xySpinBox->setMaximum( zSize - 1 );
    m_LastXYPosition = -1;

    const vpl::tSize ySize=spVolume->getYSize();
    ui->xzSliceSlider->setMaximum( ySize - 1 );
    ui->xzSpinBox->setMaximum( ySize - 1 );
    m_LastXZPosition = -1;

    const vpl::tSize xSize=spVolume->getXSize();
    ui->yzSliceSlider->setMaximum( xSize - 1 );
    ui->yzSpinBox->setMaximum( xSize - 1 );
    m_LastYZPosition = -1;

    // TODO: fix problem with onNewSliceXY coming too soon so it doesn't reflect real state
    m_bDontNotify=false;
}

void COrthoSlicesWidget::onNewSliceXY(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice( pEntry );

    ui->xySliceSlider->setValue(spSlice->getPosition());
    m_LastXYPosition = spSlice->getPosition();
}

void COrthoSlicesWidget::onNewSliceXZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice( pEntry );

    ui->xzSliceSlider->setValue(spSlice->getPosition());
    m_LastXZPosition = spSlice->getPosition();
}

void COrthoSlicesWidget::onNewSliceYZ(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice( pEntry );

    ui->yzSliceSlider->setValue(spSlice->getPosition());
    m_LastYZPosition = spSlice->getPosition();
}

void COrthoSlicesWidget::on_xySliceSlider_valueChanged(int value)
{
    if (m_bDontNotify) return;
    if (value!=m_LastXYPosition)
    {
        m_LastXYPosition=value;
        VPL_SIGNAL(SigSetSliceXY).invoke(value);
    }
}

void COrthoSlicesWidget::on_xzSliceSlider_valueChanged(int value)
{
    if (m_bDontNotify) return;
    if (value!=m_LastXZPosition)
    {
        m_LastXZPosition=value;
        VPL_SIGNAL(SigSetSliceXZ).invoke(value);
    }
}

void COrthoSlicesWidget::on_yzSliceSlider_valueChanged(int value)
{
    if (m_bDontNotify) return;
    if (value!=m_LastYZPosition)
    {
        m_LastYZPosition=value;
        VPL_SIGNAL(SigSetSliceYZ).invoke(value);
    }
}

void COrthoSlicesWidget::on_xySliceModeCombo_currentIndexChanged(int index)
{
    VPL_SIGNAL(SigSetSliceModeXY).invoke(static_cast<data::COrthoSlice::EMode>(index));
}

void COrthoSlicesWidget::on_xzSliceModeCombo_currentIndexChanged(int index)
{
    VPL_SIGNAL(SigSetSliceModeXZ).invoke(static_cast<data::COrthoSlice::EMode>(index));
}

void COrthoSlicesWidget::on_yzSliceModeCombo_currentIndexChanged(int index)
{
    VPL_SIGNAL(SigSetSliceModeYZ).invoke(static_cast<data::COrthoSlice::EMode>(index));
}
