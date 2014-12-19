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

CPreferencesDialog::CPreferencesDialog(const QDir &localeDir, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::CPreferencesDialog)
{
    m_bColorsChanged = false;
    m_bChangesNeedRestart = false;
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
    ui->listPages->setCurrentRow(0);
}

CPreferencesDialog::~CPreferencesDialog()
{
    delete ui;
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

void CPreferencesDialog::resetDefaultsPressed( )
{
    // set threading to single threaded
    ui->comboBoxRenderingMode->setCurrentIndex(0);
    // set error logging
    ui->checkBoxLogging->setChecked(DEFAULT_LOGGING);
    // set background color
    m_bgColor=DEFAULT_BACKGROUND_COLOR;
    setButtonColor(m_bgColor, m_bgColor, ui->buttonBGColor);
    //
    int savePathMode = DEFAULT_SAVE_PATH_MODE;
    ui->radioButtonPathLastUsed->setChecked(savePathMode==0);
    ui->radioButtonPathProject->setChecked(savePathMode==1);
}