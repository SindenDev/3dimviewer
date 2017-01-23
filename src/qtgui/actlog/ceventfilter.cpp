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

///////////////////////////////////////////////////////////////////////////////
// KNOWN PROBLEMS:
// o there is no double click in special event (Ctrl+..., Shift+..., Alt+...)
// o there is no double click with middle mouse button
// o when you press shortcut, which raises a dialog window (e.g. Ctrl+O), there is no Crtl release after it,
//	 so the next Ctrl+... shortcut, which is not connected to any action is handled without the Ctrl (others are handled correctly)
//	 for example: you press Ctrl+O, then Ctrl+d (not connected to any action) -> only keyboard event witk 'd' key is written to output
///////////////////////////////////////////////////////////////////////////////

#include <actlog/ceventfilter.h>
#include "mainwindow.h" // I really don't like this!

#include <qtgui/app/Signals.h>

#include <QWidget>
#include <QDebug>
#include <QDateTime>
#include <QSpinBox>
#include <QLineEdit>
#include <QContextMenuEvent>
#include <QTabBar>
#include <QFileInfo>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QSettings>
#include <QDir>
#include <QDialog>
#include <QLabel>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

CEventFilter::CEventFilter()
{
	// initialize variables
    m_outputFile = NULL;
    m_rollCount = 0;
    m_prevWheelEvent = "";
	m_specialEvent = "";
	m_lastPressedKey = "";
	m_prevMouseEvent = "";
	m_currentMouseEvent = "";
	m_keyHold = false;
	m_shortcutRaised = false;
	m_shortcutProcessed = true;

	// connect custom event
	VPL_SIGNAL(SigLogCustomEvent).connect(this, &CEventFilter::customEventHandler);
}

CEventFilter::~CEventFilter()
{
    closeOutputFile();
}

void CEventFilter::loadSettings()
{
	QSettings settings;

	m_state = settings.value("logEnabled", true).toBool();

	m_mouseEnabled = settings.value("logMouseEnabled", true).toBool();
	m_keyboardEnabled = settings.value("logKeyboardEnabled", true).toBool();
	m_customEnabled = settings.value("logCustomEnabled", true).toBool();

	setSupportedClasses("QMenu", settings.value("logMenu", true).toBool());
	setSupportedClasses("QPushButton", settings.value("logButtons", true).toBool());
	setSupportedClasses("QToolButton", settings.value("logButtons", true).toBool());
	setSupportedClasses("ColorWidget", settings.value("logButtons", true).toBool());
	setSupportedClasses("QSpinBox", settings.value("logSpinBoxes", true).toBool());
	setSupportedClasses("QDoubleSpinBox", settings.value("logSpinBoxes", true).toBool());
	setSupportedClasses("QCheckBox", settings.value("logCheckBoxes", true).toBool());
	setSupportedClasses("QRadioButton", settings.value("logRadioButtons", true).toBool());
	setSupportedClasses("QPlainTextEdit", settings.value("logTextEdits", true).toBool());
	setSupportedClasses("QLineEdit", settings.value("logTextEdits", true).toBool());
	setSupportedClasses("QComboBox", settings.value("logComboBoxes", true).toBool());
	setSupportedClasses("QSlider", settings.value("logSliders", true).toBool());
	setSupportedClasses("CSliderEx", settings.value("logSliders", true).toBool());
	setSupportedClasses("CVolumeRendererWindow", settings.value("logOsgWindows", true).toBool());
	setSupportedClasses("OSGOrtho2DCanvas", settings.value("logOsgWindows", true).toBool());
	setSupportedClasses("QTabBar", settings.value("logTabBars", true).toBool());
	setSupportedClasses("QListWidget", settings.value("logLists", true).toBool());
	setSupportedClasses("QTableWidget", settings.value("logTables", true).toBool());

#if QT_VERSION < 0x050000
    QString homeLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString homeLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
#endif
    if (!homeLocation.isEmpty())
    {
        QDir d(homeLocation);
        d.mkdir("Traumatech");
        d.cd("Traumatech");
        homeLocation = d.absolutePath();
    }
    m_path = settings.value("logOuputDir", homeLocation).toString();
}

