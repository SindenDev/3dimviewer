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

#include "cpreferencesdialog.h"
#include "ui_cpreferencesdialog.h"
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QColorDialog>
#include <QStyleFactory>
#include <C3DimApplication.h>

CPreferencesDialog::CPreferencesDialog(const QDir &localeDir, QMenuBar* pMenuBar, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::CPreferencesDialog)
{
    m_bColorsChanged = false;
    m_bChangesNeedRestart = false;
	m_bChangedShortcuts = false;
    ui->setupUi(this);
    // get current language
    QSettings settings;
    QString lngFile=settings.value("Language","").toString();
    // setup combo with available languages
    ui->comboBoxLanguage->addItem(tr("English"));
    // search qm files in applications directory
    QDir myDir = localeDir;
    QStringList list = myDir.entryList(QStringList("*.qm"));
    QStringList::const_iterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it)
    {
        QLocale loc((*it));
        // if filename is not a valid locale name (qt defaults to "C") then it is not a translation
        // of the main app but of a plugin in a format pluginname_localename.qm
        if (QLocale::C!=loc.language())
        {
            ui->comboBoxLanguage->addItem(QLocale::languageToString(loc.language()),(*it));
            if (0==QString::compare(lngFile,(*it),Qt::CaseInsensitive))
                ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->count()-1);
        }
    }
    // get threading mode
    int nThreadingMode=settings.value("Threading").toInt();
    ui->comboBoxRenderingMode->setCurrentIndex(nThreadingMode);
    // error logging
    bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(true)).toBool();
    ui->checkBoxLogging->setChecked(bLoggingEnabled);
	// Models linked to regions
	bool bModelsLinkEnabled = settings.value("ModelRegionLinkEnabled", QVariant(DEFAULT_MODEL_REGION_LINK)).toBool();
	ui->checkBoxLinkModels->setChecked(bModelsLinkEnabled);
    //
    // get bg color
    // because style sheets aren't compatible with QProxyStyle that we use
    // we have to set some default style to the button
    m_bgColor=settings.value("BGColor",DEFAULT_BACKGROUND_COLOR).toUInt();
    ui->buttonBGColor->setStyle(QStyleFactory::create("windows"));
    setButtonColor(m_bgColor, m_bgColor, ui->buttonBGColor);
    // save path
    int savePathMode = settings.value("SavePathMode",DEFAULT_SAVE_PATH_MODE).toInt();
    ui->radioButtonPathLastUsed->setChecked(savePathMode==0);
    ui->radioButtonPathProject->setChecked(savePathMode==1);

    connect(this,SIGNAL(accepted()),this,SLOT(on_CPreferencesDialog_accepted()));
    QPushButton* resetButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetDefaultsPressed()));
    connect(ui->pushButtonShowLog,SIGNAL(clicked()),qobject_cast<C3DimApplication*>(qApp),SLOT(showLog()));
    //
	connect(ui->listPages,SIGNAL(currentRowChanged(int)),this,SLOT(pageChange(int)));

	// shortcuts
	ui->treeWidget->header()->setStretchLastSection(false);
	ui->treeWidget->header()->setResizeMode(0, QHeaderView::Stretch);
	ui->treeWidget->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	ui->treeWidget->header()->setMinimumSectionSize(50);
	if (NULL!=pMenuBar)
	{
		QList<QMenu*> lst = pMenuBar->findChildren<QMenu*>();
		foreach(QMenu* m, lst)
		{
			if (m->parent()==pMenuBar)
				addTreeMenu(m,NULL);
		}
	}
	connect(ui->treeWidget,SIGNAL(itemSelectionChanged()),this,SLOT(treeItemSelectionChanged()));
	ui->lineEditShortCut->installEventFilter(this);
	ui->lineEditShortCut->setEnabled(false);
	ui->pushButtonSetShortcut->setEnabled(false);
	ui->pushButtonClearShortcut->setEnabled(false);
    
    settings.beginGroup("PreferencesDialog");
    resize(settings.value("size").toSize());
    settings.endGroup();

	int page = settings.value("PreferencesPage",0).toInt();
    ui->listPages->setCurrentRow(page);	
}

