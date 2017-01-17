///////////////////////////////////////////////////////////////////////////////
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

#include "CPreviewDialog.h"
#include "ui_CPreviewDialog.h"
#include <C3DimApplication.h>

#include <QSettings>
#include <QDebug>
#include <QTimer>

CPreviewDialog::CPreviewDialog(QWidget *parent, const QString& title, CPreviewDialogData& data) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
	ui(new Ui::CPreviewDialog),
	m_data(data),
	m_filterNames(data.getFiltersNames()),
	m_currentSlice(data.getCurrentSlice()),
	m_currentFilterIndex(data.getCurrentFilterIndex()),
	m_paramsDesc(data.getParamDescription(m_currentFilterIndex)),
	m_loadSlice(false),
	m_sliderPressed(false)
{
	// if current slice is out of the range of slices that can be shown, set it in the middle of the volume
	int hiddenSlices = data.getHiddenSliceCnt();
	(m_currentSlice < hiddenSlices || m_currentSlice > m_data.getVolumeZSize() - 1 - hiddenSlices ? m_currentSlice = m_data.getVolumeZSize() / 2 : m_currentSlice = m_currentSlice);

    ui->setupUi(this);

	// create radio buttons for filters
	for (int i = 0; i < m_filterNames.size(); ++i)
	{
		m_filterTypes.push_back(new QRadioButton(m_filterNames[i], this));
		ui->horizontalLayoutTypes->addWidget(m_filterTypes[i]);
		connect(m_filterTypes[i], SIGNAL(toggled(bool)), this, SLOT(filterTypeChanged(bool)));

		if (i == m_currentFilterIndex)
		{
			m_filterTypes[i]->setChecked(true);
		}
	}

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	if (!title.isEmpty())
	{
		setWindowTitle(title);
	}

	// fill vectors of widgets
	m_paramLabels.push_back(ui->labelParam1);
	m_paramLabels.push_back(ui->labelParam2);
	m_paramLabels.push_back(ui->labelParam3);

	m_paramSliders.push_back(ui->sliderParam1);
	m_paramSliders.push_back(ui->sliderParam2);
	m_paramSliders.push_back(ui->sliderParam3);

	m_paramSpinBoxes.push_back(ui->doubleSpinBoxParam1);
	m_paramSpinBoxes.push_back(ui->doubleSpinBoxParam2);
	m_paramSpinBoxes.push_back(ui->doubleSpinBoxParam3);

	// set widget based on filter parametrs
	setParamsProperties();

	// set range of slider that changes slices
	ui->sliderSlice->blockSignals(true);
	ui->sliderSlice->setRange(hiddenSlices, m_data.getVolumeZSize() - 1 - hiddenSlices);
	ui->sliderSlice->setValue(m_currentSlice);
	ui->sliderSlice->blockSignals(false);

	// connect actions
	connect(ui->sliderSlice, SIGNAL(valueChanged(int)), this, SLOT(sliceChanged(int)));
	connect(ui->sliderSlice, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(ui->sliderSlice, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
	connect(ui->sliderSlice, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int)));

	connect(ui->sliderParam1, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(ui->sliderParam2, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
	connect(ui->sliderParam3, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

	connect(ui->sliderParam1, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
	connect(ui->sliderParam2, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
	connect(ui->sliderParam3, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));

	connect(ui->sliderParam1, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int)));
	connect(ui->sliderParam2, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int)));
	connect(ui->sliderParam3, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int)));

	connect(ui->sliderParam1, SIGNAL(valueChanged(int)), this, SLOT(sliderParam1Changed(int)));
	connect(ui->sliderParam2, SIGNAL(valueChanged(int)), this, SLOT(sliderParam2Changed(int)));
	connect(ui->sliderParam3, SIGNAL(valueChanged(int)), this, SLOT(sliderParam3Changed(int)));

	connect(ui->doubleSpinBoxParam1, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBoxParam1Changed(double)));
	connect(ui->doubleSpinBoxParam2, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBoxParam2Changed(double)));
	connect(ui->doubleSpinBoxParam3, SIGNAL(valueChanged(double)), this, SLOT(doubleSpinBoxParam3Changed(double)));

	//QTimer::singleShot(0, this, SLOT(loadCurrentSlice()));
	//loadCurrentSlice(); // this should resize the images, but for some reason it doesn't work
}

