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

#ifndef CPREVIEWDIALOGDATA_H
#define CPREVIEWDIALOGDATA_H

#include <VPL/Base/Types.h>
#include <QString>
#include <QImage>

#include <data/CObjectPtr.h>
#include <data/CDensityData.h>
#include <VPL/Module/Progress.h>
#include <dialogs/cprogress.h>

class CPreviewDialogParametersDescription
{

protected:
	// vector of filter parameters values
	std::vector<double> m_params;

	// vector of parameters lower limit
	std::vector<double> m_paramsMin;

	// vector of parameters upper limit
	std::vector<double> m_paramsMax;

	// for different filters and detectors (bilateral, median, anizotropic, canny,...) we need different amount of parameters and different step in those values
	// (eg canny needs 2 parameters with step 0.01, but bilateral needs 3 parameters with step 1), so those vectors are here to tell the dialog this information
	std::vector<bool> m_paramsVisible;
	std::vector<double> m_paramsSteps;
	std::vector<uint> m_paramsDecimalsCnt;
	std::vector<QString> m_paramsNames;

	// sometimes we need some parameter value to keep lower than another one (eg canny), so we need to restrict some slider's movement
	// to achieve that and keep the dialog universal, we need some variables, but now it is possible to do that only for one parameter for each filter
	// index is 0-2
	bool m_isSomeParamRestricted;
	int m_restrictedParamIndex;
	int m_boundaryParamIndex;

public:
	CPreviewDialogParametersDescription()
	{
		// fill vectors and variables with default values

		m_params.push_back(1);
		m_params.push_back(1);
		m_params.push_back(1);

		m_paramsMin.push_back(0);
		m_paramsMin.push_back(0);
		m_paramsMin.push_back(0);

		m_paramsMax.push_back(1);
		m_paramsMax.push_back(1);
		m_paramsMax.push_back(1);

		m_paramsVisible.push_back(true);
		m_paramsVisible.push_back(true);
		m_paramsVisible.push_back(true);

		m_paramsSteps.push_back(1);
		m_paramsSteps.push_back(1);
		m_paramsSteps.push_back(1);

		m_paramsDecimalsCnt.push_back(0);
		m_paramsDecimalsCnt.push_back(0);
		m_paramsDecimalsCnt.push_back(0);

		m_paramsNames.push_back("param 1: ");
		m_paramsNames.push_back("param 2: ");
		m_paramsNames.push_back("param 3: ");

		m_isSomeParamRestricted = false;
		m_restrictedParamIndex = -1;
		m_boundaryParamIndex = -1;
	}

	~CPreviewDialogParametersDescription() {}

	void setParams(double v1, double v2, double v3)
	{
		m_params[0] = v1;
		m_params[1] = v2;
		m_params[2] = v3;
	}

	void setParamsMin(double v1, double v2, double v3)
	{
		m_paramsMin[0] = v1;
		m_paramsMin[1] = v2;
		m_paramsMin[2] = v3;
	}

	void setParamsMax(double v1, double v2, double v3)
	{
		m_paramsMax[0] = v1;
		m_paramsMax[1] = v2;
		m_paramsMax[2] = v3;
	}

	void setParamsVisibility(bool v1, bool v2, bool v3)
	{
		m_paramsVisible[0] = v1;
		m_paramsVisible[1] = v2;
		m_paramsVisible[2] = v3;
	}

	void setParamsSteps(double v1, double v2, double v3)
	{
		m_paramsSteps[0] = v1;
		m_paramsSteps[1] = v2;
		m_paramsSteps[2] = v3;
	}

	void setParamsDecimalsCnt(uint v1, uint v2, uint v3)
	{
		m_paramsDecimalsCnt[0] = v1;
		m_paramsDecimalsCnt[1] = v2;
		m_paramsDecimalsCnt[2] = v3;
	}

	void setParamsNames(QString v1, QString v2, QString v3)
	{
		m_paramsNames[0] = v1;
		m_paramsNames[1] = v2;
		m_paramsNames[2] = v3;
	}

	void setParam(uint index, double v)
	{
		if (index >= m_params.size())
			return;

		m_params[index] = v;
	}

	void setParamMin(uint index, double v)
	{
		if (index >= m_paramsMin.size())
			return;

		m_paramsMin[index] = v;
	}

	void setParamMax(uint index, double v)
	{
		if (index >= m_paramsMax.size())
			return;

		m_paramsMax[index] = v;
	}

	void setParamVisibility(uint index, bool v)
	{
		if (index >= m_paramsVisible.size())
			return;

		m_paramsVisible[index] = v;
	}

	void setParamStep(uint index, double v)
	{
		if (index >= m_paramsSteps.size())
			return;

		m_paramsSteps[index] = v;
	}

	void setParamDecimalCnt(uint index, uint v)
	{
		if (index >= m_paramsDecimalsCnt.size())
			return;

		m_paramsDecimalsCnt[index] = v;
	}

	void setParamName(uint index, QString v)
	{
		if (index >= m_paramsNames.size())
			return;

		m_paramsNames[index] = v;
	}

	std::vector<double>& getParams()
	{
		return m_params;
	}

	std::vector<double>& getParamsMin()
	{
		return m_paramsMin;
	}

	std::vector<double>& getParamsMax()
	{
		return m_paramsMax;
	}

	std::vector<bool>& getParamsVisibility()
	{
		return m_paramsVisible;
	}

