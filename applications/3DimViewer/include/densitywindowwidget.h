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

#ifndef DENSITYWINDOWWIDGET_H
#define DENSITYWINDOWWIDGET_H

#include <QWidget>

#include <VPL/Module/Signal.h>

#include <data/CGeneralObjectObserver.h>
#include <data/CDensityWindow.h>

namespace Ui {
class DensityWindowWidget;
}

//! Panel with density window settings
class CDensityWindowWidget : public QWidget, public data::CGeneralObjectObserver<CDensityWindowWidget>
{
    Q_OBJECT
    
public:
    //! Constructor
    explicit CDensityWindowWidget(QWidget *parent = 0);
    //! Destructor
    ~CDensityWindowWidget();
    
    void objectChanged(data::CStorageEntry *,const data::CChangedEntries &);

    //! Method called on any density data change.
    void onNewDensityWindow(data::CStorageEntry *,const data::CChangedEntries &);

    //! Called on volume data change.
    void onNewDensityData(data::CStorageEntry *,const data::CChangedEntries &);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_densityWindowCenter_valueChanged(int arg1);
    void on_densityWindowWidth_valueChanged(int arg1);
    void on_pushButton_2_clicked();
private:
    Ui::DensityWindowWidget *ui;
    bool                     m_bDontNotify;
};

#endif // DENSITYWINDOWWIDGET_H
