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

#include <osg/Constraints.h>

using namespace osgManipulator;


bool CLineConstraint::constrain(TranslateInLineCommand &command) const
{
	osg::Vec3 direction(command.getLineEnd() - command.getLineStart());

	osg::Vec3d localTranslatedPoint = ((command.getLineStart() + command.getTranslation())
		* command.getLocalToWorld() * getWorldToLocal());

	osg::Vec3 ls(command.getLineStart());
	osg::Vec3 le(command.getLineEnd());

	int i;
	for(i = 0; i < 3; i++)
		if(direction[i] != 0.0)
			break;

	if(direction[i] == 0.0)
		return true;

	double divided(localTranslatedPoint[i] / direction[i]);

	if(divided > 1.0)
		command.setTranslation(le * getLocalToWorld() * command.getWorldToLocal());

	if(divided < 0.0)
		command.setTranslation(ls * getLocalToWorld() * command.getWorldToLocal());

	return true;
}
