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

#ifndef CPREVIEWDIALOG_H
#define CPREVIEWDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>

#include "CPreviewDialogData.h"

namespace Ui {
	class CPreviewDialog;
}

//! Volume filtering dialog with preview on single slice
class CPreviewDialog : public QDialog
{
    Q_OBJECT  

protected:
	//! UI object
	Ui::CPreviewDialog *ui;

	//! Object with filters data
	CPreviewDialogData& m_data;

	//! Vector of filters names
	std::vector<QString>& m_filterNames;

	//! Object with data, which describes current filter's parameters
	CPreviewDialogParametersDescription* m_paramsDesc;

	//! Index of current slice
	int m_currentSlice;

	//! Index of current filter
	int m_currentFilterIndex;

	//! Vectors of widgets in ui
	std::vector<QRadioButton *> m_filterTypes;
	std::vector<QLabel *> m_paramLabels;
	std::vector<QSlider *> m_paramSliders;
	std::vector<QDoubleSpinBox *> m_paramSpinBoxes;

	// ! Flag indicating that slice can be filtered
	bool m_loadSlice;

	// ! Flag indicating that some slider has been pressed
	bool m_sliderPressed;

public:
	//! Constructor
	explicit CPreviewDialog(QWidget *parent, const QString& title, CPreviewDialogData& data);

	//! Destructor
	~CPreviewDialog();

protected:   
	//! Calls preview method from data and shows original and filtered slice
	void loadCurrentSlice();

	virtual void resizeEvent(QResizeEvent *);

	//! Sets ui based on current filter parameters
	void setParamsProperties();

protected slots:
	// ! Called, when slice changed 
	void sliceChanged(int);

	// ! Called, when value of slider for parameter 1 changed
	void sliderParam1Changed(int);

	// ! Called, when value of slider for parameter 2 changed
	void sliderParam2Changed(int);

	// ! Called, when value of slider for parameter 3 changed
	void sliderParam3Changed(int);

	// ! Called, when value of spin box for parameter 1 changed
	void doubleSpinBoxParam1Changed(double);

	// ! Called, when value of spin box for parameter 2 changed
	void doubleSpinBoxParam2Changed(double);

	// ! Called, when value of spin box for parameter 3 changed
	void doubleSpinBoxParam3Changed(double);

	// ! Called, when filter is switched
	void filterTypeChanged(bool);

	// ! Called, when some slider is release
	void sliderReleased();

	// ! Called, when some slider is pressed
	void sliderPressed();

	// ! Called on some slider actions (keyUp, keyDown, pageUp, ...)
	void sliderActionTriggered(int);
};

#endif // CPREVIEWDIALOG_H
