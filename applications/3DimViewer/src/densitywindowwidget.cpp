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

#include "densitywindowwidget.h"
#include "ui_densitywindowwidget.h"

#include <data/CActiveDataSet.h>
#include <data/CDensityData.h>

#include <coremedi/app/Signals.h>

CDensityWindowWidget::CDensityWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DensityWindowWidget)
{
    m_bDontNotify=false;

    ui->setupUi(this);

    ui->densityWindowCenter->setValue(data::DEFAULT_DENSITY_WINDOW.m_Center);
    ui->densityWindowCenter->setRange(data::CDensityWindow::getMinDensity(), data::CDensityWindow::getMaxDensity());
    ui->densityWindowWidth->setValue(data::DEFAULT_DENSITY_WINDOW.m_Width);
    ui->densityWindowWidth->setRange(0, data::CDensityWindow::getDensityRange());
    ui->densityWindowCenterSlider->setValue(data::DEFAULT_DENSITY_WINDOW.m_Center);
    ui->densityWindowCenterSlider->setRange(data::CDensityWindow::getMinDensity(), data::CDensityWindow::getMaxDensity());
    ui->densityWindowWidthSlider->setValue(data::DEFAULT_DENSITY_WINDOW.m_Width);
    ui->densityWindowWidthSlider->setRange(0, data::CDensityWindow::getDensityRange());

    QObject::connect(ui->densityWindowCenterSlider,SIGNAL(valueChanged(int)),this,SLOT(on_densityWindowCenter_valueChanged(int)));
    QObject::connect(ui->densityWindowWidthSlider,SIGNAL(valueChanged(int)),this,SLOT(on_densityWindowWidth_valueChanged(int)));

    CGeneralObjectObserver<CDensityWindowWidget>::connect(APP_STORAGE.getEntry(data::Storage::DensityWindow::Id ).get(), CGeneralObjectObserver<CDensityWindowWidget>::tObserverHandler(this, &CDensityWindowWidget::onNewDensityWindow));
    CGeneralObjectObserver<CDensityWindowWidget>::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id ).get(), CGeneralObjectObserver<CDensityWindowWidget>::tObserverHandler(this, &CDensityWindowWidget::onNewDensityData));
}

CDensityWindowWidget::~CDensityWindowWidget()
{
    CGeneralObjectObserver<CDensityWindowWidget>::connect(APP_STORAGE.getEntry(data::Storage::DensityWindow::Id ).get());
    CGeneralObjectObserver<CDensityWindowWidget>::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id ).get());
    delete ui;
}

void CDensityWindowWidget::objectChanged(data::CStorageEntry *,const data::CChangedEntries &)
{
}

void CDensityWindowWidget::onNewDensityWindow(data::CStorageEntry * pEntry,const data::CChangedEntries &)
{    
    data::CDensityWindow *pData = pEntry->getDataPtr<data::CObjectHolder<data::CDensityWindow> >()->getObjectPtr();
    m_bDontNotify=true; // setValue would cause another objectChanged with outdated width value
    ui->densityWindowCenter->setValue(pData->getCenter());
    ui->densityWindowCenterSlider->setValue(pData->getCenter());
    m_bDontNotify=false;
    ui->densityWindowWidth->setValue(pData->getWidth());
    ui->densityWindowWidthSlider->setValue(pData->getWidth());

}

void CDensityWindowWidget::onNewDensityData(data::CStorageEntry *,const data::CChangedEntries &)
{    
    m_bDontNotify=true;
    ui->densityWindowCenter->setRange(data::CDensityWindow::getMinDensity(), data::CDensityWindow::getMaxDensity());
    ui->densityWindowCenterSlider->setRange(data::CDensityWindow::getMinDensity(), data::CDensityWindow::getMaxDensity());
    ui->densityWindowWidth->setRange(1, data::CDensityWindow::getDensityRange());
    ui->densityWindowWidthSlider->setRange(1, data::CDensityWindow::getDensityRange());
    m_bDontNotify=false;
}

void CDensityWindowWidget::on_pushButton_clicked()
{
    VPL_SIGNAL(SigSetDensityWindow).invoke(data::DEFAULT_DENSITY_WINDOW.m_Center,
                                           data::DEFAULT_DENSITY_WINDOW.m_Width,
										   data::Storage::DensityWindow::Id
                                           );
}

void CDensityWindowWidget::on_pushButton_3_clicked()
{
    VPL_SIGNAL(SigSetDensityWindow).invoke(data::BONES_DENSITY_WINDOW.m_Center,
                                           data::BONES_DENSITY_WINDOW.m_Width,
										   data::Storage::DensityWindow::Id
                                           );
}

void CDensityWindowWidget::on_densityWindowCenter_valueChanged(int center)
{
    if (m_bDontNotify) return;
    int width=ui->densityWindowWidth->value();
    m_bDontNotify=true;
    ui->densityWindowCenter->setValue(center);
    ui->densityWindowCenterSlider->setValue(center);
    m_bDontNotify=false;
	VPL_SIGNAL(SigSetDensityWindow).invoke(center, width, data::Storage::DensityWindow::Id);
}

void CDensityWindowWidget::on_densityWindowWidth_valueChanged(int width)
{
    if (m_bDontNotify) return;
    int center=ui->densityWindowCenter->value();
    m_bDontNotify=true;
    ui->densityWindowWidth->setValue(width);
    ui->densityWindowWidthSlider->setValue(width);
    m_bDontNotify=false;
	VPL_SIGNAL(SigSetDensityWindow).invoke(center, width, data::Storage::DensityWindow::Id);
}

void CDensityWindowWidget::on_pushButton_2_clicked()
{
    // First, call signal that calculates optimal density window
    data::SDensityWindow NewWindow = VPL_SIGNAL(SigEstimateDensityWindow).invoke2();
    // Set the density window
	VPL_SIGNAL(SigSetDensityWindow).invoke(NewWindow.m_Center, NewWindow.m_Width, data::Storage::DensityWindow::Id);
}