void CEventFilter::setSupportedClasses(QString className, int checked)
{ 
	// if checked, add class name to supported classes, remove it otherwise
	if (checked)
	{
		if (!m_supportedClasses.contains(className))
		{
			m_supportedClasses.append(className);
		}
	}
	else
	{
		m_supportedClasses.removeAt(m_supportedClasses.indexOf(className));
	}
};


void CEventFilter::openOutputFile()
{
	// check if selected path is path to a folder
    QFileInfo pathInfo(m_path);
    if (pathInfo.isDir())
    {
		// add filename, which is current timestamp
        QString outputPath = m_path;
        outputPath += "/";
        outputPath += QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");
        outputPath += ".xml";

		// create output file and open it
        m_outputFile = new QFile(outputPath);
        if (!m_outputFile->open(QIODevice::ReadWrite | QIODevice::Text))
        {
            delete m_outputFile;
            return;
        }

		QTextStream out(m_outputFile);
		out.setCodec("UTF-8");

		// write main xml element to it
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		out << "<eventFilter windowSize=\"" + QString::number(MainWindow::getInstance()->frameSize().width()) + ":" + QString::number(MainWindow::getInstance()->frameSize().height()) + "\">\n";
    }
}

void CEventFilter::closeOutputFile()
{
	// check if file is created and opened
    if (m_outputFile != NULL)
    {
        if (m_outputFile->isOpen())
        {
			checkWheelEvent();
			checkMouseEvent();

			// write the end of main xml element and close the file    
			QTextStream out(m_outputFile);
			out.setCodec("UTF-8");
            out << "</eventFilter>\n";
            m_outputFile->close();
        }

        delete m_outputFile;
        m_outputFile = NULL;
    }
}

void CEventFilter::start()
{
	// file is not created, create and open new one
	if (m_outputFile == NULL)
	{
		openOutputFile();
	}

	// or check if the path to output folder was changed, if so, close current output file and create new one in selected folder
    else
    {
        QFileInfo fi(*m_outputFile);
        if (fi.absolutePath() != m_path)
        {
            closeOutputFile();
            openOutputFile();
        }
    }

	// start catching events
    qApp->installEventFilter(this);
}

void CEventFilter::stop()
{
    closeOutputFile();
    qApp->removeEventFilter(this);
}

QString CEventFilter::objectPath(QObject *obj)
{
	// go thru object parent hierarchy and save it to string
	QString path;

	for(; obj; obj = obj->parent())
	{
		if (!path.isEmpty())
		{
			path.prepend("/");
		}

        path.prepend(obj->objectName());
	}

    return path;
}

void CEventFilter::customEventHandler(const QString &classString, const QString &nameString, const QString &textString, const QString &pathString)
{
	checkWheelEvent();
	checkMouseEvent();

    if (!m_customEnabled || NULL == m_outputFile)
	{
		return;
	}

	// write custom event to the file
	QTextStream out(m_outputFile);
	out.setCodec("UTF-8");
	out << "\t<event type=\"customevent\">\n";
	out << "\t\t<time>" << QTime::currentTime().toString("hh:mm:ss:zzz") << "</time>\n";
	out << "\t\t<objectClass>" << classString << "</objectClass>\n";
	out << "\t\t<objectName>" << nameString << "</objectName>\n";
	out << "\t\t<objectText>" << textString << "</objectText>\n";
	out << "\t\t<objectPath>" << pathString << "</objectPath>\n";
	out << "\t</event>\n";
}

QString CEventFilter::getObjectLabel(QObject &obj)
{
	// get label (or text) of object
	// it is not possible for all classes

    QString className = obj.metaObject()->className();
	QString text = "";

    if (className == "QPushButton")
    {
        QPushButton* pb = qobject_cast<QPushButton*>(&obj);

		if (pb)
		{
			text = pb->text();
		}
    }
    /*else if (className == "QComboBox") // doesnt work, gets text, which is in combo box before event
    {
        QComboBox* cb = qobject_cast<QComboBox*>(obj);
        if (cb)
		{
			text = cb->currentText();
		}
    }*/
    else if (className == "QRadioButton")
    {
        QRadioButton* rb = qobject_cast<QRadioButton*>(&obj);

		if (rb)
		{
			text = rb->text();
		}
    }
    else if (className == "QCheckBox")
    {
        QCheckBox* cb = qobject_cast<QCheckBox*>(&obj);

		if (cb)
		{
			text = cb->text();
		}
    }

	if (text.contains('&'))
	{
		return "";
	}

    return text;
}

