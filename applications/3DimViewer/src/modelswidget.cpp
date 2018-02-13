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

#include "modelswidget.h"
#include "ui_modelswidget.h"
#include <data/CModelManager.h>
#include "qtcompat.h"
#include <coremedi/app/Signals.h>
#include <Signals.h>
#include <data/CDensityData.h>
#include <mainwindow.h>

#include <QPushButton>
#include <QColorDialog>
#include <QStyleFactory>
#include <QSettings>
#include <QMenu>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QLabel>

#include <data/CActiveDataSet.h>

#include <cinfodialog.h>

#include <QDesktopServices>
#include <QUrl>


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

	// Set model-region link status
	QSettings settings;
	m_bModelsLinked = settings.value("ModelRegionLinkEnabled", QVariant(true)).toBool();

    // Create table
    ui->tableModels->clearContents();
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_NAME, QHeaderView::Stretch);
    ui->tableModels->blockSignals(true);
    ui->tableModels->clearContents();
    ui->tableModels->setColumnCount(COL_COUNT);
    ui->tableModels->setColumnWidth(COL_VISIBLE, 45);
	ui->tableModels->setColumnWidth(COL_CUT, 30);	
    ui->tableModels->setColumnWidth(COL_COLOR, 40);
    ui->tableModels->setColumnWidth(COL_DELETE, 30);
	ui->tableModels->setColumnWidth(COL_INFO, 30);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_NAME, QHeaderView::Stretch);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_VISIBLE, QHeaderView::ResizeToContents);
	ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_CUT, QHeaderView::ResizeToContents);
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_COLOR, QHeaderView::ResizeToContents);    
    ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_DELETE, QHeaderView::ResizeToContents); 
	ui->tableModels->horizontalHeader()->SETRESIZEMODE(COL_INFO, QHeaderView::ResizeToContents);
	ui->tableModels->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(ui->tableModels, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableModelsContextMenu(QPoint)));

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
        data::CGeneralObjectObserver<CModelsWidget>::connect(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i ).get());

    updateTable();
    ui->tableModels->selectRow(0);

	ui->checkBoxIOApplyDICOM->setChecked(settings.value("STLUseDICOMCoord",false).toBool());

    CGeneralObjectObserver<CModelsWidget>::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id ).get(), CGeneralObjectObserver<CModelsWidget>::tObserverHandler(this, &CModelsWidget::onNewDensityData));
}