CPreviewDialog::~CPreviewDialog()
{
	for (int i = 0; i < m_filterTypes.size(); ++i)
	{
		delete m_filterTypes[i];
	}

    delete ui;
}

void CPreviewDialog::setParamsProperties()
{
	// loads parameters from object with parameters and sets ui elements based on them

	double paramDecimalCnt = 0;
	double paramStep = 0;
	double decimalCntPow = 0;
	bool hidden = true;
	double paramMin = 0;
	double paramMax = 0;
	double paramVal = 0;

	for (int i = 0; i < m_paramSliders.size(); ++i)
	{
		// get parametr properties
		paramDecimalCnt = m_paramsDesc->getParamDecimalCnt(i);
		paramStep = m_paramsDesc->getParamStep(i);
		decimalCntPow = pow(10.0, static_cast<int>(paramDecimalCnt));
		hidden = !m_paramsDesc->isParamVisible(i);
		paramMin = m_paramsDesc->getParamMin(i);
		paramMax = m_paramsDesc->getParamMax(i);
		paramVal = m_paramsDesc->getParam(i);

		m_paramSliders[i]->blockSignals(true);
		m_paramSpinBoxes[i]->blockSignals(true);

		m_paramLabels[i]->setHidden(hidden);
		m_paramLabels[i]->setText(m_paramsDesc->getParamName(i));

		m_paramSliders[i]->setHidden(hidden);
		m_paramSliders[i]->setMinimum(paramMin);

		if (paramStep < 2)
		{
			m_paramSliders[i]->setMaximum(paramMax * decimalCntPow);
			m_paramSliders[i]->setValue(paramVal * decimalCntPow);
		}
		else
		{
			// if parameter step is bigger than 1, the maximum of the slider and the value should be recalculated
			m_paramSliders[i]->setMaximum(paramMax / paramStep + 1);
			m_paramSliders[i]->setValue(paramVal / paramStep + 1);
		}

		m_paramSpinBoxes[i]->setHidden(hidden);
		m_paramSpinBoxes[i]->setSingleStep(paramStep);
		m_paramSpinBoxes[i]->setDecimals(paramDecimalCnt);
		m_paramSpinBoxes[i]->setMinimum(paramMin);
		m_paramSpinBoxes[i]->setMaximum(paramMax);
		m_paramSpinBoxes[i]->setValue(paramVal);

		m_paramSliders[i]->blockSignals(false);
		m_paramSpinBoxes[i]->blockSignals(false);
	}
}

void CPreviewDialog::loadCurrentSlice()
{
	m_loadSlice = false;

	// original preview
	QImage original;

	// apply filter on slice
	QImage img;

	// apply current filter on data and fill these images
	m_data.preview(original, img, m_currentSlice);

	// need to resize the images based on the aspect ration of them
	double rImg = (double)img.width() / img.height();
	double rWidget = (double)ui->groupBoxFiltered->width() / ui->groupBoxFiltered->height();


	// set filtered preview
	QPixmap pm(QPixmap::fromImage((rWidget > rImg) ? img.scaledToHeight(ui->groupBoxFiltered->height() - 45, Qt::SmoothTransformation) : img.scaledToWidth(ui->groupBoxFiltered->width() - 18, Qt::SmoothTransformation)));
	ui->previewFiltered->setPixmap(pm);
	ui->previewFiltered->setFixedSize(QSize(pm.width(), pm.height()));

	// set original preview
	QPixmap pmOriginal(QPixmap::fromImage((rWidget > rImg) ? original.scaledToHeight(ui->groupBoxFiltered->height() - 45, Qt::SmoothTransformation) : original.scaledToWidth(ui->groupBoxFiltered->width() - 18, Qt::SmoothTransformation)));	
	ui->previewOriginal->setPixmap(pmOriginal);
	ui->previewOriginal->setFixedSize(QSize(pmOriginal.width(), pmOriginal.height()));
}