bool CEventFilter::checkDoubleClick(QEvent &ev, QObject &obj)
{
	// get event time from previous mouse event and compare it with time from current mouse event
	// if there is less then 300 ms diference, it is double click
	if (!m_prevMouseEvent.isEmpty())
	{
		QMouseEvent *me = static_cast<QMouseEvent *>(&ev);

		if (me)
		{
			QString prevTime = m_prevMouseEvent.mid(m_prevMouseEvent.indexOf("<time>") + 6, 12);
			QString currentTime = m_currentMouseEvent.mid(m_currentMouseEvent.indexOf("<time>") + 6, 12);

			if (QTime::fromString(currentTime, "hh:mm:ss:zzz") < QTime::fromString(prevTime, "hh:mm:ss:zzz").addMSecs(300))
			{
				return true;
			}
		}
	}

	return false;
}

void CEventFilter::writeMouseEvent(bool isDbl, bool isDrag)
{
	QTextStream out(m_outputFile);
	out.setCodec("UTF-8");

	// check if mouse event is drag or double click and decide what to do
	if (!isDrag)
	{
		if (isDbl)
		{
			out << m_currentMouseEvent;
			m_currentMouseEvent = "";
			m_prevMouseEvent = "";
		}
		else if (m_prevMouseEvent.isEmpty())
		{
			m_prevMouseEvent = m_currentMouseEvent;
			m_currentMouseEvent = "";
		}
		else
		{
			out << m_prevMouseEvent;
			m_prevMouseEvent = m_currentMouseEvent;
			m_currentMouseEvent = "";
		}
	}
	else
	{
		if (!m_prevMouseEvent.isEmpty())
		{
			out << m_prevMouseEvent;
		}

		out << m_currentMouseEvent;
		m_currentMouseEvent = "";
		m_prevMouseEvent = "";
	}
}