CPreferencesDialog::~CPreferencesDialog()
{
	QSettings settings;
	settings.setValue("PreferencesPage",ui->listPages->currentRow());
    if ( Qt::WindowMaximized!=QWidget::windowState ())
    {
        settings.beginGroup("PreferencesDialog");
        settings.setValue("size",size());
        settings.endGroup();
    }
    delete ui;
}

void CPreferencesDialog::pageChange(int index)
{    
    ui->labelPageName->setText(ui->listPages->item(index)->text());
    ui->stackedWidget->setCurrentIndex(index);
}

void CPreferencesDialog::on_CPreferencesDialog_accepted()
{
    int index=ui->comboBoxLanguage->currentIndex();
    QString lngFile=ui->comboBoxLanguage->itemData(index).toString();
    QSettings settings;
    if (0!=lngFile.compare(settings.value("Language","").toString(),Qt::CaseInsensitive))
    {
        settings.setValue("Language",lngFile);
        m_bChangesNeedRestart=true;
    }

    // advanced settings
    const int threading = ui->comboBoxRenderingMode->currentIndex();
    if (settings.value("Threading").toInt()!=threading)
    {
        settings.setValue("Threading", ui->comboBoxRenderingMode->currentIndex());
        m_bChangesNeedRestart=true;
    }

    const bool bWantLogging = ui->checkBoxLogging->isChecked();
    bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(DEFAULT_LOGGING)).toBool();
    if (bWantLogging!=bLoggingEnabled)
    {
        settings.setValue("LoggingEnabled", bWantLogging);
        m_bChangesNeedRestart=true;
    }

	const bool bWantModelRegionLink = ui->checkBoxLinkModels->isChecked();
	bool bModelRegionLink = settings.value("ModelRegionLinkEnabled", QVariant(DEFAULT_MODEL_REGION_LINK)).toBool();
	if(bModelRegionLink!=bWantModelRegionLink)
	{
		settings.setValue("ModelRegionLinkEnabled", bWantModelRegionLink);
		m_bChangesNeedRestart = true;
	}
    QRgb color;
    color = m_bgColor.rgb();
    if (settings.value("BGColor",DEFAULT_BACKGROUND_COLOR).toUInt()!=color)
    {
        settings.setValue("BGColor",color);
        m_bColorsChanged=true;
    }

    int savePathMode = DEFAULT_SAVE_PATH_MODE;
    if (ui->radioButtonPathLastUsed->isChecked())
        savePathMode=0;
    if (ui->radioButtonPathProject->isChecked())
        savePathMode=1;
    settings.setValue("SavePathMode",savePathMode);
}

void CPreferencesDialog::setButtonColor(QColor& targetColor, const QColor& sourceColor, QPushButton *targetButton)
{
    targetColor=sourceColor;
    QString str="* { background-color: "+sourceColor.name()+" }";
    targetButton->setStyleSheet(str);
}

void CPreferencesDialog::on_buttonBGColor_clicked()
{
    QColorDialog dlg(this);
    QColor color=dlg.getColor(m_bgColor,this);
    if (color.isValid())
    {
        setButtonColor(m_bgColor, color, ui->buttonBGColor);
    }
}


void CPreferencesDialog::removeCustomShortcuts( QTreeWidgetItem *item )
{
	if (NULL!=item)
	{
		QAction* pAct=(QAction*)item->data(1,Qt::UserRole).value<void*>();
		if (NULL!=pAct)
		{
			if (0==pAct->property("Shortcut").toString().compare("Custom",Qt::CaseInsensitive))
			{
				QKeySequence ks;					
				pAct->setShortcut(ks);
				pAct->setProperty("Shortcut","");
				item->setText(1,"");
			}
		}
	}

    for( int i = 0; i < item->childCount(); ++i )
	{
        removeCustomShortcuts( item->child(i) );		
	}
}

