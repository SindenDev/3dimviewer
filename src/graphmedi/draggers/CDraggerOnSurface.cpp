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

#include <draggers/CDraggerOnSurface.h>

#include <osg/CDraggableGeometry.h>
#include "osg/CModelVisualizer.h"

osg::CDraggerOnSurface::CDraggerOnSurface(CDraggableGeometry* parent, osg::Node* geometry/* = new osg::CSphereGeometry()*/)
    : CDraggerPlane(geometry, osg::Plane(0.0, 0.0, 1.0, 0.0))
    , m_wasIntersection(false)
    , m_parent(parent)
    , m_modelId(-1)
{
}

void osg::CDraggerOnSurface::setModelId(int id)
{
    m_modelId = id;
}

const osgUtil::LineSegmentIntersector::Intersection& osg::CDraggerOnSurface::getIntersection()
{
    return m_intersection;
}

bool osg::CDraggerOnSurface::handle(const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    m_wasIntersection = false;

    if (m_modelId != -1)
    {
        if (calculateIntersection(ea, aa, m_intersection))
        {
            m_wasIntersection = true;
        }

        if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
        {
            m_startPoint = m_parent->getGeometryPositionMatrix().getTrans();
        }
    }

    return osgManipulator::CDraggerPlane::handle(pi, ea, aa);
}

void osg::CDraggerOnSurface::updateCommand(osgManipulator::MotionCommand& command)
{
    if (!m_wasIntersection)
    {
        return;
    }

    osgManipulator::TranslateInPlaneCommand* cmd(dynamic_cast<osgManipulator::TranslateInPlaneCommand*>(&command));

    cmd->setLocalToWorldAndWorldToLocal(osg::computeLocalToWorld(m_intersection.nodePath), osg::computeWorldToLocal(m_intersection.nodePath));
    cmd->setTranslation(m_intersection.getLocalIntersectPoint() - m_startPoint);
    cmd->setReferencePoint(m_startPoint);
}

bool osg::CDraggerOnSurface::calculateIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osgUtil::LineSegmentIntersector::Intersection & intersect)
{
    osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View*>(&aa);

    auto intersector = osg::ref_ptr<osgUtil::LineSegmentIntersector>(new osgUtil::LineSegmentIntersector(osgUtil::Intersector::PROJECTION, ea.getXnormalized(), ea.getYnormalized()));

    osgUtil::IntersectionVisitor iv(intersector);
    view->getCamera()->accept(iv);

    for (auto& intersection : intersector->getIntersections())
    {
        for (auto node : intersection.nodePath)
        {
            auto modelVisualizer = dynamic_cast<osg::CModelVisualizer*>(node);

            if (modelVisualizer && modelVisualizer->getId() == m_modelId)
            {
                intersect = intersection;

                return true;
            }
        }
    }

    return false;
}