void CEventFilter::processMouseEvent(QEvent &ev, QObject &obj)
{
	checkWheelEvent();

	// check if object class is in supported classes
	if (!m_supportedClasses.contains(QString(obj.metaObject()->className())))
	{
		return;
	}

	// if event occured on menu item, save object path
    if (QString(obj.metaObject()->className()) == "QMenu" || QString(obj.metaObject()->className()) == "QToolButton")
    {
        m_menuPath = objectPath(&obj);
        return;
    }

    QMouseEvent *me = static_cast<QMouseEvent *>(&ev);

	if (NULL == me)
	{
		return;
	}

	// if event is mouse press, save position
    if (ev.type() == QEvent::MouseButtonPress)
    {
        m_prevMousePos = me->globalPos() - MainWindow::getInstance()->pos();
		m_prevMouseButton = QString(((me->button() == Qt::LeftButton) ? "left" : ((me->button() == Qt::RightButton) ? "right" : "middle")));

		// if it is special event (special key is pressed), add mousePressed to key sequence
		if (!m_specialEventKeySequence.isEmpty())
		{
			m_specialEventKeySequence.append("mousePressed");
		}
    }
    else //mouseRelease
    {
		QPoint pos = me->globalPos() - MainWindow::getInstance()->pos();

		int distance = (int)sqrt(pow(pos.x() - m_prevMousePos.x(), 2.0) + pow(pos.y() - m_prevMousePos.y(), 2.0));

		// special event - add mouse action to it
		if (!m_specialEventKeySequence.isEmpty() && m_specialEventKeySequence.contains("mousePressed"))
		{
			m_specialEventKeySequence.removeAt(m_specialEventKeySequence.indexOf("mousePressed"));

			m_specialEvent += "\t\t\t<mouse>\n";
			m_specialEvent += "\t\t\t\t<mouseButton>" + QString(((me->button() == Qt::LeftButton) ? "left" : ((me->button() == Qt::RightButton) ? "right" : "middle"))) + "</mouseButton>\n";

			if (distance < 3) //click
			{
				m_specialEvent += "\t\t\t\t<action>click</action>\n";
				m_specialEvent += "\t\t\t\t<position>" + QString::number(pos.x()) + ":" + QString::number(pos.y()) + "</position>\n";
			}
			else //drag
			{
				// if event is mouse drag, write two positions
				m_specialEvent += "\t\t\t\t<action>drag</action>\n";
				m_specialEvent += "\t\t\t\t<startPosition>" + QString::number(m_prevMousePos.x()) + ":" + QString::number(m_prevMousePos.y()) + "</startPosition>\n";
				m_specialEvent += "\t\t\t\t<endPosition>" + QString::number(pos.x()) + ":" + QString::number(pos.y()) + "</endPosition>\n";
			}

			m_specialEvent += "\t\t\t</mouse>\n";

			// if all special event keys was processed, write it to output file
			if (m_specialEventKeySequence.isEmpty())
			{
				QTextStream out(m_outputFile);
				out.setCodec("UTF-8");
				out << m_specialEvent + "\t\t</actionsSequence>\n\t</event>\n";
				m_specialEvent = "";
			}

		}
		else // normal event
		{
			bool dbl = false;	
			bool drag = false;

			m_currentMouseEvent += "\t<event type=\"mouseevent\">\n";
			m_currentMouseEvent += "\t\t<time>" + QTime::currentTime().toString("hh:mm:ss:zzz") + "</time>\n";
			m_currentMouseEvent += "\t\t<objectClass>" + QString(obj.metaObject()->className()) + "</objectClass>\n";
			m_currentMouseEvent += "\t\t<objectName>" + obj.objectName() + "</objectName>\n";

			// if object is QTabBar, get selected tab text
			if (QString(obj.metaObject()->className()) == "QTabBar")
			{
				QTabBar* tb = qobject_cast<QTabBar*>(&obj);

				if (tb)
				{
					m_currentMouseEvent += "\t\t<objectText>" + tb->tabText(tb->currentIndex()) + "</objectText>\n";
				}
				else
				{
					m_currentMouseEvent += "\t\t<objectText></objectText>\n";
				}
			}
			else
			{
				m_currentMouseEvent += "\t\t<objectText>" + getObjectLabel(obj) + "</objectText>\n";
			}

			m_currentMouseEvent += "\t\t<objectPath>" + objectPath(&obj) + "</objectPath>\n";
			m_currentMouseEvent += "\t\t<mouseButton>" + QString((me->button() == Qt::LeftButton) ? "left" : ((me->button() == Qt::RightButton) ? "right" : "middle")) + "</mouseButton>\n";

			if (distance < 3) //click
			{
				if (checkDoubleClick(ev, obj))
				{
					dbl = true;
					m_currentMouseEvent += "\t\t<action>doubleclick</action>\n";
					m_currentMouseEvent += "\t\t<position>" + QString::number(pos.x()) + ":" + QString::number(pos.y()) + "</position>\n";
				}
				else
				{
					m_currentMouseEvent += "\t\t<action>click</action>\n";
					m_currentMouseEvent += "\t\t<position>" + QString::number(pos.x()) + ":" + QString::number(pos.y()) + "</position>\n";
				}
			}
			else //drag
			{
				drag = true;

				// if event is mouse drag, write two positions
				m_currentMouseEvent += "\t\t<action>drag</action>\n";
				m_currentMouseEvent += "\t\t<startPosition>" + QString::number(m_prevMousePos.x()) + ":" + QString::number(m_prevMousePos.y()) + "</startPosition>\n";
				m_currentMouseEvent += "\t\t<endPosition>" + QString::number(pos.x()) + ":" + QString::number(pos.y()) + "</endPosition>\n";
			}

			m_currentMouseEvent += "\t</event>\n";

			writeMouseEvent(dbl, drag);
		}
    }
}