void CPreferencesDialog::resetDefaultsPressed( )
{
    // set threading to single threaded
    ui->comboBoxRenderingMode->setCurrentIndex(0);
    // set error logging
    ui->checkBoxLogging->setChecked(DEFAULT_LOGGING);
	// Set model/region link
	ui->checkBoxLinkModels->setChecked(DEFAULT_MODEL_REGION_LINK);
    // set background color
    m_bgColor=DEFAULT_BACKGROUND_COLOR;
    setButtonColor(m_bgColor, m_bgColor, ui->buttonBGColor);
    //
    int savePathMode = DEFAULT_SAVE_PATH_MODE;
    ui->radioButtonPathLastUsed->setChecked(savePathMode==0);
    ui->radioButtonPathProject->setChecked(savePathMode==1);
	// reset keyboard shortcuts
	QSettings settings;
	settings.beginGroup("Shortcuts");
	settings.remove("");
	m_bChangedShortcuts = false;
	// reset tree widgets and actions
	removeCustomShortcuts( ui->treeWidget->invisibleRootItem() );
}

QTreeWidgetItem* CPreferencesDialog::addTreeRoot(QString name, QString description) 
{ 
	QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget); 
	int idxAnd = name.indexOf("&");
	if (idxAnd>=0 && idxAnd<name.length()-1)
	{
		name.remove("&");
		if (description.isEmpty())
			description = tr("Alt+%1").arg(QString(name.at(idxAnd).toUpper()));
	}
	treeItem->setText(0, name); 
	treeItem->setText(1, description); 
	return treeItem;
} 

QTreeWidgetItem * CPreferencesDialog::addTreeChild(QTreeWidgetItem *parent, QString name, QString description, QAction* pAction) 
{ 	
	QTreeWidgetItem *treeItem = new QTreeWidgetItem(); 
	if (NULL==pAction) // menu
		name.remove("&");
	treeItem->setText(0, name); 
	treeItem->setText(1, description); 
	treeItem->setData(1, Qt::UserRole, QVariant::fromValue((void*)pAction));
	if (NULL!=pAction)
	{
		if (pAction->icon().isNull())
		{
			QPixmap pm(24,24);
			pm.fill(Qt::transparent);
			treeItem->setIcon(0,QIcon(pm));
		}
		else
			treeItem->setIcon(0,pAction->icon());
	}
	parent->addChild(treeItem); 
	return treeItem;
} 

QTreeWidgetItem* CPreferencesDialog::addTreeMenu(QMenu* m, QTreeWidgetItem *parent) 
{
	if (NULL==m)
		return NULL;
	if (0==m->objectName().compare("menuRecent",Qt::CaseInsensitive)) // ignore "Recent" menu
	{
	}
	else
	{
		QTreeWidgetItem* pMenuItem = NULL==parent?addTreeRoot(m->title(),"") : addTreeChild(parent,m->title(),"",NULL);
		foreach(QAction* a, m->actions())
		{
			if (!a->isSeparator() && a->isVisible())
			{
				if (NULL!=a->menu())
					addTreeMenu(a->menu(), pMenuItem);
				else
					addTreeChild(pMenuItem,a->text(),a->shortcut().toString(QKeySequence::NativeText),a);
			}
			
		}
		pMenuItem->setExpanded(true);
		return pMenuItem;
	}
	return NULL;
}

void CPreferencesDialog::treeItemSelectionChanged()
{
	ui->lineEditShortCut->setText("");
	ui->lineEditShortCut->setStyleSheet("");
	QList<QTreeWidgetItem *> selItems = ui->treeWidget->selectedItems();
	ui->lineEditShortCut->setEnabled(!selItems.empty() && 0==selItems[0]->childCount());
	ui->pushButtonSetShortcut->setEnabled(!selItems.empty() && 0==selItems[0]->childCount());
	ui->pushButtonClearShortcut->setEnabled(!selItems.empty() && 0==selItems[0]->childCount());
	if (!selItems.empty())
	{
		foreach(QTreeWidgetItem *it, selItems)
		{
			if (0==it->childCount())
				ui->lineEditShortCut->setText(it->text(1));
		}
	}		
}

