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

#include "modelswidget.h"
#include "ui_modelswidget.h"
#include <data/CModelManager.h>
#include "qtcompat.h"
#include <app/Signals.h>

#include <QPushButton>
#include <QColorDialog>
#include <QStyleFactory>

#include <data/CActiveDataSet.h>



////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Constructor.
//!
//!\param [in,out]  parent  If non-null, the parent window.
////////////////////////////////////////////////////////////////////////////////////////////////////
CModelsWidget::CModelsWidget(QWidget *parent /*= 0*/) 
    : QWidget(parent)
    , ui(new Ui::ModelsWidget)
    , m_bMyChange(false)
{
    ui->setupUi(this);

    // Create table
    ui->tableModels->clearContents();
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_NAME, QHeaderView::Stretch);
    ui->tableModels->blockSignals(true);
    ui->tableModels->clearContents();
    ui->tableModels->setColumnCount(COL_COUNT);
    ui->tableModels->setColumnWidth(COL_VISIBLE, 45);
    ui->tableModels->setColumnWidth(COL_COLOR, 40);
    ui->tableModels->setColumnWidth(COL_DELETE, 30);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_NAME, QHeaderView::Stretch);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_VISIBLE, QHeaderView::Fixed);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_COLOR, QHeaderView::Fixed);    
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_DELETE, QHeaderView::ResizeToContents); 

    QPalette tablePalette;
    tablePalette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(150, 150, 150));
    tablePalette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(255, 255, 255));
    ui->tableModels->setPalette(tablePalette);

    // Set selection mode
    ui->tableModels->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableModels->setSelectionMode(QAbstractItemView::SingleSelection);

    // Connect item changed signal
    QObject::connect(ui->tableModels, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(onModelItemChanged(QTableWidgetItem *)));

    // Connect to all models
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        APP_STORAGE.connect(data::Storage::ImportedModel::Id + i, this);

    updateTable();
    ui->tableModels->selectRow(0);

    m_Connection = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CModelsWidget::onNewDensityData);
}


//!\brief   Destructor.
CModelsWidget::~CModelsWidget()
{
    APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_Connection);

    // Connect to all models
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        APP_STORAGE.disconnect(data::Storage::ImportedModel::Id + i, this);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the new density data action.
//!
//!\param [in,out]  pEntry  If non-null, the entry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelsWidget::onNewDensityData(data::CStorageEntry *pEntry)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Object changed.
//!
//!\param [in,out]  pData   If non-null, the data.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelsWidget::objectChanged(data::CModel *pData)
{
    if(!m_bMyChange)
        updateTable();

    m_bMyChange = false;
}


//!\brief   Updates a table.
void CModelsWidget::updateTable()
{
    ui->tableModels->blockSignals(true);
    ui->tableModels->setUpdatesEnabled(false);

    {
        ui->tableModels->clearContents();
        ui->tableModels->setRowCount(0);

        // For all models
        for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        {
            int storage_id(i + data::Storage::ImportedModel::Id);

            // Get model
            data::CObjectPtr<data::CModel> pModel(APP_STORAGE.getEntry(storage_id));

            // Is model valid?
            if(!pModel->hasData())
                continue;

            // Resize table - add row
            ui->tableModels->setRowCount(ui->tableModels->rowCount() + 1);

            // Label
            const std::string &std_name(pModel->getName());
            QString qs_name(QString::fromStdString(std_name));
            QTableWidgetItem *ti_name(new QTableWidgetItem(qs_name));
            ti_name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

            // Visibility checkable button
            QTableWidgetItem *ti_visibility = new QTableWidgetItem();
			ti_visibility->setCheckState(VPL_SIGNAL(SigGetModelVisibility).invoke2(storage_id)?Qt::Checked:Qt::Unchecked);
            ti_visibility->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);

            // Color
            QPushButton *ti_color = new QPushButton(ui->tableModels);
            ti_color->setStyle(QStyleFactory::create("windows"));
            ti_color->setProperty("StorageID",storage_id);
            ti_color->setObjectName("ColorButton");
            QObject::connect(ti_color,SIGNAL(clicked()),this, SLOT(onModelColorButton()));

            const data::CColor4f& color = pModel->getColor();
            QColor qcolor;
            qcolor.setRgbF(color.getR(), color.getG(), color.getB(), color.getA());
            QString str = "* { background-color: " + qcolor.name() + " }";
            ti_color->setStyleSheet(str);            
            ti_color->setProperty("Color",qcolor.rgba());

            // Remove button
            QPushButton* ti_remove = new QPushButton();
            ti_remove->setProperty("StorageID",storage_id);
            ti_remove->setIcon(QIcon(":/icons/delete.png"));
            ti_remove->setFlat(true);
            ti_remove->setObjectName("DeleteButton");
            QObject::connect(ti_remove, SIGNAL(clicked()), this, SLOT(onModelRemoveButton()));

            // Modify table row
            int row_id( ui->tableModels->rowCount() - 1);
            ui->tableModels->setItem(row_id, COL_NAME, ti_name);
            ui->tableModels->setItem(row_id, COL_VISIBLE, ti_visibility);
            ui->tableModels->setCellWidget(row_id, COL_COLOR, ti_color);
            ui->tableModels->setCellWidget(row_id, COL_DELETE, ti_remove);
        }

    }

    ui->tableModels->setUpdatesEnabled(true);
    ui->tableModels->blockSignals(false);
}


