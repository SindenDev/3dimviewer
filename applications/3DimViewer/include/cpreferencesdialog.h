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

#ifndef CPREFERENCESDIALOG_H
#define CPREFERENCESDIALOG_H

#include <QDialog>
#include <QDir>

#define DEFAULT_BACKGROUND_COLOR        qRgb(51,51,102) 
#define DEFAULT_LOGGING                 true
#define DEFAULT_MODEL_REGION_LINK		false
#define DEFAULT_SAVE_PATH_MODE          0
#define DEFAULT_STYLESHEET              ""

namespace Ui {
class CPreferencesDialog;
}

//! Preferences dialog - language, threading etc
class CPreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    //! Constructor
    explicit CPreferencesDialog(const QDir &localeDir, QWidget *parent = 0, Qt::WindowFlags f = 0);

    //! Destructor
    ~CPreferencesDialog();

    //! Returns true when a setting which requires application restart has changed
    bool needsRestart() { return m_bChangesNeedRestart; }

    //! Returns true when background color changed
    bool colorsChanged() const { return m_bColorsChanged; }
    
private slots:
    void on_CPreferencesDialog_accepted();
    void on_buttonBGColor_clicked();
    void resetDefaultsPressed( );  
private:
    Ui::CPreferencesDialog *ui;
    bool                    m_bColorsChanged,
                            m_bChangesNeedRestart;
    QColor                  m_bgColor;
    void    setButtonColor(QColor& targetColor, const QColor& sourceColor, QPushButton *targetButton);
};

#endif // CPREFERENCESDIALOG_H