void CPreferencesDialog::on_pushButtonSetShortcut_clicked()
{
	QList<QTreeWidgetItem *> selItems = ui->treeWidget->selectedItems();
	if (!selItems.empty())
	{
		foreach(QTreeWidgetItem *it, selItems)
		{
			if (0==it->childCount())
			{				
				QAction* pAct=(QAction*)it->data(1,Qt::UserRole).value<void*>();
				if (NULL!=pAct)
				{
					m_bChangedShortcuts = true;

					QKeySequence ks(ui->lineEditShortCut->text());					
					pAct->setShortcut(ks);
					pAct->setProperty("Shortcut","Custom");
					ui->lineEditShortCut->setText(ks.toString(QKeySequence::NativeText));
					it->setText(1,ks.toString(QKeySequence::NativeText));
				}
			}				
		}
	}	
}

void CPreferencesDialog::on_pushButtonClearShortcut_clicked()
{
	QList<QTreeWidgetItem *> selItems = ui->treeWidget->selectedItems();
	if (!selItems.empty())
	{
		foreach(QTreeWidgetItem *it, selItems)
		{
			if (0==it->childCount())
			{				
				QAction* pAct=(QAction*)it->data(1,Qt::UserRole).value<void*>();
				if (NULL!=pAct)
				{
					m_bChangedShortcuts = true;

					QKeySequence ks;					
					pAct->setShortcut(ks);
					pAct->setProperty("Shortcut","Custom");
					ui->lineEditShortCut->setText(ks.toString(QKeySequence::NativeText));
					it->setText(1,ks.toString(QKeySequence::NativeText));
				}
			}				
		}
	}	
}

bool CPreferencesDialog::shortcutUsed(const QString& shortcut)
{
	QList<QTreeWidgetItem*> lst = ui->treeWidget->findItems(shortcut,Qt::MatchExactly|Qt::MatchRecursive,1);
	return !lst.empty();
}

bool CPreferencesDialog::eventFilter(QObject* obj, QEvent *event)
{
    if (obj == ui->lineEditShortCut)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			int keyInt = keyEvent->key(); 
			Qt::Key key = static_cast<Qt::Key>(keyInt); 
			if(key == Qt::Key_unknown)
				return false;
			Qt::KeyboardModifiers mod = keyEvent->modifiers();
			if(key == Qt::Key_Control || 
				key == Qt::Key_Shift || 
				key == Qt::Key_Alt || 
				key == Qt::Key_Meta ||
				(mod.testFlag(Qt::NoModifier) && key == Qt::Key_Escape) ||
				(mod.testFlag(Qt::NoModifier) && key == Qt::Key_Backspace) ||
				(mod.testFlag(Qt::NoModifier) && key == Qt::Key_Return)
				)
			{
				keyInt=0;
			}
			if (mod.testFlag(Qt::ShiftModifier))
				keyInt += Qt::SHIFT; 
			if (mod.testFlag(Qt::ControlModifier))
				keyInt += Qt::CTRL; 
			if (mod.testFlag(Qt::AltModifier))
				keyInt += Qt::ALT; 
			if (mod.testFlag(Qt::MetaModifier))
				keyInt += Qt::META; 
			ui->lineEditShortCut->setText(QKeySequence(keyInt).toString(QKeySequence::NativeText));
			if (shortcutUsed(ui->lineEditShortCut->text()))
				ui->lineEditShortCut->setStyleSheet("color: #ff0000");
			else
				ui->lineEditShortCut->setStyleSheet("");
			return true;
        }
        return false;
    }
    return QDialog::eventFilter(obj, event);
}