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

#include "cpreferencesdialog.h"
#include "ui_cpreferencesdialog.h"
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QColorDialog>
#include <QStyleFactory>
#include <QShortcut>
#include <C3DimApplication.h>
#include <qtcompat.h>
#include <QFileDialog>
#include <mainwindow.h>

#ifdef ENABLE_PYTHON
#include <qtpython/pyconfigure.h>
#include <qtpython/interpret.h>
#include <qtpython/settings.h>
#include <QtGui/dialogs/messageWindow.h>
#endif

CPreferencesDialog::CPreferencesDialog(const QDir &localeDir, QMenuBar* pMenuBar, CEventFilter &eventFilter, QWidget *parent, Qt::WindowFlags f) :
    m_eventFilter(eventFilter),
	QDialog(parent, f),
    ui(new Ui::CPreferencesDialog)
{
    m_bColorsChanged = false;
    m_bChangesNeedRestart = false;
	m_bChangedShortcuts = false;
    m_bReinitializeInterpret = false;
    ui->setupUi(this);
    //ui->comboBoxRenderingMode->setVisible(false);
    //ui->labelRenderingMode->setVisible(false);

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
    // antialiasing
    bool bAntialiasingEnabled = settings.value("AntialiasingEnabled", QVariant(true)).toBool();
    ui->checkBoxAntialiasing->setChecked(bAntialiasingEnabled);
    // error logging
    bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(true)).toBool();
    ui->checkBoxLogging->setChecked(bLoggingEnabled);
    // log opengl errors
    bool logOpenglErrors = settings.value("LogOpenglErrors", QVariant(DEFAULT_LOG_OPENGL_ERRORS)).toBool();
    ui->checkBoxLogOpenglErrors->setChecked(logOpenglErrors);
    // DICOM port
    int nDicomPort = settings.value("DicomPort", DEFAULT_DICOM_PORT).toInt();
    ui->spinBoxDicomPort->setValue(nDicomPort);
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

	// name of saved files
	int savedFilesNameMode = settings.value("SavedFilesNameMode", DEFAULT_SAVED_FILES_NAME_MODE).toInt();
	ui->radioButtonPatientName->setChecked(savedFilesNameMode == 0);
	ui->radioButtonFolderName->setChecked(savedFilesNameMode == 1);

    // get models format
    int modelsFormat = settings.value("ExportedModelsFormat").toInt();
    ui->comboBoxDefaultModelsFormat->setCurrentIndex(modelsFormat);

    connect(this,SIGNAL(accepted()),this,SLOT(on_CPreferencesDialog_accepted()));
    QPushButton* resetButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetDefaultsPressed()));
    connect(ui->pushButtonShowLog,SIGNAL(clicked()),qobject_cast<C3DimApplication*>(qApp),SLOT(showLog()));
    //
	connect(ui->listPages,SIGNAL(currentRowChanged(int)),this,SLOT(pageChange(int)));

    // segmentation
    int brushThickness = settings.value("BrushThickness", DEFAULT_BRUSH_THICKNESS).toInt();
    ui->radioButtonThin->setChecked(brushThickness == 0);
    ui->radioButtonThick->setChecked(brushThickness == 1);

	// shortcuts
	ui->treeWidget->header()->setStretchLastSection(false);
	ui->treeWidget->header()->SETRESIZEMODE(0, QHeaderView::Stretch);
	ui->treeWidget->header()->SETRESIZEMODE(1, QHeaderView::ResizeToContents);
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

	// event filter stuff
	ui->eFTypes->setEnabled(false);
	ui->eFObjects->setEnabled(false);

	connect(ui->eFShowLog, SIGNAL(clicked()), &m_eventFilter, SLOT(showEventFilterLog()));

	connect(ui->eFBrowse, SIGNAL(clicked()), this, SLOT(showFileDialog()));
	connect(ui->eFEnable, SIGNAL(stateChanged(int)), this, SLOT(showHideFilterOptions(int)));

	ui->eFPath->setReadOnly(true);
	ui->eFEnable->setChecked(settings.value("logEnabled", false).toBool());
	ui->eFMouse->setChecked(settings.value("logMouseEnabled", true).toBool());
	ui->eFKeyboard->setChecked(settings.value("logKeyboardEnabled", true).toBool());
	ui->eFCustom->setChecked(settings.value("logCustomEnabled", true).toBool());

	setMonitoredObjectsChecked();

