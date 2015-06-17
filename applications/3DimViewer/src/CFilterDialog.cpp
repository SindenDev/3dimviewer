///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#include "CFilterDialog.h"
#include "ui_CFilterDialog.h"
#include <C3DimApplication.h>

#include <QSettings>
#include <QDebug>

#include <data/CDensityData.h>
#include <data/CModelManager.h>
#include <data/CImageLoaderInfo.h>
#include <data/CVolumeTransformation.h>
#include <data/CActiveDataSet.h>
#include <data/CDensityWindow.h>
#include <data/COrthoSlice.h>

#include <VPL/Module/Progress.h>
#include <VPL/Module/Serialization.h>
#include <VPL/Image/VolumeFiltering.h>
#include <VPL/Image/VolumeFunctions.h>

CFilterDialog::CFilterDialog(QWidget *parent, const QString& title) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
    ui(new Ui::CFilterDialog),
	m_curSlice(0),
	m_strength(0),
	m_pSliceFilter(NULL)
{
    ui->setupUi(this);
	connect(ui->buttonBox,SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox,SIGNAL(rejected()), this, SLOT(reject()));
	if (!title.isEmpty())
		setWindowTitle(title);

    data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );
	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
	m_curSlice = spSlice->getPosition();
	ui->sliderSlice->setRange(0,zSize-1);
	ui->sliderSlice->setValue(m_curSlice);
	
    QSettings settings;
    settings.beginGroup("FilterDialog");
	m_strength = settings.value("strength",ui->sliderStrength->value()).toInt();
	ui->sliderStrength->blockSignals(true);
	ui->sliderStrength->setValue(m_strength);
	ui->sliderStrength->blockSignals(false);
    resize(settings.value("size",minimumSize()).toSize());
    settings.endGroup();

	connect(ui->sliderSlice,SIGNAL(valueChanged(int)),this,SLOT(sliceChanged(int)));
	connect(ui->sliderStrength,SIGNAL(valueChanged(int)),this,SLOT(sliderStrengthChanged(int)));
}

CFilterDialog::~CFilterDialog()
{
    QSettings settings;
    settings.beginGroup("FilterDialog");
	if ( Qt::WindowMaximized!=QWidget::windowState ())
		settings.setValue("size",size());
	settings.setValue("strength",m_strength);
    settings.endGroup();
    delete ui;
}

void CFilterDialog::loadCurrentSlice()
{
	if (NULL!=m_pSliceFilter)
	{
		// apply filter on slice
		QImage img;
		(*m_pSliceFilter)(m_curSlice,m_strength,img);
		// set preview
		QPixmap pm(QPixmap::fromImage(img.scaledToHeight(ui->preview->height(),Qt::SmoothTransformation)));
		ui->preview->setIconSize(QSize(pm.width(),pm.height()));
		ui->preview->setIcon(QIcon(pm));
	}	
}

void CFilterDialog::sliceChanged(int slice)
{
	m_curSlice = slice;
	loadCurrentSlice();
}

void CFilterDialog::sliderStrengthChanged(int val)
{
	m_strength = val;
	loadCurrentSlice();
}

void CFilterDialog::resizeEvent(QResizeEvent *e)
{
	QDialog::resizeEvent(e);
	loadCurrentSlice();
}