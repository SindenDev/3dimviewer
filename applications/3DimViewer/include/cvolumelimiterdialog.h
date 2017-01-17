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

#ifndef CVOLUMELIMITERDIALOG_H
#define CVOLUMELIMITERDIALOG_H

//! Includes
#include <QDialog>
#include <data/CDensityData.h>
#include <data/COrthoSlice.h>

// OSG windows
#include <osg/OSGOrtho2DCanvas.h>
#include <osg/CLimiterSceneOSG.h>

namespace Ui {
class CVolumeLimiterDialog;
}

//! Dialog for volume limiting (crop)
class CVolumeLimiterDialog : public QDialog
{
    Q_OBJECT
    
public:
    //! Constructor
    explicit CVolumeLimiterDialog(QWidget *parent = 0);

    //! Destructor
    ~CVolumeLimiterDialog();

    //! Sets the density data.
    void    setVolume(data::CDensityData *pData, bool bInitLimits);

    //! Sets minimal and maximal coordinates of the volume of interest.
    void    setLimits(const data::SVolumeOfInterest& Limits);

    //! Returns minimal and maximal coordinates of the volume of interest.
    const data::SVolumeOfInterest& getLimits() const { return m_Limits; }
    
private:
    Ui::CVolumeLimiterDialog *ui;

protected:
    //! OSG widgets with slices
    OSGOrtho2DCanvas* m_OrthoXYSlice;
    OSGOrtho2DCanvas* m_OrthoXZSlice;
    OSGOrtho2DCanvas* m_OrthoYZSlice;

    //! Volume data
    data::CDensityData::tSmartPtr m_spVolume;

    //! OSG scenes
    osg::ref_ptr<scene::CLimiterXY> m_SceneXY;
    osg::ref_ptr<scene::CLimiterXZ> m_SceneXZ;
    osg::ref_ptr<scene::CLimiterYZ> m_SceneYZ;

    //! Volume of interest
    data::SVolumeOfInterest m_Limits;

	//! Original dimensions of volume
	vpl::img::CPoint3i m_volumeSize;
	vpl::img::CPoint3d m_voxelSize;

    //! Ortho slices.
    data::COrthoSliceXY m_SliceXY;
    data::COrthoSliceXZ m_SliceXZ;
    data::COrthoSliceYZ m_SliceYZ;

    //! Signal connections
    vpl::mod::tSignalConnection m_SigConnection[6];

    //! Inits controls with OSG scenes
    void    createControls();

    //! Update edits on drag in scene
    void    updateMinX(int iValue);
    void    updateMinY(int iValue);
    void    updateMinZ(int iValue);
    void    updateMaxX(int iValue);
    void    updateMaxY(int iValue);
    void    updateMaxZ(int iValue);

	//! Redraws all scenes
	void refresh();

private slots:
    void on_comboBoxMode_currentIndexChanged(int index);
    void on_buttonResetZoom_clicked();
};

#endif // CVOLUMELIMITERDIALOG_H