#if QT_VERSION < 0x050000
	QString previousDir = settings.value("logOuputDir", QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
	QString previousDir = settings.value("logOuputDir", QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

	ui->eFPath->setText(previousDir);

#ifdef ENABLE_DEEPLEARNING
    // utilities
    connect(ui->eFBrowseDeepLearning, SIGNAL(clicked()), this, SLOT(showFileDialogDeepLearning()));
    settings.beginGroup("DeepLearning");
    QString text = settings.value("deepLearningDir2").toString();
    ui->eFPathDeepLearning->setText(text.isEmpty() ? QCoreApplication::applicationDirPath() + "/deeplearning" : text);
    settings.endGroup();
    ui->eFPathDeepLearning->setReadOnly(true);
    ui->eFPathDeepLearning->setEnabled(true);

    connect(ui->buttonResetDeepLearningDir, SIGNAL(clicked()), this, SLOT(resetDeepLearningDirToDefault()));
#else
    ui->eFBrowseDeepLearning->hide();
    ui->eFPathDeepLearning->hide();
    ui->label_6->hide();
    ui->buttonResetDeepLearningDir->hide();

    // comparison in czech doesn't work
    //for (int index = 0; index < ui->listPages->count(); index++)
    {
        int index = 4;

        //if (ui->listPages->item(index)->text() == "Utilities" || ui->listPages->item(index)->text() == QString::fromUtf8("Další"))
        {
            if (page == index)
            {
                ui->listPages->setCurrentRow(0);
            }
            ui->listPages->item(index)->setHidden(true);
        }
    }
#endif

#ifdef ENABLE_PYTHON
	pythonQt::Settings& pySettings = VPL_SINGLETON(pythonQt::Settings);
	const bool isPythonEnabled = pySettings.isPythonEnabled();
	ui->checkBoxEnablePython->setChecked(isPythonEnabled);
    ui->checkBoxEnablePython->hide();
    ui->lblVPLSwig->hide();
    ui->eFPathVPLSwig->hide();
    ui->eFBrowseVPLSwig->hide();
    ui->pbRestorePython->hide();
    ui->eFPathPythonPath->setReadOnly(true);
    ui->eFPathPythonPath->setEnabled(true);

    connect(ui->eFBrowseVPLSwig, SIGNAL(clicked()), this, SLOT(showFileDialogVPLSwig()));
    connect(ui->eFBrowsePythonPath, SIGNAL(clicked()), this, SLOT(showFileDialogPython()));
    connect(ui->comboBoxPythonType, SIGNAL(currentIndexChanged(int)), this, SLOT(pythonTypeChanged(int)));
    connect(ui->checkBoxEnablePython, SIGNAL(stateChanged(int)), this, SLOT(showHidePythonOptions(int)));
    connect(ui->pbRestorePython, SIGNAL(clicked()), this, SLOT(restorePython()));

	const QString previousVPLSwig = pySettings.getVplSwigPath();
	const QString previousPythonInterpret = pySettings.getExternalPythonPath();
    if (!pySettings.isSelectedInternalInterpret())
    {
	    const bool oldState = ui->comboBoxPythonType->blockSignals(true);
        ui->comboBoxPythonType->setCurrentIndex(1);
        ui->comboBoxPythonType->blockSignals(oldState);
    }

    ui->eFPathVPLSwig->setText(previousVPLSwig);
    ui->eFPathPythonPath->setText(previousPythonInterpret);

    ui->eFBrowseVPLSwig->setEnabled(isPythonEnabled);
    ui->eFBrowsePythonPath->setEnabled(true);
    ui->comboBoxPythonType->setEnabled(isPythonEnabled);
#else
    ui->eFBrowseVPLSwig->setDisabled(true);
    ui->eFBrowsePythonPath->setDisabled(true);
    ui->comboBoxPythonType->setDisabled(true);
    ui->checkBoxEnablePython->setDisabled(true);

    for (int index = 0; index < ui->listPages->count(); index++)
    {
        if (ui->listPages->item(index)->text() == "Python")
        {
            if (page == index)
            {
                ui->listPages->setCurrentRow(0);
            }
            ui->listPages->item(index)->setHidden(true);
        }
    }
#endif
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

void CPreferencesDialog::showFileDialog()
{
	QSettings settings;
#if QT_VERSION < 0x050000
	QString previousDir = settings.value("logOuputDir", QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
	QString previousDir = settings.value("logOuputDir", QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

	//show file dialog, which allows the user to select a folder
	QFileDialog *dialog = new QFileDialog();
	dialog->setFileMode(QFileDialog::Directory);
	dialog->setOption(QFileDialog::ShowDirsOnly);

	//get path to selected folder
	QString path = dialog->getExistingDirectory(this, tr("Select Directory"), previousDir);

	//show that path in ui
	if (!path.isEmpty())
	{
		settings.setValue("logOuputDir", path);
		ui->eFPath->setText(path);
	}

	delete dialog;
}

void CPreferencesDialog::showHideFilterOptions(int state)
{
    if (state == Qt::Checked)
    {
        ui->eFPath->setEnabled(true);
        ui->eFBrowse->setEnabled(true);
		ui->eFTypes->setEnabled(true);
		ui->eFObjects->setEnabled(true);
    }
    else
    {
		ui->eFPath->setEnabled(false);
		ui->eFBrowse->setEnabled(false);
		ui->eFTypes->setEnabled(false);
		ui->eFObjects->setEnabled(false);
    }
}

void CPreferencesDialog::setMonitoredObjectsChecked()
{
	QSettings settings;
	ui->eFMenu->setChecked(settings.value("logMenu", true).toBool());
	ui->eFPushButtons->setChecked(settings.value("logButtons", true).toBool());
	ui->eFSpinBoxes->setChecked(settings.value("logSpinBoxes", true).toBool());
	ui->eFCheckBoxes->setChecked(settings.value("logCheckBoxes", true).toBool());
	ui->eFRadioButtons->setChecked(settings.value("logRadioButtons", true).toBool());
	ui->eFTextEdits->setChecked(settings.value("logTextEdits", true).toBool());
	ui->eFComboBoxes->setChecked(settings.value("logComboBoxes", true).toBool());
	ui->eFSliders->setChecked(settings.value("logSliders", true).toBool());
	ui->eFOsgWindows->setChecked(settings.value("logOsgWindows", true).toBool());
	ui->eFTabBars->setChecked(settings.value("logTabBars", true).toBool());
	ui->eFLists->setChecked(settings.value("logLists", true).toBool());
	ui->eFTables->setChecked(settings.value("logTables", true).toBool());
}

void CPreferencesDialog::pageChange(int index)
{    
    ui->labelPageName->setText(ui->listPages->item(index)->text());
    ui->stackedWidget->setCurrentIndex(index);
}
#ifdef ENABLE_PYTHON
void CPreferencesDialog::restorePython()
{
    QSettings settings;
    // reset python settings
    const QString vplSwigDefault = QDir::currentPath() + "/" + pythonQt::settings::defaultVplswigPath;
    ui->eFPathVPLSwig->setText(vplSwigDefault);
    ui->comboBoxPythonType->setCurrentIndex(0);
}
#endif


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

    const bool bWantAntialiasing = ui->checkBoxAntialiasing->isChecked();
    bool bAntialiasingEnabled = settings.value("AntialiasingEnabled", QVariant(DEFAULT_ANTIALIASING)).toBool();
    if (bWantAntialiasing != bAntialiasingEnabled)
    {
        settings.setValue("AntialiasingEnabled", bWantAntialiasing);
        m_bChangesNeedRestart = true;
    }

    const bool bWantLogging = ui->checkBoxLogging->isChecked();
    bool bLoggingEnabled = settings.value("LoggingEnabled", QVariant(DEFAULT_LOGGING)).toBool();
    if (bWantLogging!=bLoggingEnabled)
    {
        settings.setValue("LoggingEnabled", bWantLogging);
        m_bChangesNeedRestart=true;
    }

    const bool wantLogOpenglErrors = ui->checkBoxLogOpenglErrors->isChecked();
    if (wantLogOpenglErrors != settings.value("LogOpenglErrors", QVariant(DEFAULT_LOG_OPENGL_ERRORS)).toBool())
    {
        settings.setValue("LogOpenglErrors", wantLogOpenglErrors);
        m_bChangesNeedRestart = true;
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

	int savesFilesNameMode = DEFAULT_SAVED_FILES_NAME_MODE;
	if (ui->radioButtonPatientName->isChecked())
		savesFilesNameMode = 0;
	if (ui->radioButtonFolderName->isChecked())
		savesFilesNameMode = 1;
	settings.setValue("SavedFilesNameMode", savesFilesNameMode);

    const int nDicomPort = ui->spinBoxDicomPort->value();
    if (settings.value("DicomPort").toInt() != nDicomPort)
		settings.setValue("DicomPort", ui->spinBoxDicomPort->value());

    int brushThickness = DEFAULT_BRUSH_THICKNESS;
    if (ui->radioButtonThin->isChecked())
        brushThickness = 0;
    if (ui->radioButtonThick->isChecked())
        brushThickness = 1;
    settings.setValue("BrushThickness", brushThickness);

	// save event filter stuff
	settings.setValue("logEnabled", ui->eFEnable->isChecked());
	settings.setValue("logMouseEnabled", ui->eFMouse->isChecked());
	settings.setValue("logKeyboardEnabled", ui->eFKeyboard->isChecked());
	settings.setValue("logCustomEnabled", ui->eFCustom->isChecked());

	settings.setValue("logMenu", ui->eFMenu->isChecked());
	settings.setValue("logButtons", ui->eFPushButtons->isChecked());
	settings.setValue("logSpinBoxes", ui->eFSpinBoxes->isChecked());
	settings.setValue("logCheckBoxes", ui->eFCheckBoxes->isChecked());
	settings.setValue("logRadioButtons", ui->eFRadioButtons->isChecked());
	settings.setValue("logTextEdits", ui->eFTextEdits->isChecked());
	settings.setValue("logComboBoxes", ui->eFComboBoxes->isChecked());
	settings.setValue("logSliders", ui->eFSliders->isChecked());
	settings.setValue("logOsgWindows", ui->eFOsgWindows->isChecked());
	settings.setValue("logTabBars", ui->eFTabBars->isChecked());
	settings.setValue("logLists", ui->eFLists->isChecked());
	settings.setValue("logTables", ui->eFTables->isChecked());
    settings.setValue("isPythonEnabled", ui->checkBoxEnablePython->isChecked());


    const int modelsFormat = ui->comboBoxDefaultModelsFormat->currentIndex();
    if (settings.value("ExportedModelsFormat").toInt() != modelsFormat)
    {
        settings.setValue("ExportedModelsFormat", ui->comboBoxDefaultModelsFormat->currentIndex());
    }

#ifdef ENABLE_PYTHON
	pythonQt::Settings& pySettings = VPL_SINGLETON(pythonQt::Settings);

	const QString vplSwigPath = pySettings.getVplSwigPath();
	if(vplSwigPath != ui->eFPathVPLSwig->displayText())
	{
		pySettings.setVplSwigPath(ui->eFPathVPLSwig->displayText());
		m_bReinitializeInterpret = true;
	}
	const QString externalPythonPath = pySettings.getExternalPythonPath();
	if (externalPythonPath != ui->eFPathPythonPath->displayText())
	{
		pySettings.setPythonExternalPath(ui->eFPathPythonPath->displayText());
		m_bReinitializeInterpret = true;
	}
	const bool isInternal = ui->comboBoxPythonType->currentIndex() == 0;
	if(isInternal != pySettings.isSelectedInternalInterpret())
	{
		pySettings.setInternalInterpret(isInternal);
		m_bReinitializeInterpret = true;
	}
	if (m_bReinitializeInterpret)
	{
		data::Interpret& interpret = *data::CObjectPtr<data::Interpret>(APP_STORAGE.getEntry(data::Storage::InterpretData::Id));
		interpret.notValid();
		m_bChangesNeedRestart = true;
	}
#endif
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
    // set antialiasing
    ui->checkBoxAntialiasing->setChecked(DEFAULT_ANTIALIASING);
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

	int savesFilesNameMode = DEFAULT_SAVED_FILES_NAME_MODE;
	ui->radioButtonPatientName->setChecked(savesFilesNameMode == 0);
	ui->radioButtonFolderName->setChecked(savesFilesNameMode == 1);

    int brushThickness = DEFAULT_BRUSH_THICKNESS;
    ui->radioButtonThin->setChecked(brushThickness == 0);
    ui->radioButtonThick->setChecked(brushThickness == 1);

	QSettings settings;
#ifdef ENABLE_PYTHON
    restorePython();
#endif
#ifdef ENABLE_DEEPLEARNING
    resetDeepLearningDirToDefault();
#endif

    // reset keyboard shortcuts
	settings.beginGroup("Shortcuts");
	settings.remove("");
	m_bChangedShortcuts = false;
	// reset tree widgets and actions
	removeCustomShortcuts( ui->treeWidget->invisibleRootItem() );
	// dicom port
	ui->spinBoxDicomPort->setValue(DEFAULT_DICOM_PORT);
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
    if (0==m->objectName().compare("menuRecent",Qt::CaseInsensitive) || m->objectName().isEmpty()) // ignore "Recent" and "empty" menu
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
            {
				ui->lineEditShortCut->setText(it->text(1));
                if (shortcutUsedCount(it->text(1))>1 && !it->text(1).isEmpty())
                    ui->lineEditShortCut->setStyleSheet("color: #ff6060");
                else
                    ui->lineEditShortCut->setStyleSheet("");
            }
		}
	}		
}

void CPreferencesDialog::on_pushButtonSetShortcut_clicked()
{
	QList<QTreeWidgetItem *> selItems = ui->treeWidget->selectedItems();
	if (!selItems.empty())
	{
        QString wsShortcut = ui->lineEditShortCut->text();
        // clear existing shortcut use
        if (shortcutUsedCount(wsShortcut, true)>0)
        {
            int retval = QMessageBox::question(this,QCoreApplication::applicationName(),tr("Shortcut already used! Do you want to clear existing shortcut?"),QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,QMessageBox::Yes);
            if (QMessageBox::Cancel==retval)
                return;
            if (QMessageBox::Yes==retval)
            {
                // clear any existing matching shortcut
                QList<QTreeWidgetItem*> lst = ui->treeWidget->findItems(wsShortcut,Qt::MatchExactly|Qt::MatchRecursive,1);
                foreach(QTreeWidgetItem* item, lst)
                {
                    if (0==item->childCount())
                    {                
                        QAction* pAct=(QAction*)item->data(1,Qt::UserRole).value<void*>();
                        if (NULL!=pAct)
                        {
                            m_bChangedShortcuts = true;
                            QKeySequence ks;
                            pAct->setShortcut(ks);
                            pAct->setProperty("Shortcut","Custom");
                            item->setText(1,ks.toString(QKeySequence::NativeText));
                        }
                    }                
                }
            }
        }
        // set shortcut
		foreach(QTreeWidgetItem *it, selItems)
		{
			if (0==it->childCount())
			{				
				QAction* pAct=(QAction*)it->data(1,Qt::UserRole).value<void*>();
				if (NULL!=pAct)
				{
					m_bChangedShortcuts = true;
					QKeySequence ks(wsShortcut);					
					pAct->setShortcut(ks);
					pAct->setProperty("Shortcut","Custom");
					ui->lineEditShortCut->setText(ks.toString(QKeySequence::NativeText));
					it->setText(1,ks.toString(QKeySequence::NativeText));
				}
			}				
		}
        treeItemSelectionChanged();
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
        treeItemSelectionChanged();
	}	
}

int CPreferencesDialog::shortcutUsedCount(const QString& shortcut, bool bIgnoreSelected)
{
    QList<QTreeWidgetItem*> lst = ui->treeWidget->findItems(shortcut,Qt::MatchExactly|Qt::MatchRecursive,1);
    int cnt = lst.size();
    if (cnt>0 && bIgnoreSelected)
    {
        QList<QTreeWidgetItem *> selItems = ui->treeWidget->selectedItems();
        if (!selItems.isEmpty())
        {
            lst.removeAll(selItems.front());        
            cnt = lst.size();
        }
    }
    {
        QList<QShortcut*> lstSct = MainWindow::getInstance()->findChildren<QShortcut*>();
        foreach(QShortcut* pSct, lstSct)
        {
            QString wsKey = pSct->key().toString();
            if (0==shortcut.compare(wsKey,Qt::CaseInsensitive))
                cnt++;
        }
    }
    return cnt;
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
			if (shortcutUsedCount(ui->lineEditShortCut->text(), true) > 0)
				ui->lineEditShortCut->setStyleSheet("color: #ff0000");
			else
				ui->lineEditShortCut->setStyleSheet("");
			return true;
        }
        return false;
    }
    return QDialog::eventFilter(obj, event);
}

