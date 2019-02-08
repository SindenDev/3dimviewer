///////////////////////////////////////////////////////////////////////////////
// $Id: ccolorcombobox.cpp 3810 2013-07-24 08:55:52Z tryhuk $
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

#include <controls/ccolorcombobox.h>
#include <QPainter>

CColorComboBox::CColorComboBox(QComboBox *pCombo)
{
	m_pCombo = pCombo;
	m_bSyncActive = true;
    m_bShowNewRegionItem = false;
    m_bNewRegionItemAdded = false;
    m_bShowEmptyItem = false;
    m_bEmptyItemAdded = false;
    m_activeRegion = -1;
    // can't connect directly, because it is used from plugins too
    //data::CGeneralObjectObserver<CColorComboBox>::connect(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id).get(), data::CGeneralObjectObserver<CColorComboBox>::tObserverHandler(this, &CColorComboBox::sigRegionColoringChanged));
}

CColorComboBox::~CColorComboBox()
{
}

void CColorComboBox::setCombo(QComboBox *pCombo)
{
    m_pCombo = pCombo;
}

QComboBox *CColorComboBox::getCombo() const
{
    return m_pCombo;
}

void CColorComboBox::setSyncActiveRegion(bool bSyncActive)
{
    m_bSyncActive = bSyncActive;
}

