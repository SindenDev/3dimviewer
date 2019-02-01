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

#include <core/alg/MultiStageProgress.h>
//#include <osg/dbout.h>

/************************************************************************/
/* CLASS CMultiStagedProgressProvider                                   */
/************************************************************************/

/*!
 * \fn	bool CMultiStagedProgressProvider::setCurrentProgressStage(const std::string &stage)
 *
 * \brief	Sets current progress stage (identified by string name)
 *
 * \param	stage	The stage.
 *
 * \return	True if it succeeds, false if it fails.
 */

bool CMultiStagedProgressProvider::setCurrentProgressStage(const std::string &stage)
{
	// Initialize stages if not done yet...
	if (!m_progres_stages.wasInitialized())
		initProgressStages();

	// Complete length can be changed by adding stage, so change maximal value for sure...
	m_progressObj.setProgressMax(m_progres_stages.getCompleteDuration());

	int min, max;
	if (m_progres_stages.getStageLimits(stage, min, max)) 
	{ 
//		DBOUT("Current stage: " << stage.c_str() << ", min: " << min << ", max: " << max);

		m_current_progress_segment.set(min, max); 
		return true; 
	}
	
	return false;
}

/*!
 * \fn	bool CMultiStagedProgressProvider::localProgress(int count, int max)
 *
 * \brief	Local progress method - recalculates subprogress value to the final progress 
 *
 * \param	count	Number of.
 * \param	max  	The maximum.
 *
 * \return	True if it succeeds, false if it fails.
 */

bool CMultiStagedProgressProvider::localProgressFunction(int count, int max)
{
	// Initialize stages if not done yet...
	if (!m_progres_stages.wasInitialized())
		initProgressStages();

	int recalculated(m_current_progress_segment.recalculate(count, max));
	int current_value(m_progressObj.getProgressCount());
	
	return m_progressObj.progress(recalculated - current_value);
}

/************************************************************************/
/* CLASS CProgresStages                                                 */
/************************************************************************/

/*!
 * \fn	CMultiStagedProgressProvider::CProgresStages::CProgresStages()
 *
 * \brief	Default constructor
 */

CMultiStagedProgressProvider::CProgressStages::CProgressStages()
{

}

/*!
 * \fn	CMultiStagedProgressProvider::SProgressStage CMultiStagedProgressProvider::CProgresStages::getStage(const std::string &stage_name) const
 *
 * \brief	Gets a stage data
 *
 * \param	stage_name	Name of the stage.
 *
 * \return	The stage.
 */

CMultiStagedProgressProvider::SProgressStage CMultiStagedProgressProvider::CProgressStages::getStage(const std::string &stage_name) const
{
	auto iter = findStage(stage_name);
	if (iter != m_stages.end())
		return *iter;

	return SProgressStage();
}

/*!
 * \fn	void CMultiStagedProgressProvider::CProgresStages::addStage(const std::string name, int order_number, int length)
 *
 * \brief	Adds a new stage - by values
 *
 * \param	name			The name.
 * \param	order_number	The order number.
 * \param	length			The length.
 */

void CMultiStagedProgressProvider::CProgressStages::addStage(const std::string name, int order_number, int length)
{
	addStage(SProgressStage(name, order_number, length));
}

/*!
 * \fn	void CMultiStagedProgressProvider::CProgresStages::addStage(const SProgressStage &stage)
 *
 * \brief	Adds a stage - by filled structure
 *
 * \param	stage	The stage.
 */

void CMultiStagedProgressProvider::CProgressStages::addStage(const SProgressStage &stage)
{
	m_stages.push_back(stage);
	reorderInternal();
}

/*!
 * \fn	void CMultiStagedProgressProvider::CProgresStages::removeStage(const std::string &stage_name)
 *
 * \brief	Removes the stage described by stage_name
 *
 * \param	stage_name	Name of the stage.
 */

void CMultiStagedProgressProvider::CProgressStages::removeStage(const std::string &stage_name)
{
	auto iter = findStage(stage_name);
	if (iter != m_stages.end())
		m_stages.erase(iter);

	reorderInternal();
}

/*!
 * \fn	bool CMultiStagedProgressProvider::CProgresStages::updateStageLength(const std::string &stage_name, int length)
 *
 * \brief	Updates the stage length
 *
 * \param	stage_name	Name of the stage.
 * \param	length	  	The new length.
 *
 * \return	True if it succeeds, false if it fails (stage with given name does not exist).
 */

bool CMultiStagedProgressProvider::CProgressStages::updateStageDuration(const std::string &stage_name, int length)
{
	auto iter = findStage(stage_name);
	if (iter == m_stages.end())
		return false;

	iter->duration = length;
	reorderInternal();
	return true;
}

