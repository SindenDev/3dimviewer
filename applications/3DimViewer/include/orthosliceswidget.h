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

#ifndef ORTHOSLICESWIDGET_H
#define ORTHOSLICESWIDGET_H

#include <QWidget>
#include <VPL/Module/Signal.h>
#include <data/CDataStorage.h>

namespace Ui {
class OrthoSlicesWidget;
}

//! Panel with ortho slices settings (positions)
class COrthoSlicesWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit COrthoSlicesWidget(QWidget *parent = 0);
    ~COrthoSlicesWidget();
    
private slots:
    void on_xySliceSlider_valueChanged(int value);

    void on_xzSliceSlider_valueChanged(int value);

    void on_yzSliceSlider_valueChanged(int value);

    void on_xySliceModeCombo_currentIndexChanged(int index);

    void on_xzSliceModeCombo_currentIndexChanged(int index);

    void on_yzSliceModeCombo_currentIndexChanged(int index);

private:
    Ui::OrthoSlicesWidget *ui;

    // Signal connections.
    vpl::mod::tSignalConnection m_Connection;
    vpl::mod::tSignalConnection m_ConnectionXY, m_ConnectionXZ, m_ConnectionYZ;

    int m_LastXYPosition, m_LastXZPosition, m_LastYZPosition;
    bool m_bDontNotify;

    //! Called on volume data change.
    void onNewDensityData(data::CStorageEntry *pEntry);

    //! Called on slice change.
    void onNewSliceXY(data::CStorageEntry *pEntry);
    void onNewSliceXZ(data::CStorageEntry *pEntry);
    void onNewSliceYZ(data::CStorageEntry *pEntry);
};

#endif // ORTHOSLICESWIDGET_H
