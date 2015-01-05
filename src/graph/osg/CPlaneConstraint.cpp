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

#include "osg/CPlaneConstraint.h"


namespace manipul
{

bool CSliceXYConstraint::constrain(osgManipulator::TranslateInLineCommand &command) const
{
	osg::Vec3 v = command.getTranslation();

    float fNewPosition = f_Position + v[2];

    if( fNewPosition < f_StartLimit )
    {
        v[2] = f_StartLimit - f_Position;
    }
    else if( fNewPosition > f_EndLimit )
    {
        v[2] = f_EndLimit - f_Position;
    }

	command.setTranslation(v);

	return true;
}

bool CSliceXZConstraint::constrain(osgManipulator::TranslateInLineCommand &command) const
{
	osg::Vec3 v = command.getTranslation();

    float fNewPosition = f_Position + v[1];

    if( fNewPosition < f_StartLimit )
    {
        v[1] = f_StartLimit - f_Position;
    }
    else if( fNewPosition > f_EndLimit )
    {
        v[1] = f_EndLimit - f_Position;
    }

	command.setTranslation(v);

	return true;
}


bool CSliceYZConstraint::constrain(osgManipulator::TranslateInLineCommand &command) const
{
	osg::Vec3 v = command.getTranslation();

    float fNewPosition = f_Position + v[0];

    if( fNewPosition < f_StartLimit )
    {
        v[0] = f_StartLimit - f_Position;
    }
    else if( fNewPosition > f_EndLimit )
    {
        v[0] = f_EndLimit - f_Position;
    }

    command.setTranslation(v);

	return true;
}



bool CSliceConstraint::constrain( osgManipulator::MotionCommand & command ) const
{
	try
	{
		osgManipulator::TranslateInLineCommand & c( dynamic_cast< osgManipulator::TranslateInLineCommand &>(command ));
		return constrain( c );
	}
	catch (std::bad_cast &)
	{
		return false;
	}
	return false;
}

} // namespace manipul