/*!
 * \fn	bool CMultiStagedProgressProvider::CProgresStages::isValidStage(const std::string &stage_name) const
 *
 * \brief	Query if 'stage_name' is valid stage name
 *
 * \param	stage_name	Name of the stage.
 *
 * \return	True if valid stage, false if not.
 */

bool CMultiStagedProgressProvider::CProgressStages::stageExists(const std::string &stage_name) const
{
	return findStage(stage_name) != m_stages.end();
}

/*!
 * \fn	bool CMultiStagedProgressProvider::CProgresStages::wasInitialized() const
 *
 * \brief	Determines if stages was initialized (if any stage is stored in the vector).
 *
 * \return	True if it succeeds, false if it fails.
 */

bool CMultiStagedProgressProvider::CProgressStages::wasInitialized() const
{
	return m_stages.size() > 0;
}

/*!
 * \fn	bool CMultiStagedProgressProvider::CProgresStages::getStageLimits(const std::string &stage_name, int &min, int &max) const
 *
 * \brief	Gets stage limits - minimal and maximal values of the step counter
 *
 * \param 		  	stage_name	Name of the stage.
 * \param [in,out]	min		  	The minimum.
 * \param [in,out]	max		  	The maximum.
 *
 * \return	True if it succeeds, false if it fails.
 */

bool CMultiStagedProgressProvider::CProgressStages::getStageLimits(const std::string &stage_name, int &min, int &max) const
{
	auto iter = findStage(stage_name);
	if (iter == m_stages.end())
		return false;

	int sum(0);
	std::for_each(m_stages.begin(), iter, [&sum](const SProgressStage &stage) {sum += stage.duration; });

	min = sum;
	max = sum + iter->duration;

	return true;
}

/*!
 * \fn	int CMultiStagedProgressProvider::CProgresStages::getStageLength(const std::string &stage_name) const
 *
 * \brief	Gets stage length
 *
 * \param	stage_name	Name of the stage.
 *
 * \return	The stage length.
 */

int CMultiStagedProgressProvider::CProgressStages::getStageDuration(const std::string &stage_name) const
{
	auto iter = findStage(stage_name);
	if (iter == m_stages.end())
		return 0;

	return iter->duration;
}

/*!
 * \fn	int CMultiStagedProgressProvider::CProgresStages::getLastStageOrderNumber() const
 *
 * \brief	Gets the last stage order number
 *
 * \return	The last stage order number.
 */

int CMultiStagedProgressProvider::CProgressStages::getLastStageOrderNumber() const
{
	if (m_stages.size() > 0) return m_stages.back().order_number; return 0;
}

/*!
 * \fn	int CMultiStagedProgressProvider::CProgresStages::getCompleteLength() const
 *
 * \brief	Gets summed length of all stored stages
 *
 * \return	The complete length.
 */

int CMultiStagedProgressProvider::CProgressStages::getCompleteDuration() const
{
	return m_length;
}

/**
 * Change stage order number. Use at own risk - if the same order number already exists, old
 * stage will be removed without warning...
 *
 * \param   stage_name          Name of the stage modified.
 * \param   new_order_number    Stage new order number
 */
void CMultiStagedProgressProvider::CProgressStages::changeStageOrderNumber(const std::string &stage_name, int new_order_number)
{
    auto stageIt(findStage(stage_name));
    if (stageIt == m_stages.end())
        return;

    int duration(stageIt->duration);
    removeStage(stage_name);
    addStage(stage_name, new_order_number, duration);
}

/*!
 * \fn	CMultiStagedProgressProvider::tProgressStagesVec::iterator CMultiStagedProgressProvider::CProgresStages::findStage(const std::string &name)
 *
 * \brief	Searches for the stage by name - non const version
 *
 * \param	name	The name.
 *
 * \return	The found stage.
 */

CMultiStagedProgressProvider::tProgressStagesVec::iterator CMultiStagedProgressProvider::CProgressStages::findStage(const std::string &name)
{
	const std::string &stage_name(name);
	return std::find_if(m_stages.begin(), m_stages.end(), [stage_name](const SProgressStage &s) { return s.name == stage_name; });
}

/*!
 * \fn	CMultiStagedProgressProvider::tProgressStagesVec::const_iterator CMultiStagedProgressProvider::CProgresStages::findStage(const std::string &name) const
 *
 * \brief	Searches for the stage by name - const version
 *
 * \param	name	The name.
 *
 * \return	The found stage.
 */

CMultiStagedProgressProvider::tProgressStagesVec::const_iterator CMultiStagedProgressProvider::CProgressStages::findStage(const std::string &name) const
{
	const std::string &stage_name(name);
	return std::find_if(m_stages.begin(), m_stages.end(), [stage_name](const SProgressStage &s) { return s.name == stage_name; });
}

