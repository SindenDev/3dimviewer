///////////////////////////////////////////////////////////////////////////////
// OpenSceneGraph compatibility header
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

#ifndef OSGCOMPAT_H_INCLUDED
#define OSGCOMPAT_H_INCLUDED

#pragma once

#include <osg/Version>
#include <osg/Drawable>
#include <vector>
#include <osg/ref_ptr>
#include <osg/Geode>

#if OSG_VERSION_GREATER_OR_EQUAL(3,2,0)
	typedef std::vector< osg::ref_ptr<osg::Drawable> >	UniDrawableList;
	UniDrawableList getGeodeDrawableList(osg::Geode *pGeode);
	#define OSGGETBOUND(x)		x->getBoundingBox()
	#define OSGCOMPUTEBOUND(x)	x->computeBoundingBox()
#else
	typedef osg::Geode::DrawableList UniDrawableList;
	UniDrawableList getGeodeDrawableList(osg::Geode *pGeode);
	#define OSGGETBOUND(x)		x->getBound()
	#define OSGCOMPUTEBOUND(x)	x->computeBound()
#endif

#endif

