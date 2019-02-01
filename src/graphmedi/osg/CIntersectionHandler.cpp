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

#include <osg/CIntersectionHandler.h>

#include <osgViewer/View>


namespace osgGA
{
    bool CIntersectionHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
    {
        if (m_shouldCalcIntersectFunc(ea))
        {
            return calculateIntersection(ea, aa);
        }

        return false;
    }

    CIntersectionHandler::CIntersectionHandler()
        : m_shouldCalcIntersectFunc([](const osgGA::GUIEventAdapter&) { return false; })
    {
    }

    CIntersectionHandler::CIntersectionHandler(std::function<bool(osg::Node*)> checkNodeFunc, std::function<bool(const osgGA::GUIEventAdapter&)> shouldCalcIntersectFunc)
        : m_checkNodeFunc(checkNodeFunc)
        , m_shouldCalcIntersectFunc(shouldCalcIntersectFunc)
    {
    }

    CIntersectionHandler::IntersectSignal & CIntersectionHandler::getIntersectSignal()
    {
        return m_intersectSignal;
    }

    void CIntersectionHandler::setCheckNodeFunc(std::function<bool(osg::Node*)> checkNodeFunc)
    {
        m_checkNodeFunc = checkNodeFunc;
    }

    void CIntersectionHandler::setShouldCalcIntersectFunc(std::function<bool(const osgGA::GUIEventAdapter&)> shouldCalcIntersectFunc)
    {
        m_shouldCalcIntersectFunc = shouldCalcIntersectFunc;
    }

    bool CIntersectionHandler::calculateIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>(&aa);

        auto intersector = osg::ref_ptr<osgUtil::LineSegmentIntersector>(new osgUtil::LineSegmentIntersector(osgUtil::Intersector::PROJECTION, ea.getXnormalized(), ea.getYnormalized()));

        osgUtil::IntersectionVisitor iv(intersector);
        view->getCamera()->accept(iv);

        for (auto& intersection : intersector->getIntersections())
        {
            for (auto node : intersection.nodePath)
            {
                if (m_checkNodeFunc(node))
                {
                    m_intersectSignal.invoke(ea, intersection);

                    return true;
                }
            }

        }

        return false;
    }

} // namespace