void CEventFilter::processWheelEvent(QEvent &ev, QObject &obj)
{
	checkMouseEvent();

	// check if object class is in supported classes
	if (!m_supportedClasses.contains(QString(obj.metaObject()->className())))
	{
		return;
	}

    QWheelEvent *we = static_cast<QWheelEvent *>(&ev);

	if (NULL == we)
	{
		return;
	}

	// count the same wheel events (all rollups, all rolldowns), when the other event occures, write the previous event to file and reset counting
    if (m_rollCount == 0 && we->delta() > 0)
    {
        m_rollCount++;
    }
    else if (m_rollCount == 0 && we->delta() < 0)
    {
        m_rollCount--;
    }
    else if (m_rollCount > 0 && we->delta() > 0)
    {
        m_rollCount++;
    }
    else if (m_rollCount < 0 && we->delta() < 0)
    {
        m_rollCount--;
    }
    else if (m_rollCount < 0 && we->delta() > 0)
    {
	    writeWheelEventToFile();
        m_rollCount++;
    }
    else if (m_rollCount > 0 && we->delta() < 0)
    {
        writeWheelEventToFile();
        m_rollCount--;
    }

	// store wheel event to string
    m_prevWheelEvent = "";

    QString className = obj.metaObject()->className();

    m_prevWheelEvent += "\t<event type=\"wheelevent\">\n";
    m_prevWheelEvent += "\t\t<time>" + QTime::currentTime().toString("hh:mm:ss:zzz") + "</time>\n";
    m_prevWheelEvent += "\t\t<objectClass>" + className + "</objectClass>\n";
    m_prevWheelEvent += "\t\t<objectName>" + obj.objectName() + "</objectName>\n";

	// if object is QTabBar, get selected tab text
    if (className == "QTabBar")
    {
        QTabBar* tb = qobject_cast<QTabBar*>(&obj);

		if (tb)
		{
			if (m_rollCount < 0)
			{
				m_prevWheelEvent += "\t\t<objectText>" + tb->tabText((tb->currentIndex() == tb->count() - 1) ? tb->currentIndex() : tb->currentIndex() + 1) + "</objectText>\n";
			}
			else
			{
				m_prevWheelEvent += "\t\t<objectText>" + tb->tabText((tb->currentIndex() == 0) ? tb->currentIndex() : tb->currentIndex() - 1) + "</objectText>\n";
			}
		}
		else
		{
			m_prevWheelEvent += "\t\t<objectText></objectText>\n";
		}
	}
	else
	{
		m_prevWheelEvent += "\t\t<objectText>" + getObjectLabel(obj) + "</objectText>\n";
	}

    m_prevWheelEvent += "\t\t<objectPath>" + objectPath(&obj) + "</objectPath>\n";
    m_prevWheelEvent += "\t\t<mouseButton>middle</mouseButton>\n";

    if (m_rollCount > 0) //rollup
    {
        m_prevWheelEvent += "\t\t<action>rollup</action>\n";
        m_prevWheelEvent += "\t\t<rollCount>" + QString::number(m_rollCount) + "</rollCount>\n";
    }
    else //rolldown
    {
        m_prevWheelEvent += "\t\t<action>rolldown</action>\n";
        m_prevWheelEvent += "\t\t<rollCount>" + QString::number(m_rollCount) + "</rollCount>\n";
    }

    m_prevWheelEvent += "\t</event>\n";
}

void CEventFilter::writeWheelEventToFile()
{
	// special event - just add wheel event to it
	if (!m_specialEventKeySequence.isEmpty())
	{
		m_specialEvent += "\t\t\t<wheel>\n";

		if (m_rollCount > 0) //rollup
		{
			m_specialEvent += "\t\t\t\t<action>rollup</action>\n";
			m_specialEvent += "\t\t\t\t<rollCount>" + QString::number(m_rollCount) + "</rollCount>\n";
		}
		else //rolldown
		{
			m_specialEvent += "\t\t\t\t<action>rolldown</action>\n";
			m_specialEvent += "\t\t\t\t<rollCount>" + QString::number(m_rollCount) + "</rollCount>\n";
		}

		m_specialEvent += "\t\t\t</wheel>\n";
	}
	else // else write it to output file
	{
		QTextStream out(m_outputFile);
		out.setCodec("UTF-8");
		out << m_prevWheelEvent;
	}

    m_rollCount = 0;
}