//!\brief   Destructor.
CModelsWidget::~CModelsWidget()
{
    CGeneralObjectObserver<CModelsWidget>::disconnect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id ).get());

    // Connect to all models
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        CGeneralObjectObserver<CModelsWidget>::disconnect(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i).get());
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the new density data action.
//!
//!\param [in,out]  pEntry  If non-null, the entry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelsWidget::onNewDensityData(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Object changed.
//!
//!\param [in,out]  pData   If non-null, the data.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelsWidget::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
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
            const std::string &std_name(pModel->getLabel());
            QString qs_name(QString::fromStdString(std_name));
            QTableWidgetItem *ti_name(new QTableWidgetItem(qs_name));
            ti_name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

            // Visibility checkable button
            QTableWidgetItem *ti_visibility = new QTableWidgetItem();
			ti_visibility->setCheckState(VPL_SIGNAL(SigGetModelVisibility).invoke2(storage_id)?Qt::Checked:Qt::Unchecked);
            ti_visibility->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);

			// Cut checkable button
			QTableWidgetItem *ti_cut = new QTableWidgetItem();
			ti_cut->setCheckState(VPL_SIGNAL(SigGetModelCutVisibility).invoke2(storage_id)?Qt::Checked:Qt::Unchecked);
			ti_cut->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);

            // Color
            QPushButton *ti_color = new QPushButton(ui->tableModels);
            ti_color->setStyle(QStyleFactory::create("windows"));
            ti_color->setProperty("StorageID",storage_id);
            ti_color->setObjectName("ColorButton");
            QObject::connect(ti_color,SIGNAL(clicked()),this, SLOT(onModelColorButton()));

            const data::CColor4f& color = pModel->getColor();
            QColor qcolor;
            qcolor.setRgbF(color.getR(), color.getG(), color.getB(), color.getA());
            QString str = "QPushButton { background-color: " + qcolor.name() + " } QToolTip { background-color: white }";
            ti_color->setStyleSheet(str);            
            ti_color->setProperty("Color",qcolor.rgba());

			ti_color->setToolTip(tr("Click here to change model color"));

            // Remove button
            QPushButton* ti_remove = new QPushButton();
            ti_remove->setProperty("StorageID",storage_id);
            ti_remove->setIcon(QIcon(":/icons/delete.png"));
            ti_remove->setFlat(true);
            ti_remove->setObjectName("DeleteButton");
			ti_remove->setToolTip(tr("Remove model"));
            QObject::connect(ti_remove, SIGNAL(clicked()), this, SLOT(onModelRemoveButton()));

			// Info button
			QPushButton* ti_info= new QPushButton();
			ti_info->setProperty("StorageID", storage_id);
			ti_info->setIcon(QIcon(":/icons/information.png"));
			ti_info->setFlat(true);
			ti_info->setObjectName("InfoButton");
			ti_info->setToolTip(tr("Show information about model"));
			QObject::connect(ti_info, SIGNAL(clicked()), this, SLOT(showModelInfo()));

            // Modify table row
            int row_id( ui->tableModels->rowCount() - 1);
            ui->tableModels->setItem(row_id, COL_NAME, ti_name);
            ui->tableModels->setItem(row_id, COL_VISIBLE, ti_visibility);
			ui->tableModels->setItem(row_id, COL_CUT, ti_cut);
            ui->tableModels->setCellWidget(row_id, COL_COLOR, ti_color);
            ui->tableModels->setCellWidget(row_id, COL_DELETE, ti_remove);
			ui->tableModels->setCellWidget(row_id, COL_INFO, ti_info);

			geometry::CMesh *pMesh = pModel->getMesh();

			QString toolTip = tr("Triangles: ") + QString::number(pMesh->n_faces()) + tr(", Nodes: ") + QString::number(pMesh->n_vertices()) + tr(", Components: ") + QString::number(pMesh->componentCount());

			ui->tableModels->item(row_id, COL_NAME)->setToolTip(toolTip);
			ui->tableModels->item(row_id, COL_VISIBLE)->setToolTip(toolTip);
			ui->tableModels->item(row_id, COL_CUT)->setToolTip(toolTip);
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
    QString str = "* { background-color: " + selectedColor.name() + " }  QToolTip { background-color: white }";
    pb->setStyleSheet(str);            

    // Change model color
    data::CColor4f colModel; // Our color model
    colModel.setColor(selectedColor.redF(),selectedColor.greenF(),selectedColor.blueF(), selectedColor.alphaF());
	VPL_SIGNAL(SigSetModelColor).invoke(index,colModel);

	modelToRegion(index-data::Storage::ImportedModel::Id, COL_COLOR);
}


//!\brief   Executes the model remove button action.
void CModelsWidget::onModelRemoveButton()
{
	if (QMessageBox::No == QMessageBox::question(NULL, QCoreApplication::applicationName(), tr("Do you really want to remove this model?"), QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

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
	spModel->createAndStoreSnapshot();
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
            APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::LABEL_CHANGED);

			modelToRegion(storage_id - data::Storage::ImportedModel::Id, COL_NAME);
        }
        break;

    // Visibility changed
    case COL_VISIBLE:
        {
            if (Qt::ItemIsEnabled == (item->flags() & Qt::ItemIsEnabled))
            {
                 data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(storage_id) );
                bool bWasVisible(spModel->isVisible());
                bool bVisible = item->checkState() == Qt::Checked;
                if ( (bWasVisible && !bVisible) || (!bWasVisible && bVisible) )
                {
                    spModel->setVisibility(bVisible);
                    m_bMyChange = true;
                    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::VISIBILITY_CHANGED);
                }
            }
        }
        break;

	// Cut visibility changed
	case COL_CUT:
		{
			if (Qt::ItemIsEnabled == (item->flags() & Qt::ItemIsEnabled))
			{
				bool bVisible = item->checkState() == Qt::Checked;
				VPL_SIGNAL(SigSetModelCutVisibility).invoke(storage_id,bVisible);				
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
    if(selectedList.empty())
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

/**
 * \brief	Query if this object is region linked.
 *
 * \return	true if region linked, false if not.
 */

bool CModelsWidget::isRegionLinked(int id)
{
	//data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
	//return m_bModelsLinked && id >= 0 && id < MAX_IMPORTED_MODELS && id < spColoring->getNumOfRegions();

	data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id + data::Storage::ImportedModel::Id));
	return (spModel->getRegionId() != -1) ? true : false;
}

