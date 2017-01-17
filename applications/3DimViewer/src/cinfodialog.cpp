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

#include "cinfodialog.h"
#include "ui_cinfodialog.h"

#include <QDialog>
#include <QTableWidget>
#include <QColorDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include <data/CRegionData.h>

Q_DECLARE_METATYPE(std::vector<int>)

CInfoDialog::CInfoDialog(QWidget *parent, const QString& title) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
	ui(new Ui::CInfoDialog)
{
    ui->setupUi(this);   
	this->setWindowTitle(title);
}

CInfoDialog::~CInfoDialog()
{
    delete ui;
}

void CInfoDialog::addRow(const QString& param, const QString& value, const QColor& color, int alignment)
{
	int Id = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(Id);

	QTableWidgetItem *i0 = new QTableWidgetItem(param);
	i0->setFlags(i0->flags() & ~Qt::ItemIsEditable);

	if (color.isValid())
		i0->setBackgroundColor(color);

	ui->tableWidget->setItem(Id, 0, i0);

	QTableWidgetItem *i1 = new QTableWidgetItem(value);
	i1->setFlags(i1->flags() & ~Qt::ItemIsEditable);
	i1->setTextAlignment(alignment);

	if (color.isValid())
		i1->setBackgroundColor(color);

	ui->tableWidget->setItem(Id, 1, i1);
}