QString CEventFilter::getKeyText(QKeyEvent &e)
{
	// get text of pressed key
	// if e->text() is empty, it is some special key, so convert it to string
	if (e.key() == Qt::Key_Alt)
	{
		return "Alt";
	}
	else if (e.key() == Qt::Key_Control)
	{
		return "Ctrl";
	}
	else if (e.key() == Qt::Key_Shift)
	{
		return "Shift";
	}
	else if (e.key() == Qt::Key_CapsLock)
	{
		return "Caps Lock";
	}
	else if (e.key() == Qt::Key_Enter)
	{
		return "Enter";
	}
	else if (e.key() == Qt::Key_Return)
	{
		return "Return";
	}
	else if (e.key() == Qt::Key_Backspace)
	{
		return "Back Space";
	}
	else if (e.key() == Qt::Key_Up)
	{
		return "Up";
	}
	else if (e.key() == Qt::Key_Down)
	{
		return "Down";
	}
	else if (e.key() == Qt::Key_Left)
	{
		return "Left";
	}
	else if (e.key() == Qt::Key_Right)
	{
		return "Right";
	}
	else if (e.key() == Qt::Key_Delete)
	{
		return "Delete";
	}
	else if (e.key() == Qt::Key_Space)
	{
		return "Space";
	}
	else if (e.key() == Qt::Key_Tab)
	{
		return "Tab";
	}
	else if (e.key() == Qt::Key_Escape)
	{
		return "Escape";
	}
	else if (!(e.text()).isEmpty())
	{
		return  QKeySequence(e.key()).toString().toLower();
	}

    return "";
}

void CEventFilter::writeKeyEventToFile(QKeyEvent &ke, QObject &obj)
{
	QTextStream out(m_outputFile);
	out.setCodec("UTF-8");
    out << "\t<event type=\"keyboardevent\">\n";
    out << "\t\t<time>" << QTime::currentTime().toString("hh:mm:ss:zzz") << "</time>\n";
    out << "\t\t<objectClass>" << obj.metaObject()->className() << "</objectClass>\n";
    out << "\t\t<objectName>" << obj.objectName() << "</objectName>\n";
    out << "\t\t<objectText>" << getObjectLabel(obj) << "</objectText>\n";
    out << "\t\t<objectPath>" << objectPath(&obj) << "</objectPath>\n";
    out << "\t\t<key>" << getKeyText(ke) << "</key>\n";
    out << "\t</event>\n";
}

void CEventFilter::processKeyboardEvent(QEvent &ev, QObject &obj)
{
	checkWheelEvent();
	checkMouseEvent();

	// check if object class is in supported classes
	if (!m_supportedClasses.contains(QString(obj.metaObject()->className())))
	{
		return;
	}

    QKeyEvent *ke = static_cast<QKeyEvent *>(&ev);

	if (NULL == ke)
	{
		return;
	}

	QString keyText = getKeyText(*ke);

	// when key press occures, check if it is a special key
	// if so, next pressed keys will be stored to special keybord item until the initial special key is released
    if (ev.type() == QEvent::KeyPress)
    {		
		// if keyboard event are disabled, just store special key text to list (to know, that shortcut will be raised and not processed)
		if (!m_keyboardEnabled)
		{
			if (keyText == "Ctrl" || keyText == "Shift" || keyText == "Alt")
			{
				m_specialEventKeySequence.append(keyText);
			}

			return;
		}

		m_shortcutProcessed = true;

		// check if user is holding a key (dont care if object is spin box or text edit)
		if (QString("QSpinBox,QDoubleSpinBox,QLineEdit,QPlainTextEdit").contains(QString(obj.metaObject()->className())))
		{
			m_lastPressedKey = keyText;
			m_keyHold = false;
		}
		else if (m_lastPressedKey == "")
		{
			m_lastPressedKey = keyText;
			m_lastTime = QTime::currentTime();
		}
		else if ((m_lastPressedKey == keyText) && (m_lastTime.addMSecs(600) > QTime::currentTime())) // holding key
		{
			m_lastTime = QTime::currentTime();
			m_keyHold = true;
			return;
		}
		else
		{
			m_lastPressedKey = keyText;
			m_lastTime = QTime::currentTime();
			m_keyHold = false;
		}

		// special key was pressed, special event starts
		if (m_specialEventKeySequence.isEmpty() && (keyText == "Ctrl" || keyText == "Shift" || keyText == "Alt"))
        {    
			m_specialEvent += "\t<event type=\"specialevent\">\n";
			m_specialEvent += "\t\t<time>" + QTime::currentTime().toString("hh:mm:ss:zzz") + "</time>\n";
			m_specialEvent += "\t\t<objectClass>" + QString(obj.metaObject()->className()) + "</objectClass>\n";
			m_specialEvent += "\t\t<objectName>" + obj.objectName() + "</objectName>\n";
			m_specialEvent += "\t\t<objectText>" + getObjectLabel(obj) + "</objectText>\n";
			m_specialEvent += "\t\t<objectPath>" + objectPath(&obj) + "</objectPath>\n";
			m_specialEvent += "\t\t<actionsSequence>\n";

			if (m_keyHold)
			{
				return;
			}

			m_specialEventKeySequence.append(keyText);

			m_specialEvent += "\t\t\t<key>" + keyText + "</key>\n";
        }
		else if (!m_specialEventKeySequence.isEmpty()) // if there are some keys in special event key sequence, just add new key to it
        {
			if (m_keyHold)
			{
				return;
			}

			m_specialEventKeySequence.append(keyText);
			m_specialEvent += "\t\t\t<key>" + keyText + "</key>\n";
        }
    }
    else if (ev.type() == QEvent::KeyRelease)
    {
		// if keyboard event are disabled, just remove special key text from list
		if (!m_keyboardEnabled)
		{
			if ((keyText == "Ctrl" || keyText == "Shift" || keyText == "Alt") && m_specialEventKeySequence.contains(keyText))
			{
				m_specialEventKeySequence.removeAt(m_specialEventKeySequence.indexOf(keyText));
			}

			return;
		}

		// key from special event key sequence is released
		if (!m_specialEventKeySequence.isEmpty() && m_specialEventKeySequence.contains(keyText))
        {
			m_specialEventKeySequence.removeAt(m_specialEventKeySequence.indexOf(keyText));

			if (m_shortcutRaised)
			{
				return;
			}

			if (m_specialEventKeySequence.isEmpty())
			{
				if (!m_shortcutProcessed)
				{
					return;
				}

				QTextStream out(m_outputFile);
				out.setCodec("UTF-8");
				out << m_specialEvent + "\t\t</actionsSequence>\n\t</event>\n";

				m_specialEvent = "";
			}
			
        }
		else if (m_specialEventKeySequence.isEmpty()) // normal event
        {
			if (!m_shortcutProcessed || m_keyHold || (keyText != m_lastPressedKey))
			{
				return;
			}

            writeKeyEventToFile(*ke, obj);
        }
    }
}

