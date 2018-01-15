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

#include "qtplugin/PluginInterface.h"

#include "CPluginInfoDialog.h"

#include <QPluginLoader>
#include <QStringList>
#include <QDir>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QCoreApplication>
#include <QSettings>

CPluginInfoDialog::CPluginInfoDialog(CPluginManager* pPluginManager, QWidget *parent) :
     QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
     label(new QLabel),
     treeWidget(new QTreeWidget),
     okButton(new QPushButton(tr("OK")))
{
     setSizeGripEnabled(true);

     treeWidget->setAlternatingRowColors(false);
     treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
     treeWidget->setColumnCount(1);
     treeWidget->header()->hide();

     okButton->setDefault(true);
     connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

     QGridLayout *mainLayout = new QGridLayout;
     mainLayout->setColumnStretch(0, 1);
     mainLayout->setColumnStretch(2, 1);
     mainLayout->addWidget(label, 0, 0, 1, 3);
     mainLayout->addWidget(treeWidget, 1, 0, 1, 3);
     mainLayout->addWidget(okButton, 2, 1);
     setLayout(mainLayout);

     interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                             QIcon::Normal, QIcon::On);
     interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                             QIcon::Normal, QIcon::Off);
     featureIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));

     setWindowTitle(tr("Plugin Information"));

     QSettings settings;
     settings.beginGroup("PluginInfoWindow");
     resize(settings.value("size",size()).toSize());
     settings.endGroup();

     findPlugins(pPluginManager->getPluginsPath(), *(pPluginManager->getPluginsList()));
 }

 CPluginInfoDialog::~CPluginInfoDialog()
 {
     QSettings settings;
     settings.beginGroup("PluginInfoWindow");
     settings.setValue("size",size());
     settings.endGroup();
 }

 void CPluginInfoDialog::findPlugins(const QString &path,
                                const QStringList &fileNames)
 {
     label->setText(tr("%1 found the following plugins in\n"
                       "%2")
                    .arg(QCoreApplication::applicationName(),QDir::toNativeSeparators(path)));

     foreach (QObject *plugin, QPluginLoader::staticInstances())
         populateTreeWidget(plugin, tr("%1 (Static Plugin)").arg(plugin->metaObject()->className()));

     const QDir dir(path);
     foreach (QString fileName, fileNames)
     {
         QPluginLoader loader(dir.absoluteFilePath(fileName));
         QObject *plugin = loader.instance();
         if (plugin)
             populateTreeWidget(plugin, QFileInfo(fileName).fileName());
     }
 }

 void CPluginInfoDialog::populateTreeWidget(QObject *plugin, const QString &text)
 {
     QTreeWidgetItem *pluginItem = new QTreeWidgetItem(treeWidget);
     pluginItem->setText(0, text);
     treeWidget->setItemExpanded(pluginItem, true);

     QFont boldFont = pluginItem->font(0);
     boldFont.setBold(true);
     pluginItem->setFont(0, boldFont);

     if (plugin)
     {
         PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
         if (iPlugin)
             addItems(pluginItem, "PluginInterface", QStringList());
         PluginLicenseInterface *iLicPlugin = qobject_cast<PluginLicenseInterface *>(plugin);
         if (iLicPlugin)
             addItems(pluginItem, "PluginLicenseInterface", QStringList());
     }
 }

 void CPluginInfoDialog::addItems(QTreeWidgetItem *pluginItem,
                             const char *interfaceName,
                             const QStringList &features)
 {
     QTreeWidgetItem *interfaceItem = new QTreeWidgetItem(pluginItem);
     interfaceItem->setText(0, interfaceName);
     interfaceItem->setIcon(0, interfaceIcon);

     foreach (QString feature, features)
     {
         if (feature.endsWith("..."))
             feature.chop(3);
         QTreeWidgetItem *featureItem = new QTreeWidgetItem(interfaceItem);
         featureItem->setText(0, feature);
         featureItem->setIcon(0, featureIcon);
     }
 }
