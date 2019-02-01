#ifndef OSG_TREE_ANALYSER_H
#define OSG_TREE_ANALYSER_H


#include <ciso646>

#include <osg/NodeVisitor>
#include <osg/Node>

#include <osgSim/DOFTransform>
#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <memory>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace osg {
    class Billboard;
    class ClearNode;
    class ClipNode;
    class CoordinateSystemNode;
    class Geode;
    class Group;
    class LightSource;
    class LOD;
    class MatrixTransform;
    class OccluderNode;
    class OcclusionQueryNode;
    class PagedLOD;
    class PositionAttitudeTransform;
    class Projection;
    class ProxyNode;
    class Sequence;
    class Switch;
    class TexGenNode;
    class Transform;
    class Camera;
    class CameraView;
    class Drawable;
    class Geometry;


    class Log {

    public:
        Log(std::string filename);
        ~Log();

        std::ofstream logFile;
        std::string name;


        void append(std::string msg, bool print = false);
        void done();

    };

    //every type is ultimately a Node
    class nodeWrapper {

    public:
        nodeWrapper() {
            my_index = index;
            index++;
        }

        static int index;

        //connectivity info
        int my_index = -1;
        std::vector<int> parent_indices; 
        std::vector<int> child_indices;
        std::string class_string;

        bool hasChildren = false;

        //stringyfied properties of object..
        std::vector<std::string> properties;

    };

    /*

   some naming rules...

   <TAG att="attribute"> where 'attribute' is 'node' or 'ignore'

   node - if the TAG is not recognized by the application it will add ui element for it
            It is used to sort contained attributes into the ui elements

   <TAG att="attribute" type="type">    where attribute is 'data' or 'label'

    data - expected type is 'plain' or 'nested'
        'plain' - the data is directly between start and end tags. It is default general case.
        'nested' - there is another tag WITH IGNORE ATTRIBUTE inside it and between that tag is the data..
            There can be more ignored tags inside in sequence.
            It is SPECIAL case for matrices and won't work with anything else without explicit support inside the app.

    label - the expected type is 1 or 2
        This number selects the style for that label later in ui
        1 - has separator
        2 - has not

    Every unique TAG must have unique name.  That holds also for data tags.
    Data values are identified by its TAG so different data with the same tag name
    will be displayed together.

    */


    class OSGTreeAnalyser : public NodeVisitor {
    public:

        OSGTreeAnalyser();
        OSGTreeAnalyser(std::string filename);

        ~OSGTreeAnalyser();

        std::unique_ptr<Log> dump_file;

        //all the virtual apply variants
        //inside it is called apply of the base class too.. traverse is called only in apply of base class Node
        void apply(Drawable& node) override;
        void apply(Geometry& node) override;

        void apply(Node& node) override;

        void apply(Geode& node) override;
        void apply(Billboard& node) override;

        void apply(Group& node) override;

        void apply(ProxyNode& node) override;

        void apply(Projection& node) override;

        void apply(CoordinateSystemNode& node) override;

        void apply(ClipNode& node) override;
        void apply(TexGenNode& node) override;
        void apply(LightSource& node) override;

        void apply(Transform& node) override;
        void apply(Camera& node) override;
        void apply(CameraView& node) override;
        void apply(MatrixTransform& node) override;
        void apply(PositionAttitudeTransform& node) override;

        void apply(Switch& node) override;
        void apply(Sequence& node) override;
        void apply(LOD& node) override;
        void apply(PagedLOD& node) override;
        void apply(ClearNode& node) override;
        void apply(OccluderNode& node) override;
        void apply(OcclusionQueryNode& node) override;


    private:

        //the type T must have getNumChildren method
        //checks for pre_traverse and if true adds node to list and if has some children adds itself 
        //to parent stack
        template<typename T>
        void addNode(T &node);

        std::vector<osg::nodeWrapper> nodes;

        //holds parent id and address in string form of node which is used to check for cycles..
        std::stack<std::pair<int, std::string>> parent_stack;
        std::stack<std::string> tag_stack;

        //base value is 2 because the first two elements are not taken into consideration during traversal..
        int level = 2;

        // flag to identify case of multiple apply on one object.. and be able to do particular action only once
        bool pre_traverse = true;



        //http://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
        template< typename T >
        std::string int_to_hex(T i) {
            std::stringstream stream;
            stream << std::setfill('0') << std::setw(sizeof(T) * 2)
                << std::hex << std::showbase << i;
            return stream.str();
        }

        static std::string nan_string;
        static std::string infinity_string;

        //translate functions are located in OSGTeeAnalyserTranslations.cpp
        template <class T1> std::string translateFloat(const T1& number);
        template <class T2> std::string translateDouble(const T2& number);
        std::string translateBool(bool value);
        std::string translateBoolToOnOff(bool value);
        std::string strigifyMatrix(osg::Matrixd matrix);
        std::string stringifyVec4(osg::Vec4 vec);
        
        std::string translateReferenceFrame(osg::Transform::ReferenceFrame reference_frame);
        std::string translateBinding(osg::Array::Binding binding);
        std::string translatePrimitiveSetMode(/*osg::PrimitiveSet::Mode*/ GLenum mode);
        std::string translateStateAttributeType(/*osg::StateAttribute::Type*/int attribute);
        std::string translateStateAttributeMode(/*osg::StateAttribute::GLMode*/ GLenum mode);
        std::string translateModeValue(/*osg::StateAttribute::Values*/unsigned int value_mask);
        std::string translateRenderingHint(/*osg::StateSet::RenderingHint*/int hint);
        std::string translateRenderbinMode(osg::StateSet::RenderBinMode mode);
        std::string translateTextureFilterMode(/*osg::Texture::FilterMode*/int mode);
        std::string translateInternalFormatMode(int mode);
        std::string translateTextureFormatType(int type);
        std::string translateTextureFormat(int format);
        std::string translateTextureWrapMode(/*osg::Texture::WrapMode*/int mode);
        std::string translateCullMode(int value); 
        std::string translateRenderOrder(/*osg::Camera::RenderOrder*/int value);
        std::string translateTransformOrder(/*osg::Camera::TransformOrder*/int value);
        std::string translateBufferComponent(/*osg::Camera::BufferComponent*/int value);


                
    };





} //namespace osg

#endif