/**
 * \brief	Model to region.
 *
 * \param	id  	The identifier.
 * \param	what	The what.
 */
void CModelsWidget::modelToRegion(int id, int what)
{
	if(!m_bModelsLinked)
		return;

	data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));

	// Model id is higher than number of regions
	if(id >= spColoring->getNumOfRegions())
		return;

	// Get model pointer
	data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id + data::Storage::ImportedModel::Id));
	
	switch(what)
	{
	case COL_NAME:
		spColoring->getRegionInfo(spModel->getRegionId()).setName(spModel->getLabel());
		break;

	case COL_COLOR:
		{
			data::CColor4f mc(spModel->getColor());
			data::CRegionColoring::tColor rc(mc.getR()*255, mc.getG()*255, mc.getB()*255, mc.getA()*255);
			spColoring->setColor(spModel->getRegionId(), rc);
		}
		break;

	default:
		break;
	}

	// Invalidate coloring
	APP_STORAGE.invalidate(spColoring.getEntryPtr());
}

/**
 * \brief	Models to regions.
 */

void CModelsWidget::modelsToRegions()
{
	if(!m_bModelsLinked)
		return;

	data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));

	// For all models
	for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
	{
		int storage_id(i + data::Storage::ImportedModel::Id);

		// Model id is higher than number of regions
		if(i >= spColoring->getNumOfRegions())
			break;;

		// Get model pointer
		data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storage_id));

		// Not valid model
		if(!spModel->hasData())
			continue;

		// Set label and color
		spColoring->getRegionInfo(spModel->getRegionId()).setName(spModel->getLabel());
		data::CColor4f mc(spModel->getColor());
		data::CRegionColoring::tColor rc(mc.getR()*255, mc.getG()*255, mc.getB()*255, mc.getA()*255);
		spColoring->setColor(spModel->getRegionId(), rc);
	}

	// Invalidate coloring
	APP_STORAGE.invalidate(spColoring.getEntryPtr());
}


void CModelsWidget::on_checkBoxIOApplyDICOM_clicked()
{
	bool bUse = ui->checkBoxIOApplyDICOM->isChecked();
	QSettings settings;
	settings.setValue("STLUseDICOMCoord",bUse);
}