//!\brief   Executes the model color button action.
void CModelsWidget::onModelColorButton()
{
    m_bMyChange = true;

    // Get button
    QPushButton *pb(qobject_cast<QPushButton *>(sender()));

    if(pb == 0)
        return;

    // Try to get property
    int index(pb->property("StorageID").toInt());
    if(index < data::Storage::ImportedModel::Id || index >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
        return;

    // Show dialog
    QColorDialog cdlg(this);
    QColor selectedColor = cdlg.getColor(QColor::fromRgba(pb->property("Color").toUInt()), this, tr("Select color for model"), QColorDialog::ShowAlphaChannel);
    if (!selectedColor.isValid())
    {
        // No color, no change
        return;
    }

    // Change button color property to the selected color
    pb->setProperty("Color", selectedColor.rgba());
    QString str = "* { background-color: " + selectedColor.name() + " }";
    pb->setStyleSheet(str);            

    // Change model color
    data::CColor4f colModel; // Our color model
    colModel.setColor(selectedColor.redF(),selectedColor.greenF(),selectedColor.blueF(), selectedColor.alphaF());
	VPL_SIGNAL(SigSetModelColor).invoke(index,colModel);
}


//!\brief   Executes the model remove button action.
void CModelsWidget::onModelRemoveButton()
{
    // Get button
    QPushButton *pb(qobject_cast<QPushButton *>(sender()));

    if(pb == 0)
        return;

    // Try to get property
    int index(pb->property("StorageID").toInt());
    if(index < data::Storage::ImportedModel::Id || index >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
        return;

    // Get model
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(index));

    // Reset model
    spModel->init();
    APP_STORAGE.invalidate(spModel.getEntryPtr());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the model item changed action.
//!
//!\param [in,out]  item    If non-null, the item.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelsWidget::onModelItemChanged(QTableWidgetItem *item)
{
    // Get item row
    int item_row(item->row());

    // Try to get storage id
    QWidget *color_widget(ui->tableModels->cellWidget(item_row, COL_COLOR));
    if(color_widget == 0)
        return;

    int storage_id(color_widget->property("StorageID").toInt());
    if(storage_id < data::Storage::ImportedModel::Id || storage_id >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
        return;

    switch(item->column())
    {
    // Name changed
    case COL_NAME:
        {
            data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(storage_id) );
            spModel->setLabel(item->text().toStdString());
            m_bMyChange = true;
            APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::MESH_NOT_CHANGED);
        }
        break;

    // Visibility changed
    case COL_VISIBLE:
        {
            if (Qt::ItemIsEnabled == (item->flags() & Qt::ItemIsEnabled))
            {
                 data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(storage_id) );
                bool bWasVisible(spModel->isShown());
                bool bVisible = item->checkState() == Qt::Checked;
                if ( (bWasVisible && !bVisible) || (!bWasVisible && bVisible) )
                {
                    spModel->setVisibility(bVisible);
                    m_bMyChange = true;
                    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::MESH_NOT_CHANGED);
                }
            }
        }
        break;

    default:
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Gets the selected model storage identifier.
//!
//!\return  The selected model storage identifier.
////////////////////////////////////////////////////////////////////////////////////////////////////
int CModelsWidget::getSelectedModelStorageId()
{
    // Get selected index list
    QModelIndexList selectedList = ui->tableModels->selectionModel()->selectedRows();
    if(selectedList.size() == 0)
        return -1;

    // Get item row
    int item_row(selectedList.front().row());

    // Try to get storage id
    QWidget *color_widget(ui->tableModels->cellWidget(item_row, COL_COLOR));
    if(color_widget == 0)
        return -1;

    int storage_id(color_widget->property("StorageID").toInt());
    if(storage_id < data::Storage::ImportedModel::Id || storage_id >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
        return -1;

    return storage_id;
}
