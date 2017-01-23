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

#ifndef CINFODIIALOG_H
#define CINFODIIALOG_H

#include <data/CRegionColoring.h>

#include <QTableWidget>
#include <QDialog>
#include <QColor>
#include <QString>

#if defined(__APPLE__) &&  !defined(_LIBCPP_VERSION)
  #include <tr1/array>
#else
  #include <array>
#endif
#include <data/CModelManager.h>

namespace Ui {
	class CInfoDialog;
}

class CInfoDialog : public QDialog
{
    Q_OBJECT

public:
	CInfoDialog(QWidget *parent, const QString& title);

	~CInfoDialog();

	void addRow(const QString &param, const QString &value, const QColor& color = QColor(QColor::Invalid), int alignment = Qt::AlignCenter);

private:
	Ui::CInfoDialog *ui;
};


#endif // CINFODIIALOG_H