template <class C>
void CEventFilter::processMenuAction(C &a)
{
	if (!m_state)
		return;

	checkWheelEvent();
	checkMouseEvent();

	if (!m_supportedClasses.contains("QMenu"))
	{
		return;
	}

	// process shortcut
	if (m_shortcutRaised)
	{
		if (!m_keyboardEnabled)
		{
			m_specialEvent = "";
			m_specialEventKeySequence.clear();
			m_shortcutProcessed = false;
			m_shortcutRaised = false;
			return;
		}			

		QStringList list = a.shortcut().toString().split("+");

		QTextStream out(m_outputFile);
		out.setCodec("UTF-8");
		out << "\t<event type=\"specialevent\">\n";
		out << "\t\t<time>" << QTime::currentTime().toString("hh:mm:ss:zzz") << "</time>\n";
		out << "\t\t<objectClass>" << a.metaObject()->className() << "</objectClass>\n";
		out << "\t\t<objectName>" << a.objectName() << "</objectName>\n";
		out << "\t\t<objectText>" << a.text() << "</objectText>\n";
		out << "\t\t<objectPath>" << m_menuPath << "/" << a.objectName() << "</objectPath>\n";
		out << "\t\t<actionsSequence>\n";

		foreach (QString item, list)
		{
			out << "\t\t\t<key>" << item << "</key>\n";
		}

		out << "\t\t</actionsSequence>\n";
		out << "\t</event>\n";

		m_specialEvent = "";
		m_specialEventKeySequence.clear();
		m_shortcutProcessed = false;
		m_shortcutRaised = false;
	}
	else // process mouse menu click
	{
		if (!m_mouseEnabled)
		{
			return;
		}

		QTextStream out(m_outputFile);
		out.setCodec("UTF-8");
		out << "\t<event type=\"mouseevent\">\n";
		out << "\t\t<time>" << QTime::currentTime().toString("hh:mm:ss:zzz") << "</time>\n";
		out << "\t\t<objectClass>" << a.metaObject()->className() << "</objectClass>\n";
		out << "\t\t<objectName>" << a.objectName() << "</objectName>\n";
		out << "\t\t<objectText>" << a.text() << "</objectText>\n";
		out << "\t\t<objectPath>" << m_menuPath << "/" << a.objectName() << "</objectPath>\n";
		out << "\t\t<action>" << "menuclick" << "</action>\n";
		out << "\t</event>\n";
	}
}

