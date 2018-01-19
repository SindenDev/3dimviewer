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

#ifndef modelswidget_H_included
#define modelswidget_H_included

#include <QWidget>

#include <data/CModel.h>
#include <data/CGeneralObjectObserver.h>
#include <QTableWidgetItem>

namespace Ui
{
    class ModelsWidget;
}

class CModelsWidget : public QWidget, public data::CGeneralObjectObserver<CModelsWidget>
{
    Q_OBJECT

    enum EColumns
    {
		COL_COLOR = 0,
        COL_NAME,
        COL_VISIBLE,
		COL_CUT,
        COL_DELETE,
		COL_INFO,
        COL_COUNT
    };

public:
    //! Constructor
    explicit CModelsWidget(QWidget *parent = 0);
    //! Destructor
    ~CModelsWidget();

    //! Called on volume data change.
    void onNewDensityData(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

    //! Return storage id of the selected model, -1 if no model is selected
    int getSelectedModelStorageId();

private:
    //! Method called on model change
    void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

    //! Update table
    void updateTable();

	//! Is region linked to the model?
	bool isRegionLinked(int id);

	//! Mirror model settings to the coloring
	void modelToRegion(int id, int what);

	//! Mirror all models to the coloring
	void modelsToRegions();

private slots:
    //! On model color button clicked
    void onModelColorButton();

    //! On model remove button clicked
    void onModelRemoveButton();

    //! On model item changed
    void onModelItemChanged(QTableWidgetItem *item);

	//! Checkbox for DICOM coordinates use
	void on_checkBoxIOApplyDICOM_clicked();

	//! Context menu handler for models table
	void tableModelsContextMenu(QPoint p);

	void showModelInfo();

	void on_pushButtonBuyPlugins_clicked();

	void on_pushButtonSaveModel_clicked();

private:
    // GUI
    Ui::ModelsWidget *ui;
    //! Was this my change?
    bool m_bMyChange;
	//! Are models linked to the regions?
	bool m_bModelsLinked;
};

// modelswidget_H_included
#endif