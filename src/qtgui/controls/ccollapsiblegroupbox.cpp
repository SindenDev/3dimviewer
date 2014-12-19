///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
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

#include <controls/ccollapsiblegroupbox.h>
#include <QSettings>

CCollapsibleGroupBox::CCollapsibleGroupBox() :
    m_pGBox(NULL),
    m_nHeight(0)
{

}

void CCollapsibleGroupBox::setGroupBox(QGroupBox* pGBox, const QString& regName)
{
    m_pGBox = pGBox;
    m_sRegName = regName;
    Q_ASSERT(NULL!=m_pGBox);
    Q_ASSERT(!m_sRegName.isEmpty());
    if (NULL!=pGBox)
    {        
        m_pGBox->setStyle(new CCollapsibleGroupBoxStyle());
        m_pGBox->setObjectName("CollapsibleBox");

        m_nHeight = pGBox->sizeHint().height();
        pGBox->setCheckable(true);
        QObject::connect(pGBox,SIGNAL(clicked(bool)),this,SLOT(packBox(bool)));

        // read last used settings
        QSettings settings;
        settings.beginGroup("GroupBox");
        if (settings.value(m_sRegName,false).toBool())
        {
            pGBox->setChecked(false);
            packBox(false);
        }
    }
}

CCollapsibleGroupBox::~CCollapsibleGroupBox()
{
    if (NULL!=m_pGBox)
    {
        QSettings settings;
        settings.beginGroup("GroupBox");
        settings.setValue(m_sRegName,!m_pGBox->isChecked());
    }
}

void CCollapsibleGroupBox::packBox(bool checked)
{
    if (checked)
        m_pGBox->setMaximumHeight(m_nHeight);
    else
        m_pGBox->setMaximumHeight(GROUPBOX_HEIGHT);
}
