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

#include <base/Defs.h>
#include <osg/CCommandEventHandler.h>
#include <osg/CIntersectionProspector.h>
#include <osg/CSceneOSG.h>
//#include <osg/CDraggableGeometry.h>
#include <osg/CDraggableSlice.h>

#include <osgViewer/Viewer>
#include <osgUtil/LineSegmentIntersector>
#include <app/Signals.h>

typedef osgUtil::LineSegmentIntersector osgIntersec;


scene::CCommandEventHandler::CCommandEventHandler()
{
}


bool scene::CCommandEventHandler::handle(const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter& aa, 
                               osg::Object *o, 
                               osg::NodeVisitor *nv
                                         )
{
    osgGA::GUIEventAdapter *p_EventAdapter = const_cast<osgGA::GUIEventAdapter *>(&ea);
    osgGA::GUIActionAdapter *p_ActionAdapter = const_cast<osgGA::GUIActionAdapter*>(&aa);

    // get viewer pointer, if not possible, chicken out
    osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>(p_ActionAdapter);
    if( !view )
    {
        return false;
    }

    // get scene pointer
   osg::ref_ptr<scene::CSceneBase> scene = dynamic_cast<scene::CSceneBase *>(view->getSceneData());
    //osg::ref_ptr<scene::CSceneOSG> scene = dynamic_cast<scene::CSceneOSG *>(view->getSceneData());
    if( !scene )
    {
        return false;
    }

    switch( APP_MODE.get() )
    {
        case scene::CAppMode::MODE_TRACKBALL:
        {
            if (APP_MODE.areSceneHitHandlers())
            {
              // Desired intersections as node list
                typedef osg::CNodeTypeIntersectionDesired<osgIntersec::Intersection, scene::CSliceGeode> tDesiredList;
                tDesiredList * pDesiredList = new tDesiredList();
            
                // Initialize hit prospector
                typedef osg::CIntersectionProspector<osgIntersec::Intersection, osgIntersec::Intersections> tProspector;
                tProspector Prospector;
                Prospector.addDesiredRule(pDesiredList);
                Prospector.useDesired(true);

                // Intersections with interest geometry
                osgIntersec::Intersections AllIntersections, UsableIntersections;
                view->computeIntersections(ea.getX(), ea.getY(), AllIntersections);

                // sort intersections
    //            std::stable_sort(AllIntersections.begin(), AllIntersections.end());

                // Intersection found?
                if( !Prospector.prospect(AllIntersections, UsableIntersections) )
                     return false;

                // Calculate the current mouse position
                osg::Matrix unOrthoMatrix = osg::Matrix::inverse(scene->getOrthoTransformMatrix());
                osg::Vec3 Point = UsableIntersections.begin()->getWorldIntersectPoint() * unOrthoMatrix;

                // Find the point position within the volume data
                data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
                Point.x() = osg::Vec3::value_type(CoordConv.fromSceneX(Point.x()));
                Point.y() = osg::Vec3::value_type(CoordConv.fromSceneY(Point.y()));
                Point.z() = osg::Vec3::value_type(CoordConv.fromSceneZ(Point.z()));

                // decide what to do according to how mouse behaves
                switch( p_EventAdapter->getEventType() )
                {
                    case osgGA::GUIEventAdapter::DOUBLECLICK:
                    {
                        // Invoke the scene hit signal
                    
                        APP_MODE.getSceneHitSignal().invoke(Point.x(), Point.y(), Point.z(), scene::CAppMode::DOUBLECLICK);
                        return true;
                    }
                    default:
                        break;
                }
            }
            return false;
        }

        case scene::CAppMode::COMMAND_SCENE_ZOOM:
        {
            // decide what to do according to how mouse behaves
            switch( p_EventAdapter->getEventType() )
            {
               case osgGA::GUIEventAdapter::PUSH:
                {
                    // Center and scale the scene
                    osg::ref_ptr<osgGA::CameraManipulator> manipulator = view->getCameraManipulator();
                    if( !manipulator )
                    {
                        break;
                    }
                    manipulator->computeHomePosition();
                    manipulator->home(ea, aa);

                    // Change the mode
                    APP_MODE.restore();
                    break;
                }

               default: 
                  break;
            }
            break;
        }

        case scene::CAppMode::COMMAND_SCENE_HIT:
        {
          // Desired intersections as node list
            typedef osg::CNodeTypeIntersectionDesired<osgIntersec::Intersection, scene::CSliceGeode> tDesiredList;
            tDesiredList * pDesiredList = new tDesiredList();
            
            // Initialize hit prospector
          typedef osg::CIntersectionProspector<osgIntersec::Intersection, osgIntersec::Intersections> tProspector;
            tProspector Prospector;
           Prospector.addDesiredRule(pDesiredList);
           Prospector.useDesired(true);

           // Intersections with interest geometry
            osgIntersec::Intersections AllIntersections, UsableIntersections;
            view->computeIntersections(ea.getX(), ea.getY(), AllIntersections);

            // sort intersections
//            std::stable_sort(AllIntersections.begin(), AllIntersections.end());

            // Intersection found?
            if( !Prospector.prospect(AllIntersections, UsableIntersections) )
            {
                // note: i have disabled the line below, because the mode was always restored
                //       when axial and 3d view were displayed at once
                //APP_MODE.restore();
                 return false;
            }

           // Calculate the current mouse position
           osg::Matrix unOrthoMatrix = osg::Matrix::inverse(scene->getOrthoTransformMatrix());
         osg::Vec3 Point = UsableIntersections.begin()->getWorldIntersectPoint() * unOrthoMatrix;

            // Find the point position within the volume data
            data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
            Point.x() = osg::Vec3::value_type(CoordConv.fromSceneX(Point.x()));
            Point.y() = osg::Vec3::value_type(CoordConv.fromSceneY(Point.y()));
            Point.z() = osg::Vec3::value_type(CoordConv.fromSceneZ(Point.z()));

            // decide what to do according to how mouse behaves
            switch( p_EventAdapter->getEventType() )
            {
               case osgGA::GUIEventAdapter::PUSH:
                {
                    // Invoke the scene hit signal
                    APP_MODE.getSceneHitSignal().invoke(Point.x(), Point.y(), Point.z(), scene::CAppMode::PUSH);
                }

                case osgGA::GUIEventAdapter::DRAG:
                {
                    // Invoke the scene hit signal
                    APP_MODE.getSceneHitSignal().invoke(Point.x(), Point.y(), Point.z(), scene::CAppMode::DRAG);
                }

               case osgGA::GUIEventAdapter::RELEASE:
                {
                    // Invoke the scene hit signal
                    APP_MODE.getSceneHitSignal().invoke(Point.x(), Point.y(), Point.z(), scene::CAppMode::RELEASE);

                    // Deregister all handlers automatically
                    APP_MODE.getSceneHitSignal().disconnectAll();

                    // Change the mode
                    APP_MODE.restore();
                }
                default:
                    break;
            }
            break;
        }

        // Unknown command
        default:
            return false;
    }

    // O.K.
    return true;
}

