///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// Copyright 2013 Blue Sky Bio, LLC
// All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#ifndef COnSurfaceDragger_H
#define COnSurfaceDragger_H

#include "draggers/CDraggerPlane.h"
#include <osg/IHoverDragger.h>
#include <osg/CGeometryGenerator.h>
#include <osgUtil/LineSegmentIntersector>


namespace osg
{
    class CDraggableGeometry;

    class CDraggerOnSurface : public osgManipulator::CDraggerPlane
    {
    public:
        CDraggerOnSurface(CDraggableGeometry* parent, osg::Node* geometry = new osg::CSphereGeometry());

        virtual bool handle(const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

        void setModelId(int id);

        const osgUtil::LineSegmentIntersector::Intersection& getIntersection();

    protected:
        virtual void updateCommand(osgManipulator::MotionCommand& command) override;

        bool calculateIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osgUtil::LineSegmentIntersector::Intersection& intersect);

    protected:
        osg::observer_ptr<CDraggableGeometry> m_parent;

        int m_modelId;

        osg::Vec3 m_startPoint;

        bool m_wasIntersection;

        osgUtil::LineSegmentIntersector::Intersection m_intersection;
    };
}

#endif

