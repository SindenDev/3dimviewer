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

#ifndef VOLUMERENDERINGWIDGET_H
#define VOLUMERENDERINGWIDGET_H

#include <QWidget>
#include <render/PSVRrenderer.h>
#include <osg/Vec4>

namespace Ui {
class VolumeRenderingWidget;
}

namespace PSVR { class PSVolumeRendering; }

class QGroupBox;

//! Panel with volume rendering settings
class CVolumeRenderingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CVolumeRenderingWidget(QWidget *parent = 0);
    ~CVolumeRenderingWidget();
    
    //! Sets the volume renderer controlled by this panel.
    void setRenderer(PSVR::PSVolumeRendering *pRenderer);

    //! Set background color
    void setBackgroundColor( const osg::Vec4 & color );

	void setVRMode();

protected:
    virtual void 	showEvent ( QShowEvent * event );

private slots:
    void on_radioButtonMIP_toggled(bool checked);

    void on_radioButtonShaded_toggled(bool checked);

    void on_radioButtonXRay_toggled(bool checked);

    void on_radioButtonSurface_toggled(bool checked);

    void on_radioButtonCustom_toggled(bool checked);

    void on_comboBoxColoring_currentIndexChanged(int index);

    void on_sliderRenderingQuality_valueChanged(int value);

    void on_sliderCuttingPlane_valueChanged(int value);

    void on_sliderWindShift_valueChanged(int value);

    void on_sliderWindWidth_valueChanged(int value);

    void on_sliderBrightness_valueChanged(int value);

    void on_sliderContrast_valueChanged(int value);

    void on_sliderSurfTolerance_valueChanged(int value);

    void on_sliderSurfSharpness_valueChanged(int value);

	void packGroupBox(bool checked);
	void packGroupBox(QGroupBox* pWidget, bool checked);
private:
    Ui::VolumeRenderingWidget *ui;

    //! Volume renderer
    PSVR::PSVolumeRendering *m_pRenderer;

protected:
    enum LUT
    {
        Default=0,
        SoftTissues,
        BoneTissues,
        Transparent,
        AirPockets,
        Skull,
        Spine,
        Pelvis,
        EnhanceSoft,
        EnhanceHard,
        BoneSurface,
        SkinSurface
    };

    void vrDataRemapChange(float expand, float offset);
    PSVR::PSVolumeRendering::ELookups lutCodeToVrLut(int index);
    int vrLutToLutCode(PSVR::PSVolumeRendering::ELookups index);

	void setCustomVR();
};

#endif // VOLUMERENDERINGWIDGET_H
