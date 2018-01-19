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

#ifndef CSERIESSELECTIONDIALOG_H
#define CSERIESSELECTIONDIALOG_H

#include <QDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include <data/CSeries.h>
#include <QTableWidget>
#include <QLabel>

namespace Ui {
class CSeriesSelectionDialog;
}

class CSeriesSelectionDialogPreview : public QDialog
{
	Q_OBJECT

public:
	//! Constructor
	explicit CSeriesSelectionDialogPreview(QWidget *parent = 0, Qt::WindowFlags f = 0);

	//! Destructor
	~CSeriesSelectionDialogPreview(){};

	void setSerie(data::CSerieInfo::tSmartPtr serie);

private:
	data::CSerieInfo::tSmartPtr m_serie;
	QLabel *m_slicePreview;
	QSlider *m_slider;
	QImage *m_bitmap;
	double m_rSlice;

	bool m_dirty;

	void resizeImage();

private slots:
	void sliceChanged(int val);

protected:
	virtual void resizeEvent(QResizeEvent *);
};

//! Dialog for DICOM series selection
class CSeriesSelectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    //! Constructor
    explicit CSeriesSelectionDialog(QWidget *parent = 0, Qt::WindowFlags f=0);

    //! Destructor
    ~CSeriesSelectionDialog();

    //! Sets info about preloaded series and prepares content of the dialog.
    bool setSeries(data::CSeries *pSeries);

    //! Returns index of the choosen series.
    int getSelection() const;

    //! Returns subsampling factors
    void getSubsampling(double &subsamplingX, double &subsamplingY, double &subsamplingZ) const;

	bool eventFilter(QObject *obj, QEvent *event);

protected:
    //! Pointer to the series data.
    data::CSeries::tSmartPtr    m_spSeries;
    //! Index of selected series
    int                         m_Selection;

private slots:
    virtual void accept();
    void on_tableWidget_cellDoubleClicked(int row, int column);
	void on_tableWidget_cellClicked(int row, int column);
    void on_spinSubsampling_valueChanged(double value);

private:
    Ui::CSeriesSelectionDialog *ui;
	CSeriesSelectionDialogPreview *m_preview;

	void addInfoRow(const QString &info, bool bStudyID = false);

	void showPreview();
};

#endif // CSERIESSELECTIONDIALOG_H
