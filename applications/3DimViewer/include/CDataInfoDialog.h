///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2013 3Dim Laboratory s.r.o.
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

#ifndef CDATAINFODIALOG_H
#define CDATAINFODIALOG_H

#include <QDialog>

namespace Ui {
class CDataInfoDialog;
}

class CDataInfoDialog : public QDialog
{
    Q_OBJECT    
public:
    explicit CDataInfoDialog(int model_storage_id, QWidget *parent = NULL);
    ~CDataInfoDialog();
    // reformats dicom formated date and time to locale based format
    static QString formatDicomDateAndTime(const QString& wsDate, const QString& wsTime);
protected:
    void AddRowIfNotEmpty(const QString& param, const QString& value, const QColor& color = QColor(QColor::Invalid));
    void AddRow(const QString &param, const QString &value, const QColor& color = QColor(QColor::Invalid));
private:
    Ui::CDataInfoDialog *ui;
};

#endif // CDATAINFODIALOG_H
