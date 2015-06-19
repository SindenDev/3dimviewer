///////////////////////////////////////////////////////////////////////////////
// OpenSceneGraph compatibility header
// 
// Copyright 2008-2015 3Dim Laboratory s.r.o.
// All rights reserved
//

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

