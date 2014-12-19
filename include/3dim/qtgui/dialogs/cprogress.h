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
public:
    explicit CProgress(QWidget *parent=0) :
        QProgressDialog(parent,Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
    {
		m_pModifierFn = NULL;
        setAttribute(Qt::WA_ShowWithoutActivating);
        setWindowTitle(QApplication::applicationName());
        setWindowModality(Qt::WindowModal);
    }
    void setTitle(const QString &title) { setWindowTitle(title); };
	void setModifierFn(ProgressModifierFn mfn) { m_pModifierFn = mfn; }
    bool Entry(int iCount, int iMax)
    {
        if (QThread::currentThread()==QApplication::instance()->thread())
        {
			if (NULL!=m_pModifierFn)
				m_pModifierFn(iCount,iMax);
            setMaximum (iMax);
            setValue(iCount);
            QApplication::processEvents();
            return !(this->wasCanceled());
        }
        return true;
    }
    void restart(const QString &message) { setLabelText(message); show(); QApplication::processEvents(); }
};

#endif
