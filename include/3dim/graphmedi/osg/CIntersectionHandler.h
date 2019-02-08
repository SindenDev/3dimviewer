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

#ifndef CIntersectionHandler_H
#define CIntersectionHandler_H

#include <osgGA/GUIEventHandler>
#include <osgUtil/LineSegmentIntersector>
#include <VPL/Module/Signal.h>


namespace osgGA
{
    class CIntersectionHandler : public GUIEventHandler
    {
    public:
        typedef vpl::mod::CSignal<void, const osgGA::GUIEventAdapter&, const osgUtil::LineSegmentIntersector::Intersection&> IntersectSignal;

        CIntersectionHandler();
        CIntersectionHandler(std::function<bool(osg::Node*)> checkNodeFunc, std::function<bool(const osgGA::GUIEventAdapter&)> shouldCalcIntersectFunc);

        IntersectSignal& getIntersectSignal();

        void setCheckNodeFunc(std::function<bool(osg::Node*)> checkNodeFunc);
        void setShouldCalcIntersectFunc(std::function<bool(const osgGA::GUIEventAdapter&)> shouldCalcIntersectFunc);

        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* o, osg::NodeVisitor* nv) final;

    private:
        bool calculateIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    protected:
        std::function<bool(osg::Node*)> m_checkNodeFunc;
        std::function<bool(const osgGA::GUIEventAdapter&)> m_shouldCalcIntersectFunc;

    private:
        IntersectSignal m_intersectSignal;
    };
}

#endif