	std::vector<double>& getParamsSteps()
	{
		return m_paramsSteps;
	}

	std::vector<uint>& getParamsDecimalsCnt()
	{
		return m_paramsDecimalsCnt;
	}

	std::vector<QString>& getParamsNames()
	{
		return m_paramsNames;
	}

	double getParam(uint index)
	{
		return (index < m_params.size()) ? m_params[index] : -1;
	}

	double getParamMin(uint index)
	{
		return (index < m_paramsMin.size()) ? m_paramsMin[index] : -1;
	}

	double getParamMax(uint index)
	{
		return (index < m_paramsMax.size()) ? m_paramsMax[index] : -1;
	}

	bool isParamVisible(uint index)
	{
		return (index < m_paramsVisible.size()) ? m_paramsVisible[index] : false;
	}

	double getParamStep(uint index)
	{
		return (index < m_paramsSteps.size()) ? m_paramsSteps[index] : -1;
	}

	uint getParamDecimalCnt(uint index)
	{
		return (index < m_paramsDecimalsCnt.size()) ? m_paramsDecimalsCnt[index] : -1;
	}

	QString getParamName(uint index)
	{
		return (index < m_paramsNames.size()) ? m_paramsNames[index] : "parameter :";
	}

	void setParamRestrictionFlag(bool flag)
	{
		m_isSomeParamRestricted = flag;
	}

	bool isSomeParamRestricted()
	{
		return m_isSomeParamRestricted;
	}

	void setRestrictedParamIndex(uint index)
	{
		m_restrictedParamIndex = index;
	}

	uint getRestrictedParamIndex()
	{
		return m_restrictedParamIndex;
	}

	void setBoundaryParamIndex(uint index)
	{
		m_boundaryParamIndex = index;
	}

	uint getBoundaryParamIndex()
	{
		return m_boundaryParamIndex;
	}
};

class CPreviewDialogData
{

protected:

	// index of current slice
	int m_currentSlice;

	// index of current filter
	int m_currentFilterIndex;

	// reference to source volume (used in preview method)
	vpl::img::CDensityVolume& m_srcVolume;

	// vector of filter names
	std::vector<QString> m_filterNames;

	// vector of parameters description (one for each filter)
	std::vector<CPreviewDialogParametersDescription *> m_paramsDesc;

	// number of slices on the top and the bottom margin of the volume that cannot be shown in preview, because the filter kernel is too big to filter this area
	int m_hiddenSliceCnt;

public:

	CPreviewDialogData(vpl::img::CDensityVolume& volume) : m_srcVolume(volume), m_currentSlice(0), m_currentFilterIndex(0), m_hiddenSliceCnt(0)
	{
		// create new parameters description

		m_paramsDesc.push_back(new CPreviewDialogParametersDescription());
		m_paramsDesc.push_back(new CPreviewDialogParametersDescription());
		m_paramsDesc.push_back(new CPreviewDialogParametersDescription());
		m_paramsDesc.push_back(new CPreviewDialogParametersDescription());
		m_paramsDesc.push_back(new CPreviewDialogParametersDescription());
	}

	virtual ~CPreviewDialogData() 
	{
		for (int i = 0; i < m_paramsDesc.size(); ++i)
		{
			delete m_paramsDesc[i];
		}
	}

	void setVolume(vpl::img::CDensityVolume& volume)
	{
		m_srcVolume = volume;
	}

	vpl::img::CDensityVolume& getVolume()
	{
		return m_srcVolume;
	}

	void addFilterName(QString name)
	{
		m_filterNames.push_back(name);
	}

	void setCurrentSlice(int slice)
	{
		m_currentSlice = slice;
	}

	int getCurrentSlice()
	{
		return m_currentSlice;
	}

	int getVolumeZSize()
	{
		return(m_srcVolume.getZSize());
	}

	CPreviewDialogParametersDescription* getParamDescription(int index)
	{
		return (index < m_paramsDesc.size()) ? m_paramsDesc[index] : m_paramsDesc[m_paramsDesc.size() - 1];
	}

	void setParamDescription(int index, CPreviewDialogParametersDescription* paramDesc)
	{
		if (index < m_paramsDesc.size())
		{
			m_paramsDesc[index] = paramDesc;
		}
	}

	size_t getFiltersCnt()
	{
		return m_filterNames.size();
	}

	std::vector<QString>& getFiltersNames()
	{
		return m_filterNames;
	}

	void setCurrentFilterIndex(int index)
	{
		m_currentFilterIndex = index;
	}

	int getCurrentFilterIndex()
	{
		return m_currentFilterIndex;
	}

	void setHiddenSliceCnt(int cnt)
	{
		m_hiddenSliceCnt = cnt;
	}

	int getHiddenSliceCnt()
	{
		return m_hiddenSliceCnt;
	}

	// initialize member variables here
	virtual void init() = 0;

	// called when filter index has changed
	virtual void filterChanged(int filterIndex) = 0;

	// apply current filter with current parametrs on src volume, the result is set to dst volume, needs reference to progress to show progress dialog
	virtual bool apply(vpl::img::CDensityVolume& src, vpl::img::CDensityVolume& dst, CProgress& progress) = 0;

	// called when slice or some parameter has changed
	// need to fill both original and result image here
	virtual void preview(QImage& original, QImage& result, int sliceIndex) = 0;
};

#endif // CPREVIEWDIALOGDATA_H
