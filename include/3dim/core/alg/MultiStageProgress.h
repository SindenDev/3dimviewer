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

#pragma once
#ifndef MultiStageProgress_H_included
#define MultiStageProgress_H_included

#include <string>
#include <vector>
#include <VPL/Module/Progress.h>

/*!
	* \class	CMultiStagedProgressProvider
	*
	* \brief	A multi staged progress provider. This class can be used as "collector" of other progresses. To use this class, initialize it by some of the addStage methods,
	* 			connect subordinate progresses to the localProgressFunction and that's all.
	*/

class CMultiStagedProgressProvider
{
protected:

	//! One progress stage. It had its name, duration and order number.
	struct SProgressStage
	{
		SProgressStage() : name(""), order_number(0), duration(-1) {}
		SProgressStage(const std::string _name, int _order_number, int _length) : name(_name), order_number(_order_number), duration(_length) {}

		bool isValid() const { return duration > 0; }

		//! Name is stage id - setCurrentProgressStage method uses it to find correct stage
		std::string name; 
		//! Order number is used to order stages gradually
		int order_number;
		//! Duration of the stage. Sum of durations is complete length of the progress.
		int duration;
	};

	//! Vector of progress stages data
	typedef std::vector<SProgressStage> tProgressStagesVec;

	//! Progress stages limits
	class CProgressStages
	{
	public:
		//! Default constructor
		CProgressStages();

		//! Get stage identified by given name (returns default invalid stage if name is not found).
		SProgressStage getStage(const std::string &stage_name) const;

		//! Add stage to the system. If stage name is already used, stage is replaced.
		void addStage(const SProgressStage &stage);

		//! Add stage - described by parameters. If stage name is already used, stage is replaced.
		void addStage(const std::string name, int order_number, int length);

		//! Remove given stage. 
		void removeStage(const std::string &stage_name);

		//! Update stage length.
		bool updateStageDuration(const std::string &stage_name, int length);

		//! Does given stage exist?
		bool stageExists(const std::string &stage_name) const;

		//! Was this object initialised (does it contain any stages)?
		bool wasInitialized() const;

		//! Get stage limits (minimal and maximal value). Returns true if stage was found and has valid data.
		bool getStageLimits(const std::string &stage_name, int &min, int &max) const;

		//! Get length of the stage. Returns negative value if stage not found.
		int getStageDuration(const std::string &stage_name) const;

		//! Get order number of the last stage (stored stage with the highest order number).
		int getLastStageOrderNumber() const;

		//! Get sum of lengths of all stored stages.
		int getCompleteDuration() const;

        /**
         * Change stage order number. Use at own risk - if the same order number already exists, old
         * stage will be removed without warning...
         *
         * \param   stage_name          Name of the stage modified.
         * \param   new_order_number    Stage new order number
         */

        void changeStageOrderNumber(const std::string &stage_name, int new_order_number);

        //! Sort stages in the vector according to the order number.
        void reorder() 
        { 
            reorderInternal(); 
        }

        
        /** Clears this object to its blank/initial state */
        void clear() 
        { 
            m_stages.clear(); 
            m_length = 0;
        }
	private:
		//! Find stage with the given name. Returns end() of the stages vector if stage not found. Const version.
		tProgressStagesVec::const_iterator findStage(const std::string &name) const;

		//! Find stage with the given name. Returns end() of the stages vector if stage not found. 
		tProgressStagesVec::iterator findStage(const std::string &name);

		//! Sort stages in the vector according to the order number.
		void reorderInternal();

		//! Calculate sum of lengths of all stages.
		int calcOverallDuration() const;

	private:
		//! Stages
		tProgressStagesVec m_stages;

		//! Current complete length
		int m_length;

	}; // class CProgresStages

	//! Progress segment contains calculated limits (minimum and maximum of the overall progress) and contains conversion method from the local to the global progress interval
	struct SProgressSegment
	{
		//! Simple constructor
		SProgressSegment();

		//! Recalculate from the local to the global progress interval
		int recalculate(int partial_count, int partial_max);

		//! Set minimum and maximum of the segment
		void set(int min, int max);

		int segment_start;
		int segment_end;
	};

public:
	//! Simple constructor
	CMultiStagedProgressProvider() {}

	//! Destructor
	virtual ~CMultiStagedProgressProvider() {}

	//! Get progress object
	vpl::mod::CProgress &getProgressObject() { return m_progressObj; }

	//! Progress function
	virtual bool localProgressFunction(int count, int max);

protected:
	//! This method is called by localProgress on its first use
	virtual void initProgressStages() = 0;

	//! Set current progress stage by id of the stage stored in the CProgresStages object
	bool setCurrentProgressStage(const std::string &stage);

protected:
	//! Stored stages
	CProgressStages m_progres_stages;

	//! Currently used progress stage information
	SProgressSegment m_current_progress_segment;

	//! Progress object
	vpl::mod::CProgress m_progressObj;
}; // class CMultiStagedProgressProvider

/*!
	* \class	CSimpleMultistageProgressProvider
	*
	* \brief	A simple multistage progress provider - if you don't care about names and you need only simple segments (with uniform durations), this class provides it.
	*/

class CSimpleMultistageProgressProvider
	: public CMultiStagedProgressProvider
{
public:

	//! Name and duration in the same structure
	struct SNameDuration
	{
		//! Initializing constructor
		SNameDuration(const std::string &_name, int _length) : name(_name), duration(_length) {}
		//! Simple constructor. It initializes duration to the invalid value. Such a segments are omitted when used for initialization. 
		SNameDuration() : name(""), duration(-1) {}

		std::string name;
		int duration;
	};

	//! Vector of names and durations
	typedef std::vector<SNameDuration> tNameLengthVec;

public:
	//! Simple constructor
	CSimpleMultistageProgressProvider() {}

	//! Destructor
	virtual ~CSimpleMultistageProgressProvider() {}

	//! This constructor initializes object with vector of segments (names and durations)
	CSimpleMultistageProgressProvider(const tNameLengthVec &segments)
	{
		init(segments);
	}

	//! This constructor initializes object with number of segments with automatically generated names and uniformly set durations
	CSimpleMultistageProgressProvider(size_t num_of_stages, int default_length)
	{
		init(num_of_stages, default_length);
	}

	//! Initialize object with vector of segments (names and durations). Returns number of stages used (stages with zero or negative duration are not stored).
	size_t init(const tNameLengthVec &segments);

	//! Initializes object with number of segments with automatically generated names and uniformly set durations. Returns number of stages created.
	size_t init(size_t num_automatic_stages, int length = 100);

	/*!
		* \fn	bool CSimpleMultistageProgressProvider::setCurrentProgressStage(size_t stage_number);
		*
		* \brief	Sets current progress stage using its position in the vector of stages (not the order number). Useful for automatically generated stages.
		*
		* \param	stage_number	The stage position in the stages vector..
		*
		* \return	True if it succeeds, false if it fails (given stage is not found).
		*/
	bool setCurrentProgressStage(size_t stage_number);

protected:
	//! This method is called by localProgress on its first use. Does nothing here. All should be done by constructor or init method.
	virtual void initProgressStages() { }

private:
	//! Clear internal data
	virtual void clear() { m_names.clear(); }

private:
	//! Vector of strings type
	typedef std::vector<std::string> tStringVec;

	//! Stored stages names - used for setCurrentProgressStage(number) method.
	tStringVec m_names;

};// class CSimpleMultistageProgressProvider

// MultiStageProgress_H_included
#endif
