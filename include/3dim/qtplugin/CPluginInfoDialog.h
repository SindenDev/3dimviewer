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

#ifndef PLUGININFODIALOG_H
#define PLUGININFODIALOG_H

#include <QDialog>
#include <QIcon>

class QLabel;
class QPushButton;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;

#include <CPluginManager.h>

//! Dialog with basic information about loaded plugins
class CPluginInfoDialog : public QDialog
{
    Q_OBJECT

public:
    CPluginInfoDialog(CPluginManager* pPluginManager, QWidget *parent = 0);
    virtual ~CPluginInfoDialog();

private:
    void findPlugins(const QString &path, const QStringList &fileNames);
    void populateTreeWidget(QObject *plugin, const QString &text);
    void addItems(QTreeWidgetItem *pluginItem, const char *interfaceName,
                  const QStringList &features);

    QLabel *label;
    QTreeWidget *treeWidget;
    QPushButton *okButton;
    QIcon interfaceIcon;
    QIcon featureIcon;
};

#endif