void CColorComboBox::comboAddColorItem(const QColor& color, const QString& itemName)
{
    if (!m_pCombo)
    {
        return;
    }

	QPixmap pix(12, 12);
	QPainter painter(&pix);

	if (color.isValid())
	{
		painter.setPen(Qt::gray);
		painter.setBrush(QBrush(color));
		painter.drawRect(0, 0, 12, 12);
	}

	QIcon icon;
	icon.addPixmap(pix);
	m_pCombo->addItem(icon, itemName, color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void :::objectChanged(data::CRegionColoring *pData)
//
//\brief Redraw data.
//
//\param [in,out] pData If non-null, the data.
////////////////////////////////////////////////////////////////////////////////////////////////////

void CColorComboBox::sigRegionColoringChanged(data::CStorageEntry *pData, const data::CChangedEntries &changes)
{
    if (!m_pCombo)
    {
        return;
    }

    data::CRegionColoring *regionColoring = pData->getDataPtr<data::CObjectHolder<data::CRegionColoring> >()->getObjectPtr();
    updateFromColoring(regionColoring);
}

void CColorComboBox::updateFromColoring(data::CRegionColoring * regionColoring)
{
    if (!m_pCombo) 
        return;

	m_pCombo->blockSignals(true);

	int oldIndex = m_pCombo->currentIndex();

	// Clear old data
	m_pCombo->clear();

	// Fill combo with data

	data::CRegionColoring::tColor color;

	m_mapping.clear();
    m_mapping.resize(regionColoring->getNumOfRegions() + 2);

	int regIndex = 0;

    for (int i = 0; i < regionColoring->getNumOfRegions(); ++i)
	{
        while (regionColoring->getRegionInfo(i).isAuxiliary())
		{
			++i;
		}

        if (i > regionColoring->getNumOfRegions() - 1)
		{
			break;
		}

		m_mapping[i] = regIndex;

		// Get color
        color = regionColoring->getColor(i);

        QString regName = QString::fromUtf8(regionColoring->getRegionInfo(i).getName().c_str());
		QColor  qcolor(color.getR(), color.getG(), color.getB());
		comboAddColorItem(qcolor, regName);

		++regIndex;
	}

    int activeIndex = regionColoring->getActiveRegion();

	if (m_bSyncActive)
	{
		m_pCombo->setCurrentIndex(m_mapping[activeIndex]);
	}
	else
	{
		if (oldIndex >= 0 && oldIndex < m_pCombo->count())
		{
			m_pCombo->setCurrentIndex(oldIndex);
		}
	}

	m_pCombo->blockSignals(false);
}

void CColorComboBox::setNewRegionItemVisible(bool visible)
{
    m_bShowNewRegionItem = visible;
}

void CColorComboBox::setEmptyItemVisible(bool visible)
{
    m_bShowEmptyItem = visible;
}

void CColorComboBox::sigMultiClassRegionColoringChanged(data::CStorageEntry *pData, const data::CChangedEntries &changes)
{
    if (!m_pCombo)
    {
        return;
    }

    data::CMultiClassRegionColoring *regionColoring = pData->getDataPtr<data::CObjectHolder<data::CMultiClassRegionColoring> >()->getObjectPtr();
    updateFromMultiClassColoring(regionColoring);
}

void CColorComboBox::updateFromMultiClassColoring(data::CMultiClassRegionColoring * regionColoring)
{
    if (!m_pCombo)
        return;

    m_pCombo->blockSignals(true);

    int oldIndex = m_pCombo->currentIndex();
    bool newRegionSelected = isNewRegionItemSelected();
    bool emptySelected = isEmptyItemSelected();

    // Clear old data
    m_pCombo->clear();

    // Fill combo with data

    data::CMultiClassRegionColoring::tColor color;

    m_mapping.clear();
    m_mapping.resize(regionColoring->getNumOfRegions() + 2);

    int regIndex = 0;
    int i = 0;

    m_bEmptyItemAdded = false;

    if (m_bShowEmptyItem && regIndex < data::CMultiClassRegionData::MAX_REGIONS)
    {
        m_mapping[i] = regIndex;
        ++regIndex;
        QString regName(QObject::tr("No Region"));
        QColor  qcolor(0, 0, 0);
        comboAddColorItem(qcolor, regName);
        m_bEmptyItemAdded = true;
    }

    for (; i < regionColoring->getNumOfRegions(); ++i)
    {
        while (regionColoring->getRegionInfo(i).isAuxiliary())
        {
            ++i;
        }

        if (i > regionColoring->getNumOfRegions() - 1)
        {
            break;
        }

        m_mapping[i] = regIndex;

        // Get color
        color = regionColoring->getColor(i);

        QString regName = QString::fromUtf8(regionColoring->getRegionInfo(i).getName().c_str());
        QColor  qcolor(color.getR(), color.getG(), color.getB());
        comboAddColorItem(qcolor, regName);

        ++regIndex;
    }

    m_bNewRegionItemAdded = false;

    if (m_bShowNewRegionItem && regIndex < data::CMultiClassRegionData::MAX_REGIONS)
    {
        m_mapping[i] = regIndex; 
        QString regName(QObject::tr("New Region"));
        QColor  qcolor(255, 255, 255);
        comboAddColorItem(qcolor, regName);
        m_bNewRegionItemAdded = true;
    }

    int activeIndex = regionColoring->getActiveRegion();

    if ((activeIndex != m_activeRegion) && m_bSyncActive)
    {
        m_pCombo->setCurrentIndex(m_mapping[activeIndex]);
    }
    else if (m_bSyncActive && !(m_bNewRegionItemAdded && oldIndex == m_pCombo->count() - 1) && !(m_bEmptyItemAdded && oldIndex == 0))
    {
        m_pCombo->setCurrentIndex(m_mapping[activeIndex]);
    }
    else
    {
        if (newRegionSelected)
        {
            m_pCombo->setCurrentIndex(m_pCombo->count() - 1);
        }
        else if (emptySelected)
        {
            m_pCombo->setCurrentIndex(0);
        }
        else if (oldIndex >= 0 && oldIndex < m_pCombo->count())
        {
            m_pCombo->setCurrentIndex(oldIndex);
        }
    }

    m_activeRegion = activeIndex;

    m_pCombo->blockSignals(false);
}

int CColorComboBox::getRegionNumber(int index)
{
	int pos = std::find(m_mapping.begin(), m_mapping.end(), index) - m_mapping.begin();

	return (pos >= m_mapping.size()) ? 0 : pos;
}

void CColorComboBox::usualIndexChangedHandler(data::CDataStorage *pDataStorage, int index)
{
	Q_ASSERT(NULL != pDataStorage);

	// Change the active region via data storage
	data::CObjectPtr<data::CRegionColoring> spColoring(pDataStorage->getEntry(data::Storage::RegionColoring::Id));

	if (spColoring.get())
	{
		m_pCombo->blockSignals(true); // objectchanged of combo will be called, therefore we have to block signals
		spColoring->setActiveRegion(getRegionNumber(index));
		pDataStorage->invalidate(spColoring.getEntryPtr());
		m_pCombo->blockSignals(false); // objectchanged of combo will be called, therefore we have to block signals
	}
}

void CColorComboBox::usualIndexChangedHandlerMultiClass(data::CDataStorage *pDataStorage, int index)
{
    Q_ASSERT(NULL != pDataStorage);

    // Change the active region via data storage
    data::CObjectPtr<data::CMultiClassRegionColoring> spColoring(pDataStorage->getEntry(data::Storage::MultiClassRegionColoring::Id));

    if (spColoring.get())
    {
        m_pCombo->blockSignals(true); // objectchanged of combo will be called, therefore we have to block signals
        spColoring->setActiveRegion(getRegionNumber(index));
        pDataStorage->invalidate(spColoring.getEntryPtr());
        m_pCombo->blockSignals(false); // objectchanged of combo will be called, therefore we have to block signals
    }
}

bool CColorComboBox::isNewRegionItemSelected()
{
    if (m_bNewRegionItemAdded && (m_pCombo->currentIndex() == m_pCombo->count() - 1))
    {
        return true;
    }

    return false;
}

bool CColorComboBox::isEmptyItemSelected()
{
    if (m_bEmptyItemAdded && (m_pCombo->currentIndex() == 0))
    {
        return true;
    }

    return false;
}