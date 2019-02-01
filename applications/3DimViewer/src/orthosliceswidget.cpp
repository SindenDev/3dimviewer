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
#include <data/CArbitrarySlice.h>
#include <QSettings>
#include <QString>
#include <geometry/base/types.h>

COrthoSlicesWidget::COrthoSlicesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrthoSlicesWidget)
{
    m_bDontNotify=false;

    ui->setupUi(this);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_2);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_3);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_4);

    QSettings settings;
    settings.beginGroup("OrthoSlicesPanel");

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


    // Register signal handlers
    m_Connection = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &COrthoSlicesWidget::onNewDensityData);
    m_ConnectionXY = APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).connect(this, &COrthoSlicesWidget::onNewSliceXY);
    m_ConnectionXZ = APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).connect(this, &COrthoSlicesWidget::onNewSliceXZ);
    m_ConnectionYZ = APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).connect(this, &COrthoSlicesWidget::onNewSliceYZ);
    m_ConnectionARB = APP_STORAGE.getEntrySignal(data::Storage::ArbitrarySlice::Id).connect(this, &COrthoSlicesWidget::onNewSliceARB);

    // connect sliders with spins
    QObject::connect(ui->xySpinBox, SIGNAL(valueChanged(int)), ui->xySliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->xySliceSlider,  SIGNAL(valueChanged(int)), ui->xySpinBox, SLOT(setValue(int)));

    QObject::connect(ui->xzSpinBox, SIGNAL(valueChanged(int)), ui->xzSliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->xzSliceSlider,  SIGNAL(valueChanged(int)), ui->xzSpinBox, SLOT(setValue(int)));

    QObject::connect(ui->yzSpinBox, SIGNAL(valueChanged(int)), ui->yzSliceSlider,  SLOT(setValue(int)));
    QObject::connect(ui->yzSliceSlider,  SIGNAL(valueChanged(int)), ui->yzSpinBox, SLOT(setValue(int)));

    QObject::connect(ui->arbSpinBox, SIGNAL(valueChanged(int)), ui->arbSliceSlider, SLOT(setValue(int)));
    QObject::connect(ui->arbSliceSlider, SIGNAL(valueChanged(int)), ui->arbSpinBox, SLOT(setValue(int)));

    // Default values
    int position = data::CSlice::INIT_SIZE / 2;
    double l = geometry::Vec3(data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE).length();
    m_LastXYPosition = m_LastXZPosition = m_LastYZPosition = m_LastARBPosition = position;
    ui->xySliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xzSliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->yzSliceSlider->setRange(0, data::CSlice::INIT_SIZE - 1);

    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    ui->arbSliceSlider->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());
    ui->arbSpinBox->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());

    ui->xySpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xzSpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->yzSpinBox->setRange(0, data::CSlice::INIT_SIZE - 1);
    ui->xySliceSlider->setValue(position);
    ui->xzSliceSlider->setValue(position);
    ui->yzSliceSlider->setValue(position);
    ui->arbSliceSlider->setValue(position);
}

COrthoSlicesWidget::~COrthoSlicesWidget()
{
    QSettings settings;
    settings.beginGroup("OrthoSlicesPanel");

    settings.setValue("PackGroup1", !ui->groupBox->isChecked());
    settings.setValue("PackGroup2", !ui->groupBox_2->isChecked());
    settings.setValue("PackGroup3", !ui->groupBox_3->isChecked());
    settings.setValue("PackGroup4", !ui->groupBox_4->isChecked());

    settings.endGroup();

    // De-register signal handlers
    APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_Connection);
    APP_STORAGE.getEntrySignal(data::Storage::SliceXY::Id).disconnect(m_ConnectionXY);
    APP_STORAGE.getEntrySignal(data::Storage::SliceXZ::Id).disconnect(m_ConnectionXZ);
    APP_STORAGE.getEntrySignal(data::Storage::SliceYZ::Id).disconnect(m_ConnectionYZ);
    APP_STORAGE.getEntrySignal(data::Storage::ArbitrarySlice::Id).disconnect(m_ConnectionARB);
    delete ui;
}

void COrthoSlicesWidget::packGroupBox(bool checked)
{
    QGroupBox* pWidget = qobject_cast<QGroupBox*>(sender());
    packGroupBox(pWidget, checked);
}

void COrthoSlicesWidget::packGroupBox(QGroupBox* pWidget, bool checked)
{
    if (NULL == pWidget) return;
    int height = pWidget->property("ProperHeight").toInt();
    if (checked)
        pWidget->setMaximumHeight(height);
    else
        pWidget->setMaximumHeight(GROUPBOX_HEIGHT);
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

    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    ui->arbSliceSlider->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());
    ui->arbSpinBox->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());
    m_LastARBPosition = -1;

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

void COrthoSlicesWidget::onNewSliceARB(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(pEntry);

    ui->arbSliceSlider->blockSignals(true);
    ui->arbSpinBox->blockSignals(true);
    ui->arbSliceSlider->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());
    ui->arbSpinBox->setRange(spSlice->getPositionMin(), spSlice->getPositionMax());
    ui->arbSliceSlider->blockSignals(false);
    ui->arbSpinBox->blockSignals(false);
    ui->arbSliceSlider->setValue(spSlice->getPosition());
    m_LastARBPosition = spSlice->getPosition();
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

void COrthoSlicesWidget::on_arbSliceSlider_valueChanged(int value)
{
    if (m_bDontNotify)
    {
        return;
    }

    if (value != m_LastARBPosition)
    {
        m_LastARBPosition = value;
        VPL_SIGNAL(SigSetSliceARB).invoke(value);
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

void COrthoSlicesWidget::on_pushButtonAlignXY_clicked()
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    data::CObjectPtr<data::COrthoSliceXY> spSliceXY(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));
    spSlice->setRotationMatrix(osg::Matrix::identity() * osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3(0.0, 0.0, 1.0)));
    spSlice->setPlaneCenter(osg::Vec3(spVolume->getXSize() * spVolume->getDX() * 0.5, spVolume->getYSize() * spVolume->getDY() * 0.5, (spSliceXY->getPosition() + 0.5) * spVolume->getDZ()));
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}
void COrthoSlicesWidget::on_pushButtonAlignXZ_clicked()
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    data::CObjectPtr<data::COrthoSliceXZ> spSliceXZ(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));
    spSlice->setRotationMatrix(osg::Matrix::identity() * osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3f(0.0, 0.0, 1.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(1.0, 0.0, 0.0)));
    spSlice->setPlaneCenter(osg::Vec3(spVolume->getXSize() * spVolume->getDX() * 0.5, (spSliceXZ->getPosition() + 0.5) * spVolume->getDY(), spVolume->getZSize() * spVolume->getDZ() * 0.5));
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}
void COrthoSlicesWidget::on_pushButtonAlignYZ_clicked()
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    data::CObjectPtr<data::COrthoSliceYZ> spSliceYZ(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));
    spSlice->setRotationMatrix(osg::Matrix::identity() * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(0.0, 1.0, 0.0)));
    spSlice->setPlaneCenter(osg::Vec3((spSliceYZ->getPosition() + 0.5) * spVolume->getDX(), spVolume->getYSize() * spVolume->getDY() * 0.5, spVolume->getZSize() * spVolume->getDZ() * 0.5));
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}
