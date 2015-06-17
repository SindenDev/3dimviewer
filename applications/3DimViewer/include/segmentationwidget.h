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

#ifndef SEGMENTATIONWIDGET_H
#define SEGMENTATIONWIDGET_H

#include <QWidget>

namespace Ui {
class SegmentationWidget;
}

//! Panel with segmentation settings
class CSegmentationWidget : public QWidget
{
    Q_OBJECT
    
public:
    //! Constructor
    explicit CSegmentationWidget(QWidget *parent = 0);
    //! Destructor
    ~CSegmentationWidget();
    //! Gets lower threshold
    int     getLo();
    //! Gets higher threshold
    int     getHi();
private slots:
    void on_buttonColor_clicked();

    void on_checkBoxApplyThresholds_clicked();

    void on_highThresholdSlider_valueChanged(int value);

    void on_lowThresholdSlider_valueChanged(int value);

    void on_buttonPickLo_clicked();

    void on_buttonPickHi_clicked();

	void on_pushButtonCreateSurfaceModel_clicked();

	void on_pushButtonSetToActiveRegion_clicked();

	void firstEvent();
private:
    Ui::SegmentationWidget *ui;

    QColor  m_color;

    //! Sets coloring
    void    SetColoring();
    //! Sets the lower threshold to the density value at specified point.
    void    setLowerThreshold(float x, float y, float z, int EventType);
    //! Sets the higher threshold to the density value at specified point.
    void    setHigherThreshold(float x, float y, float z, int EventType);
};

#endif // SEGMENTATIONWIDGET_H
