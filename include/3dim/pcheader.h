//3DIM precompiled header - modify we uttermost caution - it is shared by all projects!

#ifndef PCHHEADER_H
#define PCHHEADER_H

#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER

#ifndef Q_MOC_RUN

#ifdef USED_LIB_GLEW
  // GLEW must be included first!
  #ifdef __APPLE__
    #include <glew.h>
  #else
    #include <GL/glew.h>
  #endif
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

#ifdef USED_LIB_EIGEN
  #include <Eigen/Core>
  #include <Eigen/Geometry>
  #include <Eigen/StdVector>
  #include <Eigen/src/StlSupport/StdVector.h>
#endif

#ifdef USED_LIB_VPL
  #include <VPL/Base/BasePtr.h>
  #include <VPL/Base/Lock.h>
  #include <VPL/Base/SharedPtr.h>
  #include <VPL/Math/Base.h>
  #include <VPL/Math/Matrix.h>
  #include <VPL/Math/StaticVector.h>
  #include <VPL/Math/TransformMatrix.h>
  #include <VPL/Math/Quaternion.h>
  #include <VPL/Math/Vector.h>
  #include <VPL/Math/MatrixFunctions.h>
  #include <VPL/Image/Volume.h>
  #include <VPL/Module/Serializer.h>
  #include <VPL/Module/Signal.h>  
  #include <VPL/Module/GlobalSignal.h>
#endif

#ifdef USED_LIB_OSG
  #include <osg/Version>
  #include <osg/Group>
  #include <osg/Matrix>
  #include <osg/Switch>
  #include <osg/Node>
  #include <osg/Geode>
  #include <osg/Drawable>
  #include <osg/StateSet>
  #include <osg/Vec3f>
  #include <osg/Geometry>
  #include <osg/Array>
  #include <osg/MatrixTransform>
  #include <osg/Texture2D>
  #include <osgManipulator/Dragger>
#endif

#ifdef USED_LIB_OPENMESH
  #ifndef _USE_MATH_DEFINES
    #define _USE_MATH_DEFINES
  #endif
  #ifndef OM_STATIC_BUILD
    #define OM_STATIC_BUILD
  #endif
  #include <OpenMesh/Core/IO/MeshIO.hh>
  #include <OpenMesh/Core/Mesh/TriMeshT.hh>
  #include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#endif 

#ifdef USED_LIB_QT
  #include <QApplication>
  #include <QWidget>
  #include <QPushButton>
  #include <QLabel>
  #include <QString>
  #include <QMessageBox>
  #include <QDialog>
  #include <QVariant>
  #include <QList>
  #include <QAbstractItemModel>
  #include <QModelIndex>
  #include <QStandardItem>
  #include <QStandardItemModel>
  #include <QSettings>
  #include <QByteArray> // found in moc generated files
  #include <QMetaType>  // found in moc generated files
#endif

#ifdef USED_LIB_3DIMCORE
  #include <data/CStorageEntry.h>
  #include <data/CObjectPtr.h>
  #include <data/CObjectHolder.h>
  #include <data/CEntryPtr.h>
  #include <data/CDataStorage.h>
  #include <data/CGeneralObjectObserver.h>
#endif

#ifdef USED_LIB_3DIMGEOMETRY
  #include <geometry/base/types.h>
  #include <geometry/base/OMMesh.h>
  #include <geometry/base/CBaseMesh.h>
  #include <geometry/base/CMesh.h>  
#endif

#ifdef USED_LIB_3DIMGRAPH
  // #include <osg/CTriMesh.h>
  // #include <osg/CDraggableGeometry.h>
  #include <osg/CGeneralObjectObserverOSG.h>
#endif

#ifdef USED_LIB_3DIMGUIQT
  #include <osg/OSGCanvas.h>
#endif

#endif // !Q_MOC_RUN
#endif // PCHHEADER_H