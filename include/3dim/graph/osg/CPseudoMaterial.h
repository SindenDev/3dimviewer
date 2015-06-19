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

#ifndef CPseudoMaterial_H
#define CPseudoMaterial_H

#include <osg/StateSet>

namespace osg
{
    class CPseudoMaterial : public osg::StateSet::Callback
    {
    protected:
        std::vector<osg::ref_ptr<osg::Uniform> > m_uniforms;

        osg::ref_ptr<osg::Uniform> m_uniDiffuse;
        osg::ref_ptr<osg::Uniform> m_uniEmission;
        osg::ref_ptr<osg::Uniform> m_uniShininess;
        osg::ref_ptr<osg::Uniform> m_uniSpecularity;

    private:
        osg::ref_ptr<osg::Shader> m_vertShader;
        osg::ref_ptr<osg::Shader> m_fragShader;
        osg::ref_ptr<osg::Program> m_program;

    public:
        CPseudoMaterial();
        ~CPseudoMaterial();

        osg::Uniform *uniform(std::string name);

    private:
        std::string compatibility();
        std::string uniforms();
        std::vector<std::pair<std::string, std::string> > vert2fragMembers();
        std::string vert2fragStruct();
        std::string vert2fragOuts();
        std::string vert2fragIns();
        std::string surfStruct();
        std::string vertShaderSrc();
        std::string insToStruct();
        std::string fragShaderSrc();

    public:
        void apply(osg::Node *node);

    private:
        void revert(osg::StateSet *stateSet);

    public:
        virtual void operator()(StateSet *stateSet, NodeVisitor *nodeVisitor);

    private:
        virtual std::string surfaceShaderSrc();
    };

    class CPseudoMaterial_Rim : public CPseudoMaterial
    {
    public:
        osg::ref_ptr<osg::Uniform> m_uniRimColor;
        osg::ref_ptr<osg::Uniform> m_uniRimIntensity;
        osg::ref_ptr<osg::Uniform> m_uniRimPower;

    public:
        CPseudoMaterial_Rim();
        ~CPseudoMaterial_Rim();

    private:
        virtual std::string surfaceShaderSrc();
    };

} // namespace osg

#endif // CPseudoMaterial_H
