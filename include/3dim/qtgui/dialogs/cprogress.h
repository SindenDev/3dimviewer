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

#ifndef _CPROGRESS_H
#define _CPROGRESS_H

///////////////////////////////////////////////////////////////////////////////
// Custom progress

#include <QProgressDialog>
#include <QApplication>
#include <QThread>

typedef void (*ProgressModifierFn)(int& iVal, int &iMax);

//! Progress class
class CProgress : public QProgressDialog
{
    Q_OBJECT
protected:
	ProgressModifierFn m_pModifierFn;
	bool m_bCanceled;
    int m_loopsCnt;
    int m_currentLoopIndex;
public:
    explicit CProgress(QWidget *parent=0) :
        QProgressDialog(parent,Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
    {
		m_pModifierFn = NULL;
		m_bCanceled = false;
        m_loopsCnt = 1;
        m_currentLoopIndex = 0;
        setAttribute(Qt::WA_ShowWithoutActivating);
        setWindowTitle(QApplication::applicationName());
        setWindowModality(Qt::WindowModal);
#ifdef __APPLE__
        setMinimumWidth(600);
#endif
    }
    void setTitle(const QString &title) { setWindowTitle(title); };
	void setModifierFn(ProgressModifierFn mfn) { m_pModifierFn = mfn; }
    void setLoopsCnt(int cnt) { m_loopsCnt = cnt; }
    void setCurrentLoopIndex(int index) { m_currentLoopIndex = index; }
    bool Entry(int iCount, int iMax)
    {
        if (QThread::currentThread()==QApplication::instance()->thread())
        {
            if (NULL != m_pModifierFn)
            {
                int max = iMax * m_loopsCnt;
                int count = iCount + (iMax * m_currentLoopIndex);
                m_pModifierFn(count, max);
            }
			if (!isHidden())
			{
                const int prevMax = maximum();
                const int prevVal = value();
                const int newMax = iMax * m_loopsCnt;
                const int newVal = iCount + (iMax * m_currentLoopIndex);
                if ((100 * newVal) / std::max(1, newMax) != (100 * prevVal) / std::max(1, prevMax)) // some actions call progress too often           
                {
                    setMaximum(newMax);
                    setValue(newVal);
                    bool bCanceled = m_bCanceled = this->wasCanceled(); // save to local variable because processEvents can lead to destruction of this object
                    QApplication::processEvents();
                    return !bCanceled;
                }
			}
			m_bCanceled = this->wasCanceled();
            return !m_bCanceled;
        }
        return !m_bCanceled;
    }
    void restart(const QString &message)
    {
        setLabelText(message);
        m_bCanceled = false;
        show();
        QApplication::processEvents();
    }
    void setInfinite()
	{
        setRange(0,0);
        setValue(0);
	}
};

#endif
