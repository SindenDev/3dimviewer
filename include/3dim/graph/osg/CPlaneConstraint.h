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

#ifndef CPlaneConstraint_H
#define CPlaneConstraint_H

#include <osgManipulator/Constraint>
#include <osgManipulator/Command>

#include <VPL/Math/Base.h>


namespace manipul
{

///////////////////////////////////////////////////////////////////////////////
//! class description
//
class CSliceConstraint : public osgManipulator::Constraint
{
public:
    //! Constructor
    CSliceConstraint(osg::Node & refNode)
        : Constraint(refNode)
        , f_StartLimit(0.0f)
//        , f_EndLimit(1.0f)
        , f_EndLimit(0.999f)
    {
    }

	// Base constrain - call specialized constraining method
	virtual bool constrain( osgManipulator::MotionCommand & command ) const;

	// Constrani only translate in line commands
	virtual bool constrain(osgManipulator::TranslateInLineCommand & VPL_UNUSED(command)) const { return false; }

	void setStartLimit(float f) { f_StartLimit = f; }

	void setEndLimit(float f) { f_EndLimit = f; }

	void fixPosition(float f)
	{
		f_Position = f;
        vpl::math::limit(f_Position, f_StartLimit, f_EndLimit);
	}

protected:
	float f_StartLimit, f_EndLimit, f_Position;
};


class CSliceXYConstraint : public CSliceConstraint
{
public:
    //! Constructor
    CSliceXYConstraint(osg::Node & refNode) : CSliceConstraint(refNode) {}

    virtual bool constrain(osgManipulator::TranslateInLineCommand &command) const;
};


class CSliceXZConstraint : public CSliceConstraint
{
public:
    //! Constructor
    CSliceXZConstraint(osg::Node & refNode) : CSliceConstraint(refNode) {}

    virtual bool constrain(osgManipulator::TranslateInLineCommand &command) const;
};


class CSliceYZConstraint : public CSliceConstraint
{
public:
    //! Constructor
    CSliceYZConstraint(osg::Node & refNode) : CSliceConstraint(refNode) {}

    virtual bool constrain(osgManipulator::TranslateInLineCommand &command) const;
};


} // namespace manipul

#endif // CPlaneConstraint_H