void CPreviewDialog::sliceChanged(int slice)
{
	// set current slice
	m_currentSlice = slice;

	// if slice should be loaded, do it
	if (m_loadSlice)
		loadCurrentSlice();
}

void CPreviewDialog::resizeEvent(QResizeEvent *e)
{
	QDialog::resizeEvent(e);

	loadCurrentSlice();
}

void CPreviewDialog::sliderParam1Changed(int val)
{
	double decimalCntPow = pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(0)));

	// recalculate the value
	double v = val / decimalCntPow;

	// check, if this parameter is restricted or if it is a boundary for some other parameter
	if (m_paramsDesc->isSomeParamRestricted())
	{
		// this parameter is restricted
		if (m_paramsDesc->getRestrictedParamIndex() == 0)
		{
			double boundVal = m_paramsDesc->getParam(m_paramsDesc->getBoundaryParamIndex());

			if (v > boundVal)
			{
				v = boundVal;
			}
		}
		// this is boundary for another parameter
		else if (m_paramsDesc->getBoundaryParamIndex() == 0)
		{
			double restrictedVal = m_paramsDesc->getParam(m_paramsDesc->getRestrictedParamIndex());

			if (v < restrictedVal)
			{
				v = restrictedVal;
			}
		}
	}

	// set spin box value
	ui->doubleSpinBoxParam1->blockSignals(true);
	ui->sliderParam1->blockSignals(true);

	ui->sliderParam1->setValue(v * decimalCntPow);
	ui->doubleSpinBoxParam1->setValue((m_paramsDesc->getParamStep(0) < 2) ? v : v * m_paramsDesc->getParamStep(0) - 1);

	ui->doubleSpinBoxParam1->blockSignals(false);
	ui->sliderParam1->blockSignals(false);

	// set the new value in the filter parameters
	m_paramsDesc->setParam(0, ui->doubleSpinBoxParam1->value());

	// if slice should be loaded, do it
	if (m_loadSlice)
		loadCurrentSlice();
}

void CPreviewDialog::sliderParam2Changed(int val)
{
	double decimalCntPow = pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(1)));

	// recalculate the value
	double v = (double)val / decimalCntPow;

	// check, if this parameter is restricted or if it is a boundary for some other parameter
	if (m_paramsDesc->isSomeParamRestricted())
	{
		// this parameter is restricted
		if (m_paramsDesc->getRestrictedParamIndex() == 1)
		{
			double boundVal = m_paramsDesc->getParam(m_paramsDesc->getBoundaryParamIndex());

			if (v > boundVal)
			{
				v = boundVal;
			}
		}
		// this is boundary for another parameter
		else if (m_paramsDesc->getBoundaryParamIndex() == 1)
		{
			double restrictedVal = m_paramsDesc->getParam(m_paramsDesc->getRestrictedParamIndex());

			if (v < restrictedVal)
			{
				v = restrictedVal;
			}
		}
	}

	// set spin box value
	ui->doubleSpinBoxParam2->blockSignals(true);
	ui->sliderParam2->blockSignals(true);

	ui->sliderParam2->setValue(v * decimalCntPow);
	ui->doubleSpinBoxParam2->setValue((m_paramsDesc->getParamStep(1) < 2) ? v : v * m_paramsDesc->getParamStep(1) - 1);

	ui->doubleSpinBoxParam2->blockSignals(false);
	ui->sliderParam2->blockSignals(false);

	m_paramsDesc->setParam(1, ui->doubleSpinBoxParam2->value());
	
	// if slice should be loaded, do it
	if (m_loadSlice)
		loadCurrentSlice();
}