#ifdef ENABLE_DEEPLEARNING

void CPreferencesDialog::resetDeepLearningDirToDefault()
{
    QSettings settings;
    settings.beginGroup("DeepLearning");
    const QString deepLearningDefaultDir = QCoreApplication::applicationDirPath() + "/deeplearning";
    ui->eFPathDeepLearning->setText(deepLearningDefaultDir);
    settings.remove("deepLearningDir2");
}

void CPreferencesDialog::showFileDialogDeepLearning()
{
    QSettings settings;
    settings.beginGroup("DeepLearning");
    const QString previousDir = settings.value("deepLearningDir2").toString();
    
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    
    QString pathToSelectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), previousDir);
    if (!pathToSelectedFolder.isEmpty())
    {
        ui->eFPathDeepLearning->setText(pathToSelectedFolder);

        if (!VPL_SIGNAL(SigHasCnnModels).invoke2(pathToSelectedFolder))
        {
            MessageWindow(QMessageBox::Warning, tr("CNN models were not found in selected folder.")).show();
        }

        if(!VPL_SIGNAL(SigHasLandmarksDefinition).invoke2(pathToSelectedFolder))
        {
            MessageWindow(QMessageBox::Warning, tr("Landmarks definition was not found in selected folder.")).show();
        }
    }

    settings.setValue("deepLearningDir2", ui->eFPathDeepLearning->text());
}
#endif