void CModelsWidget::tableModelsContextMenu(QPoint p)
{
	QPoint pos = ui->tableModels->mapToGlobal(p);
	QTableWidgetItem* pItem = ui->tableModels->itemAt(p);
	if (NULL!=pItem)
	{
		const int row = pItem->row();
		QPushButton *pushButton = qobject_cast<QPushButton *>(ui->tableModels->cellWidget(row, COL_COLOR));
		if (pushButton == NULL)
		{
			return;
		}

		int storage_id = pushButton->property("StorageID").toInt();	
	    if(storage_id < data::Storage::ImportedModel::Id || storage_id >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
			return;

		QMenu contextMenu;
		QAction* moveAct = contextMenu.addAction(tr("Adjust Position..."));
		QAction* centerAct = contextMenu.addAction(tr("Center Position"));
		const QAction* win=contextMenu.exec(pos);
		if (NULL==win)
			return;
		if (win==moveAct)
		{
			data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storage_id));
			osg::Matrix mx = spModel->getTransformationMatrix();

			data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
			const double sX = spVolume->getXSize()*spVolume->getDX();
			const double sY = spVolume->getYSize()*spVolume->getDY();
			const double sZ = spVolume->getZSize()*spVolume->getDZ();

			QDialog dlg(this,Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
			dlg.setWindowTitle(tr("Position"));
			QGridLayout layout;
			QDoubleSpinBox spinX, spinY, spinZ;
			spinX.setSuffix(tr(" mm"));
			spinY.setSuffix(tr(" mm"));
			spinZ.setSuffix(tr(" mm"));
			spinX.setSingleStep(spVolume->getDX()/2);
			spinY.setSingleStep(spVolume->getDY()/2);
			spinZ.setSingleStep(spVolume->getDZ()/2);
			spinX.setMinimum(-sX);
			spinY.setMinimum(-sY);
			spinZ.setMinimum(-sZ);			
			spinX.setMaximum(+sX);
			spinY.setMaximum(+sY);
			spinZ.setMaximum(+sZ);
			osg::Vec3 pos = mx.getTrans();
			spinX.setValue(pos[0]);
			spinY.setValue(pos[1]);
			spinZ.setValue(pos[2]);
			layout.addWidget(new QLabel(tr("X:")),0,0);    
			layout.addWidget(&spinX,0,1);
			layout.addWidget(new QLabel(tr("Y:")),1,0);
			layout.addWidget(&spinY,1,1);
			layout.addWidget(new QLabel(tr("Z:")),2,0);
			layout.addWidget(&spinZ,2,1);
			QDialogButtonBox box(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
			layout.addWidget(&box,3,0,1,2);
			QObject::connect(&box, SIGNAL(accepted()), &dlg, SLOT(accept()));
			QObject::connect(&box, SIGNAL(rejected()), &dlg, SLOT(reject()));
			dlg.setLayout(&layout);

			if (QDialog::Accepted==dlg.exec())
			{
                std::vector<int> modelList;
				MainWindow::getInstance()->getModelManager()->createAndStoreSnapshot(modelList);
				pos[0] = spinX.value();
				pos[1] = spinY.value();
				pos[2] = spinZ.value();
				mx.setTrans(pos);
				spModel->setTransformationMatrix(mx);
				APP_STORAGE.invalidate( spModel.getEntryPtr() );
			}
		}
		if (win==centerAct)
		{
			data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storage_id));
			osg::Matrix mx = spModel->getTransformationMatrix();
			geometry::CMesh* pMesh=spModel->getMesh();
			if (NULL!=pMesh && pMesh->n_vertices() > 0)
			{
                std::vector<int> modelList;
				MainWindow::getInstance()->getModelManager()->createAndStoreSnapshot(modelList);
				data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
				const double sX = spVolume->getXSize()*spVolume->getDX();
				const double sY = spVolume->getYSize()*spVolume->getDY();
				const double sZ = spVolume->getZSize()*spVolume->getDZ();

				geometry::CMesh::Point min, max;
				pMesh->calc_bounding_box(min,max);
				mx.setTrans(-min[0]-((max[0]-min[0])/2)+sX/2,-min[1]-((max[1]-min[1])/2)+sY/2,-min[2]-((max[2]-min[2])/2)+sZ/2);
				spModel->setTransformationMatrix(mx);
				APP_STORAGE.invalidate( spModel.getEntryPtr() );
			}			
		}
	}
}

void CModelsWidget::showModelInfo()
{
	// Get button
	QPushButton *pb(qobject_cast<QPushButton *>(sender()));

	if (pb == 0)
		return;

	// Try to get property
	int index(pb->property("StorageID").toInt());
	if (index < data::Storage::ImportedModel::Id || index >= data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
		return;

	// Get model
	data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(index));

	if (!spModel->hasData())
		return;

	CInfoDialog info(NULL, tr("Model Information"));

	geometry::CMesh *pMesh = spModel->getMesh();
	data::CColor4f color = spModel->getColor();

	info.addRow(tr("Name"), QString::fromStdString(spModel->getLabel()), QColor(color.getR() * 255, color.getG() * 255, color.getB() * 255));
	info.addRow(tr("Triangles"), QString::number(pMesh->n_faces()));
	info.addRow(tr("Nodes"), QString::number(pMesh->n_vertices()));
	info.addRow(tr("Components"), QString::number(pMesh->componentCount()));

	info.exec();
}

void CModelsWidget::on_pushButtonBuyPlugins_clicked()
{
	QDesktopServices::openUrl(QUrl("http://www.3dim-laboratory.cz/software/plugins/"));
}

void CModelsWidget::on_pushButtonSaveModel_clicked()
{
	VPL_SIGNAL(SigSaveModel).invoke2(getSelectedModelStorageId());
}