void CPreviewDialog::sliderParam3Changed(int val)
{
	double decimalCntPow = pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(2)));

	// recalculate the value
	double v = (double)val / decimalCntPow;

	// check, if this parameter is restricted or if it is a boundary for some other parameter
	if (m_paramsDesc->isSomeParamRestricted())
	{
		// this parameter is restricted
		if (m_paramsDesc->getRestrictedParamIndex() == 2)
		{
			double boundVal = m_paramsDesc->getParam(m_paramsDesc->getBoundaryParamIndex());

			if (v > boundVal)
			{
				v = boundVal;
			}
		}
		// this is boundary for another parameter
		else if (m_paramsDesc->getBoundaryParamIndex() == 2)
		{
			double restrictedVal = m_paramsDesc->getParam(m_paramsDesc->getRestrictedParamIndex());

			if (v < restrictedVal)
			{
				v = restrictedVal;
			}
		}
	}

	// set spin box value
	ui->doubleSpinBoxParam3->blockSignals(true);
	//ui->sliderParam3->blockSignals(true);

	//ui->sliderParam3->setValue(v * decimalCntPow);
	ui->doubleSpinBoxParam3->setValue((m_paramsDesc->getParamStep(2) < 2) ? v : v * m_paramsDesc->getParamStep(2) - 1);

	ui->doubleSpinBoxParam3->blockSignals(false);
	//ui->sliderParam3->blockSignals(false);

	m_paramsDesc->setParam(2, ui->doubleSpinBoxParam3->value());
	
	// if slice should be loaded, do it
	if (m_loadSlice)
		loadCurrentSlice();
}

void CPreviewDialog::doubleSpinBoxParam1Changed(double val)
{
	// set slider value and load the slice
	ui->sliderParam1->setValue((int)(val * pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(0))) / ((m_paramsDesc->getParamStep(0) < 1) ? 1 : m_paramsDesc->getParamStep(0))));
	loadCurrentSlice();
}

void CPreviewDialog::doubleSpinBoxParam2Changed(double val)
{
	// set slider value and load the slice
	ui->sliderParam2->setValue((int)(val * pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(1))) / ((m_paramsDesc->getParamStep(1) < 1) ? 1 : m_paramsDesc->getParamStep(1))));
	loadCurrentSlice();
}

void CPreviewDialog::doubleSpinBoxParam3Changed(double val)
{
	// set slider value and load the slice
	ui->sliderParam3->setValue((int)(val * pow(10.0, static_cast<int>(m_paramsDesc->getParamDecimalCnt(2))) / ((m_paramsDesc->getParamStep(2) < 1) ? 1 : m_paramsDesc->getParamStep(2))));
	loadCurrentSlice();
}

void CPreviewDialog::filterTypeChanged(bool val)
{
	// get the index of selected filter

	if (!val)
		return;

	QRadioButton *rb = qobject_cast<QRadioButton *>(QObject::sender());

	int i = 0;

	for (i; i < m_filterTypes.size(); ++i)
	{
		if (rb == m_filterTypes[i])
		{
			// call method in data
			m_data.filterChanged(i);
			break;
		}
	}

	// set current filter index
	m_currentFilterIndex = i;

	// get new parameters description (which belong to the selected filter)
	m_paramsDesc = m_data.getParamDescription(m_currentFilterIndex);

	// set ui based on this parameters description
	setParamsProperties();

	// load slice
	loadCurrentSlice();
}

void CPreviewDialog::sliderReleased()
{
	// when slider is released, load slice
	m_sliderPressed = false;
	loadCurrentSlice();
}

void CPreviewDialog::sliderPressed()
{
	// when slider is pressed, the filtration will not be recalculated until the slider is released
	// (otherwise it filters the slice after each change of some slider, which can be really slow)
	m_sliderPressed = true;
}

void CPreviewDialog::sliderActionTriggered(int action)
{
	// slider is pressed, just return (loadSlice is set to false, so filtration is not done)
	if (m_sliderPressed)
		return;

	// on some actions, slice should be loaded
	if (action == QAbstractSlider::SliderSingleStepAdd || action == QAbstractSlider::SliderSingleStepSub || action == QAbstractSlider::SliderPageStepAdd || action == QAbstractSlider::SliderPageStepSub || action == QAbstractSlider::SliderMove)
	{
		m_loadSlice = true;
	}
}