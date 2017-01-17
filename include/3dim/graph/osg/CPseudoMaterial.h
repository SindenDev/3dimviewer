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

#ifndef CPseudoMaterial_H
#define CPseudoMaterial_H

#include <osg/StateSet>

namespace osg
{
    class CAttribute
    {
    public:
        enum EAttributeType
        {
            EAT_VEC4 = 0,
            EAT_VEC3,
            EAT_VEC2,
            EAT_FLOAT,
        };

    public:
        int location;
        std::string name;
        EAttributeType type;

    public:
        CAttribute(int location, std::string name, EAttributeType type)
            : location(location)
            , name(name)
            , type(type)
        { }
        ~CAttribute()
        { }
        static std::string getTypename(EAttributeType type)
        {
            switch (type)
            {
            case EAT_VEC4:
                return "vec4";
                break;

            case EAT_VEC3:
                return "vec3";
                break;

            case EAT_VEC2:
                return "vec2";
                break;

            case EAT_FLOAT:
                return "float";
                break;

            default:
                return "";
                break;
            }
        }
    };

    class CPseudoMaterial : public osg::StateSet::Callback
    {
    protected:
        std::set<osg::ref_ptr<osg::Object> > m_objects;

        std::vector<osg::ref_ptr<osg::Uniform> > m_uniforms;
        std::vector<osg::ref_ptr<osg::Uniform> > m_internalUniforms;
        std::vector<osg::CAttribute> m_attributes;

        osg::ref_ptr<osg::Uniform> m_uniDummy;

        osg::ref_ptr<osg::Uniform> m_uniDiffuse;
        osg::ref_ptr<osg::Uniform> m_uniEmission;
        osg::ref_ptr<osg::Uniform> m_uniShininess;
        osg::ref_ptr<osg::Uniform> m_uniSpecularity;

    protected:
        osg::ref_ptr<osg::Shader> m_vertShader;
        osg::ref_ptr<osg::Shader> m_geomShader;
        osg::ref_ptr<osg::Shader> m_fragShader;
        osg::ref_ptr<osg::Program> m_program;

        bool m_twoSided;
        bool m_flatShading;
        bool m_dirty;

    public:
        CPseudoMaterial(bool twoSided = false, bool flatShading = false);
        ~CPseudoMaterial();

        osg::Uniform *uniform(std::string name);

        void setFlatShading(bool flatShading);
        void makeDirty();

        void copyInternals(CPseudoMaterial *other);

    protected:
        virtual std::string programName() { return "// CPseudoMaterial"; }
        virtual std::string vertVersion();
        virtual std::string geomVersion();
        virtual std::string fragVersion();
        virtual std::string uniforms();
        virtual std::string attributes();
        virtual std::vector<std::pair<std::string, std::string> > shaderInOuts();
        virtual std::string vertOuts();
        virtual std::string geomIns();
        virtual std::string geomOuts();
        virtual std::string fragIns();
        virtual std::string fragStruct();
        virtual std::string surfStruct();
        virtual std::string vertShaderSrc();
        virtual std::string insToStruct();
        virtual std::string fragShaderSrc();
        virtual std::string geomShaderSrc();

    public:
        virtual void apply(osg::Object *object, osg::Node *shaderTesterNode = NULL);

        //! you really should know what you're doing if you try to call this
        void revert(osg::Object *object);

    public:
        virtual void operator()(StateSet *stateSet, NodeVisitor *nodeVisitor);

    protected:
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

    protected:
        virtual std::string programName() { return "// CPseudoMaterial_Rim"; }
        virtual std::string surfaceShaderSrc();
    };

} // namespace osg

#endif // CPseudoMaterial_H