#ifdef ENABLE_PYTHON

void CPreferencesDialog::showFileDialogVPLSwig()
{
    pythonQt::Settings& pySettings = VPL_SINGLETON(pythonQt::Settings);
	const QString previousVplSwig = pySettings.getVplSwigPath();

    //show file dialog, which allows the user to select a folder
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);

    //get path to selected folder
    QString path = dialog.getExistingDirectory(this, tr("Select Directory with VPLSwig package"), previousVplSwig);

    //show that path in ui
    if (!path.isEmpty())
    {
		ui->eFPathVPLSwig->setText(path);
    }
}
//! Display directory open dialog when button for searching external python is clicked.
void CPreferencesDialog::showFileDialogPython()
{

    pythonQt::Settings& pySettings = VPL_SINGLETON(pythonQt::Settings);
	const QString previousDir = pySettings.getExternalPythonPath();


    //show file dialog, which allows the user to select a folder
	QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);

    //get path to selected folder
    QString path = dialog.getExistingDirectory(this, tr("Select Directory"), previousDir);

    //show that path in ui
    if (!path.isEmpty())
    {
		if (!data::Interpret::isExistInterpret(path))
		{
			//MessageWindow(tr("Selected path does not contain python.exe.")).show();
            MessageWindow(QMessageBox::Warning, tr("External interpret was not found in selected folder. Select another folder or switch to internal interpret.")).show();
		}
        else
        {
            ui->eFPathPythonPath->setText(path);
        }
    }
}