// plugin menu actions (icon buttons clicks) are connected to this handler in CPluginManager::PopulateMenus()
void CEventFilter::catchMenuAction()
{
	QAction *a = qobject_cast<QAction *>(QObject::sender());

	if (a)
	{
		processMenuAction<QAction>(*a);
	}
}

// plugin menu actions are connected to this handler in CPluginManager::PopulateMenus()
void CEventFilter::catchPluginMenuAction(QAction *a)
{
	processMenuAction<QAction>(*a);
}

// wizard buttons are connected to this handler in wizard.cpp
void CEventFilter::processWizardButtons()
{
	QToolButton *b = qobject_cast<QToolButton *>(QObject::sender());

	if (b)
	{
		processMenuAction<QToolButton>(*b);
	}
}

// icon buttons in CVolumeRenderWindow (TT) are connected to this handler in MainWindow::createOSGStuff()
void CEventFilter::catchColorWidgetClicked(double x, double y, bool bTimer)
{
	if (!m_state)
		return;

	if (!m_mouseEnabled || !m_supportedClasses.contains(QObject::sender()->metaObject()->className()))
	{
		return;
	}

	m_prevMouseEvent = "";

	QTextStream out(m_outputFile);
	out.setCodec("UTF-8");
	out << "\t<event type=\"mouseevent\">\n";
	out << "\t\t<time>" << QTime::currentTime().toString("hh:mm:ss:zzz") << "</time>\n";
	out << "\t\t<objectClass>" << QObject::sender()->metaObject()->className() << "</objectClass>\n";
	out << "\t\t<objectName>" << QObject::sender()->objectName() << "</objectName>\n";
	out << "\t\t<objectText></objectText>\n";
	out << "\t\t<objectPath>" << m_menuPath << "/" << QObject::sender()->objectName() << "</objectPath>\n";
	out << "\t\t<mouseButton>" << m_prevMouseButton << "</mouseButton>\n";
	out << "\t\t<action>" << "click" << "</action>\n";
	out << "\t\t<position>" << QString::number(m_prevMousePos.x()) << ":" + QString::number(m_prevMousePos.y()) << "</position>\n";
	out << "\t</event>\n";
}

bool CEventFilter::eventFilter(QObject *obj, QEvent *ev)
{
	// call a function based on event type
    if (ev)
	{
		if (ev->type() == QEvent::Shortcut)
		{
			m_shortcutRaised = true;
		}

		if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease)
		{
			processKeyboardEvent(*ev, *obj);
		}

        if (m_mouseEnabled)
        {
            if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonRelease)
            {
                processMouseEvent(*ev, *obj);
            }
            else if (ev->type() == QEvent::Wheel)
            {
                processWheelEvent(*ev, *obj);
            }
        }
	}

	return false;
}

void CEventFilter::showEventFilterLog()
{
	// create dialog and show content of output file in it
	QDialog dlg(NULL, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
	dlg.setWindowTitle(QApplication::applicationName());
	dlg.setMinimumSize(720, 480);
	dlg.setSizeGripEnabled(true);
	QVBoxLayout* pLayout = new QVBoxLayout();
	dlg.setLayout(pLayout);
	QLabel* pLabelInfo = new QLabel;
	pLabelInfo->setText(tr("Event Filter Log:"));
	pLayout->addWidget(pLabelInfo);
	QTextEdit* pTextEdit = new QTextEdit();

	if (m_outputFile != NULL)
	{
		m_outputFile->close();
		if (m_outputFile->open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QTextStream in(m_outputFile);
			in.setCodec("UTF-8");

			while (true)
			{
				QString text = in.readLine();

				if (text.isEmpty())
				{
					break;
				}

				pTextEdit->append(text);
			}			
		}
		m_outputFile->close();
		if (m_outputFile->open(QIODevice::Append | QIODevice::Text))
		{
			//error
			assert(m_outputFile);
		}
	}
	
	pLayout->addWidget(pTextEdit);
	QDialogButtonBox* pButtonBox = new QDialogButtonBox;
	pButtonBox->setStandardButtons(QDialogButtonBox::Ok);
	pButtonBox->setCenterButtons(true);
	connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
	pLayout->addWidget(pButtonBox);
	dlg.exec();
}