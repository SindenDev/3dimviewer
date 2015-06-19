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

#ifndef CFILTERDIALOG_H
#define CFILTERDIALOG_H

#include <QDialog>

namespace Ui {
class CFilterDialog;
}

typedef void (*fnProcessSlice)(int z, int strength, QImage& result);

//! Volume filtering dialog with preview on single slice
class CFilterDialog : public QDialog
{
    Q_OBJECT    
protected:
	//! UI object
	Ui::CFilterDialog *ui;
	//! Current xy slice
	int	m_curSlice;					
	//! Filter strength (0-100)
	int	m_strength;					
	//! Slice filtering method
	fnProcessSlice m_pSliceFilter;	
public:
	//! Constructor
    explicit CFilterDialog(QWidget *parent, const QString& title);
	//! Destructor
	~CFilterDialog();
	//! Set method for slice filter
	void setFilter(fnProcessSlice pSliceFilter) { m_pSliceFilter = pSliceFilter; } // mandatory
	//! Filter strength getter
	int	 getStrength() const { return m_strength; }    
protected:   
			void	loadCurrentSlice();
	virtual void	resizeEvent(QResizeEvent *);
protected slots:
	void sliceChanged(int);
	void sliderStrengthChanged(int);
};

#endif // CFILTERDIALOG_H
