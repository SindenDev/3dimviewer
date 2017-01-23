///////////////////////////////////////////////////////////////////////////////
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

#ifndef _CMENUEX_H
#define _CMENUEX_H

#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPaintEvent>

#define HIGHLIGHT_COLOR qRgb(255,0,160)

//! Highlighting functionality core for QMenu and QMenuBar
class CMenuHighlighter
{
protected:
	QList<QObject*> m_listHighlighted;
public:
	CMenuHighlighter()
	{
	}
	void setHighlightedActions(const QList<QObject*>& list)
	{
		m_listHighlighted.clear();
		foreach(QObject* act, list)
			m_listHighlighted.push_back(act);
	}
	void clearHighlightedActions()
	{
		m_listHighlighted.clear();
	}
};

//! Menu bar that supports action highlighting
class CMenuBarEx : public QMenuBar, public CMenuHighlighter
{
	Q_OBJECT
public:
	CMenuBarEx(QWidget * parent = 0) : QMenuBar(parent)
	{
	}
protected:
	virtual void paintEvent(QPaintEvent* e) override
	{
		QMenuBar::paintEvent(e);
		if (m_listHighlighted.size()>0)
		{
			QPainter p(this);
			p.setPen(QColor(HIGHLIGHT_COLOR));
			foreach(QObject* pObj, m_listHighlighted)
			{
				QAction* pAct = qobject_cast<QAction*>(pObj);
				if (NULL==pAct)
				{
					QMenu* pmenu = qobject_cast<QMenu*>(pObj);
					if (NULL!=pmenu)
						pAct=pmenu->menuAction();
				}
				if (NULL!=pAct)
				{
					QRect g = actionGeometry(pAct);
					if (g.isValid())
						p.drawRect(g);
				}
			}
		}
	}
};

//! Menu that supports action highlighting
class CMenuEx : public QMenu, public CMenuHighlighter
{
	Q_OBJECT	
public:
	CMenuEx(QWidget * parent = 0) : QMenu(parent)
	{
	}
	CMenuEx(const QString & title, QWidget * parent = 0) : QMenu(title,parent)
	{
	}
protected:
	virtual void paintEvent(QPaintEvent* e) override
	{
		QMenu::paintEvent(e);
		if (m_listHighlighted.size()>0)
		{
			QPainter p(this);
			p.setPen(QColor(HIGHLIGHT_COLOR));
			foreach(QObject* pObj, m_listHighlighted)
			{
				QAction* pAct = qobject_cast<QAction*>(pObj);
				if (NULL==pAct)
				{
					QMenu* pmenu = qobject_cast<QMenu*>(pObj);
					if (NULL!=pmenu)
						pAct=pmenu->menuAction();
				}
				if (NULL!=pAct)
				{
					QRect g = actionGeometry(pAct);
					if (g.isValid())
						p.drawRect(g);
				}
			}
		}
	}
};


#endif // _CMENUEX_H