#include "osg/CFreeModelVisualizer.h"
#include <osg/CPseudoMaterial.h>
#include <osg/CTriMesh.h>
#include <osg/COnOffNode.h>
#include "geometry/base/CMesh.h"
#include <osg/osgcompat.h>
#include <osgManipulator/Dragger>


///////////////////////////////////////////////////////////////////////////////
// Model visualizer

osg::CFreeModelVisualizer::CFreeModelVisualizer(geometry::CMesh* pMesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &textures, const osg::Vec4 &diffuse, const osg::Vec3 &emission)
    : COnOffNode()
{
    m_color = diffuse;
    m_emission = emission;

    // Create a new surface mesh
    m_pTriMesh = new CTriMesh();
    addChild(m_pTriMesh);

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    m_pTriMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    m_pTriMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    // Set model color
    m_pTriMesh->setColor(m_color);
    for (auto material : m_pTriMesh->getMaterials())
    {
        material.second->uniform("Emission")->set(m_emission);
    }

    m_pTriMesh->setNodeMask(1);

    setMesh(pMesh, textures);
}

void osg::CFreeModelVisualizer::show(bool bShow)
{
    setOnOffState(bShow);
}

geometry::CMesh* osg::CFreeModelVisualizer::getMesh()
{
    return m_pMesh.get();
}

osg::observer_ptr<osg::CTriMesh> osg::CFreeModelVisualizer::getTriMesh()
{
    return m_pTriMesh;
}

void osg::CFreeModelVisualizer::setMesh(geometry::CMesh* pMesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &textures)
{
    if (!pMesh)
    {
        return;
    }

    m_pMesh = std::make_unique<geometry::CMesh>(*pMesh);

    m_pTriMesh->createMesh(*pMesh, textures, true);
}
