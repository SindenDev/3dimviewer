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

#ifndef CEVENTFILTER_H
#define CEVENTFILTER_H

#include <QEvent>
#include <QObject>
#include <QString>
#include <QApplication>
#include <QFile>
#include <QAction>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QTime>

class CEventFilter;

class IHasEventFilter
{
public:
    virtual const CEventFilter &getEventFilter() const = 0;
};

//! Event filter - filter and record ui events (mouse events, keyboard events)
class CEventFilter: public QObject
{
	Q_OBJECT

public:
	//! Constructor
	CEventFilter();

	//! Destructor
    ~CEventFilter();

	//! Catches events and processes them
	bool eventFilter(QObject *obj, QEvent *ev);

	//! Start or stop event recording
	void processSettings()
	{
		loadSettings();
		(m_state) ? start() : stop();
	};

    //! Log to a new file
    void startNewLogFile()
    {
        if (m_state)
            start();
        else
            stop();
    }

public slots:

	//! Every menu action is connected to this slot and processed here
    void catchMenuAction();

	//! Catches plugin menu buttons clicks
	void catchPluginMenuAction(QAction *a);

	//! Catches wizard buttons clicks
	void processWizardButtons();

	//! Shows content of output file
	void showEventFilterLog();

	//! Catch clicks on icon buttons (TT)
	void catchColorWidgetClicked(double x, double y, bool bTimer);
	
private:

// methods

	//! Start event recording
	void start();

	//! Stop event recording
	void stop();

	//! Write custom event to output file
	void customEventHandler(const QString &classString, const QString &nameString, const QString &textString, const QString &pathString);

	//! Creates and opens output file in selected directory (by the user)
    void openOutputFile();

	//! Processes mouse events
    void processMouseEvent(QEvent &ev, QObject &obj);

	//! Processes mouse wheel events
    void processWheelEvent(QEvent &ev, QObject &obj);

	//! Processes keyboard events
    void processKeyboardEvent(QEvent &ev, QObject &obj);

	//! Writes wheel event to output file
    void writeWheelEventToFile();

	//! Writes keyboard event to output file
	void writeKeyEventToFile(QKeyEvent &ke, QObject &obj);

	//! Gets key text if it is possible or converts special keys to QString and returns it (or returns empty string)
    QString getKeyText(QKeyEvent &e);

	//! Returns label (or text) of object (if it is possible) or empty string
    QString getObjectLabel(QObject &obj);

	//! Closes output file
    void closeOutputFile();

	//! Returns path to object (hierarchy of widgets)
	QString objectPath(QObject *obj);

	//! Get last two click event times and check if it was double click
	bool checkDoubleClick(QEvent &ev, QObject &obj);

	//! Write mouse event to output file and do some actions based on the type of event (double click or drag)
	void writeMouseEvent(bool isDbl, bool isDrag);

	//! Load stuff from QSettings
	void loadSettings();

	//! Add class name to supported classes list (or remove it, when unchecked)
	void setSupportedClasses(QString className, int checked);

	//! Writes menu action to output file (processes menu clicks and shortcuts)
	template <class C>
	void processMenuAction(C &a);

	//! Writes unwritten mouse event to output file.
	inline void checkMouseEvent()
	{
		if (!m_prevMouseEvent.isEmpty())
		{
			QTextStream out(m_outputFile);
			out.setCodec("UTF-8");
			out << m_prevMouseEvent;
			m_prevMouseEvent = "";
		}
	};

	//! Writes unwritten wheel event to output file.
	inline void checkWheelEvent()
	{
		if (m_rollCount != 0)
			writeWheelEventToFile();
	};

// variables

	//! Mouse event filtering on/off
    bool m_mouseEnabled;

	//! Keyboard event filtering on/off
    bool m_keyboardEnabled;

	//! Custom event filtering on/off
	bool m_customEnabled;

	//! Event filtering on/off
    bool m_state;

	//! Path to folder, where output file will be located
    QString m_path;

	//! Pointer to output file
    QFile *m_outputFile;

	//! Position of previous mouse event (used to handle drags)
    QPoint m_prevMousePos;

	//! Previous mouse button
	QString m_prevMouseButton;

	//! Number of same wheel events (used to write only one wheel event for the same consecutive wheel events)
    int m_rollCount;

	//! Wheel event is stored here and is written to the output file when another event occures (because the end of wheel events is unknown until another event occures)
    QString m_prevWheelEvent;

	//! Path to menu item (hierarchy of widgets)
    QString m_menuPath;

	//! Stores text of keys, which were pressed after a special key (Ctrl, Shift, ...) (it is stored here too)
	QStringList m_specialEventKeySequence;

	//! List of supported class names
	QStringList m_supportedClasses;

	//! String with special event (like Ctrl+..., Shift+..., Alt+...)
	QString m_specialEvent;

	//! Text of last pressed key (used to handle key hold)
	QString m_lastPressedKey;

	//! Time of last key press (used to handle key hold)
	QTime m_lastTime;

	//! True if user is holding a key
	bool m_keyHold;

	//! String with previous mouse event (used to handle double clicks)
	QString m_prevMouseEvent;

	//! String with current mouse event (used to handle double clicks)
	QString m_currentMouseEvent;

	//! True if shortcut was raised
	bool m_shortcutRaised;

	//! True if shortcut was processed and another event occured
	bool m_shortcutProcessed;
};

#endif // CEVENTFILTER_H