/*!
 * \fn	void CMultiStagedProgressProvider::CProgresStages::reorder()
 *
 * \brief	Reorders stages in the vector by their order numbers
 */

void CMultiStagedProgressProvider::CProgressStages::reorderInternal()
{
	std::sort(m_stages.begin(), m_stages.end(), [](const SProgressStage &s1, const SProgressStage &s2) {return s1.order_number < s2.order_number; });

// 	DBOUT("Reordered stages: ");
// 	std::for_each(m_stages.begin(), m_stages.end(), [](const SProgressStage &s) { DBOUT(s.name.c_str() << " : " << s.order_number); });
// 	DBOUT(std::endl);

	m_length = calcOverallDuration(); // Side effect, sorry for this...
}

/*!
 * \fn	int CMultiStagedProgressProvider::CProgresStages::calcOverallLength() const
 *
 * \brief	Calculates the sum of stages lengths
 *
 * \return	The calculated maximum length.
 */

int CMultiStagedProgressProvider::CProgressStages::calcOverallDuration() const
{
	int sum(0);
	std::for_each(m_stages.begin(), m_stages.end(), [&sum](const SProgressStage &stage) {sum += stage.duration; });

	return sum;
}

/************************************************************************/
/* STRUCT SProgressSegment                                              */
/************************************************************************/

/*!
 * \fn	CMultiStagedProgressProvider::SProgressSegment::SProgressSegment()
 *
 * \brief	Default constructor
 */

CMultiStagedProgressProvider::SProgressSegment::SProgressSegment() : segment_start(0), segment_end(100)
{

}

/*!
 * \fn	int CMultiStagedProgressProvider::SProgressSegment::recalculate(int partial_count, int partial_max)
 *
 * \brief	Recalculates segment
 *
 * \param	partial_count	Number of partials.
 * \param	partial_max  	The partial maximum.
 *
 * \return	An int.
 */

int CMultiStagedProgressProvider::SProgressSegment::recalculate(int partial_count, int partial_max)
{
	if (partial_max < 0)
	{
		return partial_count;
	}
	else
	{
		float step(float(partial_count) / float(partial_max));
		int value = segment_start + (float(segment_end - segment_start) * step);
//		DBOUT("Partial count: " << partial_count << " of " << partial_max << ", segment: " << segment_start << " - " << segment_end << ", value: " << value );
		return value;
	}

	
}

/*!
 * \fn	void CMultiStagedProgressProvider::SProgressSegment::set(int min, int max)
 *
 * \brief	Sets current segment limits
 *
 * \param	min	The minimum.
 * \param	max	The maximum.
 */

void CMultiStagedProgressProvider::SProgressSegment::set(int min, int max)
{
	segment_start = min; segment_end = max;
}

/*!
 * \fn	size_t CSimpleMultistageProgressProvider::init(const tNameLengthVec &segments)
 *
 * \brief	Initializes this object by vector of segments
 *
 * \param	segments	The segments.
 *
 * \return	A number of stages added to the object.
 */

size_t CSimpleMultistageProgressProvider::init(const tNameLengthVec &segments)
{
	clear();

	size_t num_added(0);
	for (auto it = segments.begin(); it != segments.end(); ++it)
	{
		if(it->duration <= 0)
			continue;

		m_progres_stages.addStage(it->name, num_added, it->duration);
		++num_added;

		m_names.push_back(it->name);
	}

	return num_added;
}

/*!
 * \fn	bool CSimpleMultistageProgressProvider::setCurrentProgressStage(size_t stage_number)
 *
 * \brief	Sets current progress stage by the stage number (useful for automatically initialized version).
 *
 * \param	stage_number	The stage number.
 *
 * \return	True if it succeeds, false if it fails.
 */

bool CSimpleMultistageProgressProvider::setCurrentProgressStage(size_t stage_number)
{
	if (m_names.size() <= stage_number)
		return false;

	return CMultiStagedProgressProvider::setCurrentProgressStage(m_names[stage_number]);
}

/*!
 * \fn	size_t CSimpleMultistageProgressProvider::init(size_t num_automatic_stages, int length )
 *
 * \brief	Initializes this object automatically - just by number of stages and the default segment length.
 *
 * \param	num_automatic_stages	Number of automatic stages.
 * \param	length					The length.
 *
 * \return	A size_t.
 */

size_t CSimpleMultistageProgressProvider::init(size_t num_automatic_stages, int length /*= 100*/)
{
	if (length <= 0 || num_automatic_stages == 0)
		return 0;

	m_names.reserve(num_automatic_stages);
	for(size_t i = 0; i < num_automatic_stages; ++i)
	{
		std::stringstream ss;
		ss << "automatic_name_" << i;

		m_progres_stages.addStage(ss.str(), i, length);
		m_names.push_back(ss.str());
	}

	return m_names.size();
}
