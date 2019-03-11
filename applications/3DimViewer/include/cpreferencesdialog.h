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

#ifndef CPREFERENCESDIALOG_H
#define CPREFERENCESDIALOG_H

#include <QDialog>
#include <QDir>
#include <QMenuBar>
#include <QTreeWidgetItem>
#include <QProxyStyle>
#include <QDesktopWidget>

#include <actlog/ceventfilter.h>

#define DEFAULT_BACKGROUND_COLOR        qRgb(51,51,102) 
#define DEFAULT_LOGGING                 true
#define DEFAULT_LOG_OPENGL_ERRORS       false
#define DEFAULT_MODEL_REGION_LINK		true
#define DEFAULT_SAVE_PATH_MODE          0
#define DEFAULT_SAVED_FILES_NAME_MODE	0
#define DEFAULT_BRUSH_THICKNESS         0
#define DEFAULT_STYLESHEET              ""
#define DEFAULT_DICOM_PORT              5678
#define DEFAULT_BIG_ICONS				true
#define DEFAULT_ANTIALIASING            true

// Proxy style for custom icon size
class BigIconsProxyStyle : public QProxyStyle
{
protected:
    bool m_bBigIcons;
public:
    //! Constructor
    BigIconsProxyStyle(bool bBigIcons) : QProxyStyle() { m_bBigIcons = bBigIcons; }
    //! Overloaded pixel metrix method to adjust icon size
    int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const
    {
        int s = QProxyStyle::pixelMetric(metric, option, widget);
        if (m_bBigIcons)
        {            
            if (metric == QStyle::PM_SmallIconSize)
            { // used by menu
                s = 24;
            }
            if (metric == QStyle::PM_ToolBarIconSize)
            {
                s = 32;
            }
			// adjust for dpi
			if (metric==QStyle::PM_ToolBarIconSize)
			{
				QDesktopWidget* pDesktop=QApplication::desktop();
				if (NULL!=pDesktop && pDesktop->logicalDpiX()>100)
				{
					double dpiFactor = pDesktop->logicalDpiX()/96.0;
					dpiFactor = 1 + (dpiFactor-1)/2; // don't scale directly because we don't have that good icons
					s = (int)(s*dpiFactor);
				}
			}
        }
        return s;
    }
};


namespace Ui {
class CPreferencesDialog;
}

//! Preferences dialog - language, threading etc
class CPreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    //! Constructor
	explicit CPreferencesDialog(const QDir &localeDir, QMenuBar* pMenuBar, CEventFilter &eventFilter, QWidget *parent = 0, Qt::WindowFlags f = 0);

    //! Destructor
    ~CPreferencesDialog();

    //! Returns true when a setting which requires application restart has changed
    bool needsRestart() { return m_bChangesNeedRestart; }

    //! Returns true when background color changed
    bool colorsChanged() const { return m_bColorsChanged; }

	//! Returns true when some keyboard shortcut has changed
	bool shortcutsChanged() const { return m_bChangedShortcuts; }	
    
private slots:
    void on_CPreferencesDialog_accepted();
    void on_buttonBGColor_clicked();
    void resetDefaultsPressed( );  
	void pageChange(int index);
	void treeItemSelectionChanged();
	void on_pushButtonSetShortcut_clicked();
	void on_pushButtonClearShortcut_clicked();
	bool eventFilter(QObject* obj, QEvent *event);

#ifdef ENABLE_PYTHON
    void pythonTypeChanged(int index);
    void showFileDialogVPLSwig();
    void showFileDialogPython();
    void showHidePythonOptions(int state);
    void restorePython();

#endif
#ifdef ENABLE_DEEPLEARNING
    void resetDeepLearningDirToDefault();
    void showFileDialogDeepLearning();
#endif
    void showFileDialog();
    void showHideFilterOptions(int state);
	void setMonitoredObjectsChecked();

private:
    Ui::CPreferencesDialog *ui;
    bool                    m_bColorsChanged,
                            m_bChangesNeedRestart,
							m_bChangedShortcuts;
    QColor                  m_bgColor;
    void    setButtonColor(QColor& targetColor, const QColor& sourceColor, QPushButton *targetButton);

	QTreeWidgetItem* addTreeRoot(QString name, QString description);
	QTreeWidgetItem * addTreeChild(QTreeWidgetItem *parent, QString name, QString description, QAction* pAction);
	QTreeWidgetItem* addTreeMenu(QMenu* pMenu, QTreeWidgetItem *parent);
	int shortcutUsedCount(const QString& shortcut, bool bIgnoreSelected = false);
	void removeCustomShortcuts( QTreeWidgetItem *item );

    CEventFilter &m_eventFilter;

    bool m_bReinitializeInterpret;
};

#endif // CPREFERENCESDIALOG_H