void CPreferencesDialog::pythonTypeChanged(int index)
{
}

void CPreferencesDialog::showHidePythonOptions(int state)
{
#ifdef ENABLE_PYTHON
    pythonQt::Settings& pySettings = VPL_SINGLETON(pythonQt::Settings);

    if (state == Qt::Checked)
    {
        bool existInterpret = false;
        bool oldState = ui->comboBoxPythonType->blockSignals(true);

        // If is selected internal interpret and not exist,
        // than check external interpret and if exist set it.
        if (pySettings.isSelectedInternalInterpret())
        {
            existInterpret = data::Interpret::isExistInterpret(data::InterpretType::INTERNAL);
            if (!existInterpret)
            {
                if (data::Interpret::isExistInterpret(data::InterpretType::EXTERNAL))
                {
                    MessageWindow(QMessageBox::Warning, tr("Internal interpret was not found, changing to external.")).show();
                    existInterpret = true;
                    ui->comboBoxPythonType->setCurrentIndex(1);
                }
            }
        }
        // If is selected external interpret and not exist,
        // than check internal interpret and if exist set it.
        else
        {
            existInterpret = data::Interpret::isExistInterpret(data::InterpretType::EXTERNAL);
            if (!existInterpret)
            {
                if (data::Interpret::isExistInterpret(data::InterpretType::INTERNAL))
                {
                    MessageWindow(QMessageBox::Warning, tr("External interpret was not found, changing to internal.")).show();
                    existInterpret = true;
                    ui->comboBoxPythonType->setCurrentIndex(0);

                }
            }
        }
        ui->comboBoxPythonType->blockSignals(oldState);


        if (existInterpret)
        {
            ui->eFBrowseVPLSwig->setEnabled(true);
            ui->eFBrowsePythonPath->setEnabled(true);
            ui->comboBoxPythonType->setEnabled(true);
            m_bReinitializeInterpret = true;
        }
        else
        {
            //ui->checkBoxEnablePython->toggle();
            ui->eFBrowseVPLSwig->setEnabled(false);
            ui->comboBoxPythonType->setEnabled(false);
            //ui->checkBoxEnablePython->setChecked(false);
            MessageWindow(tr("Internal/External interpret path does not contain python.exe.")).show();
        }
    }
    else
    {
        ui->eFBrowseVPLSwig->setEnabled(false);
        ui->comboBoxPythonType->setEnabled(false);
        //ui->checkBoxEnablePython->setChecked(false);
    }
#endif
}

#endif
