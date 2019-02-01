
#include <osg/OSGTreeAnalyser.h>

#include <osg/Billboard>
#include <osg/ClearNode>
#include <osg/ClipNode>
#include <osg/CoordinateSystemNode>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/OccluderNode>
#include <osg/OcclusionQueryNode>
#include <osg/PagedLOD>
#include <osg/PositionAttitudeTransform>
#include <osg/Projection>
#include <osg/ProxyNode>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/TexGenNode>
#include <osg/Transform>
#include <osg/Camera>
#include <osg/CameraView>



#include <qdebug.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>

#ifdef __APPLE__
#if __cplusplus < 201300
namespace std {
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args)
  {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif
#endif

int osg::nodeWrapper::index = 0;

/*
void analyseGeode(osg::Geode *geode);

void analysePrimSet(osg::PrimitiveSet*prset, const osg::Vec3Array *verts);

void analyse(osg::Node *nd) {
    /// here you have found a group.
    osg::Geode *geode = dynamic_cast<osg::Geode *> (nd);
    if (geode) { // analyse the geode. If it isnt a geode the dynamic cast gives NULL.
        analyseGeode(geode);
    } else {
        osg::Group *gp = dynamic_cast<osg::Group *> (nd);
        if (gp) {
            osg::notify(osg::WARN) << "Group " << gp->getName() << std::endl;
            for (unsigned int ic = 0; ic<gp->getNumChildren(); ic++) {
                analyse(gp->getChild(ic));
            }
        } else {
            osg::notify(osg::WARN) << "Unknown node " << nd << std::endl;
        }
    }
}
// divide the geode into its drawables and primitivesets:

void analyseGeode(osg::Geode *geode) {
    for (unsigned int i = 0; i<geode->getNumDrawables(); i++) {
        osg::Drawable *drawable = geode->getDrawable(i);
        osg::Geometry *geom = dynamic_cast<osg::Geometry *> (drawable);
        for (unsigned int ipr = 0; ipr<geom->getNumPrimitiveSets(); ipr++) {
            osg::PrimitiveSet* prset = geom->getPrimitiveSet(ipr);
            osg::notify(osg::WARN) << "Primitive Set " << ipr << std::endl;
            analysePrimSet(prset, dynamic_cast<const osg::Vec3Array*>(geom->getVertexArray()));
        }
    }
}

void analysePrimSet(osg::PrimitiveSet*prset, const osg::Vec3Array *verts) {
    unsigned int ic;
    unsigned int i2;
    unsigned int nprim = 0;
    osg::notify(osg::WARN) << "Prim set type " << prset->getMode() << std::endl;
    for (ic = 0; ic<prset->getNumIndices(); ic++) { // NB the vertices are held in the drawable -
        osg::notify(osg::WARN) << "vertex " << ic << " is index " << prset->index(ic) << " at " <<
            (*verts)[prset->index(ic)].x() << "," <<
            (*verts)[prset->index(ic)].y() << "," <<
            (*verts)[prset->index(ic)].z() << std::endl;
    }
    // you might want to handle each type of primset differently: such as:
    switch (prset->getMode()) {
    case osg::PrimitiveSet::TRIANGLES: // get vertices of triangle
        osg::notify(osg::WARN) << "Triangles " << nprim << " is index " << prset->index(ic) << std::endl;
        for (i2 = 0; i2<prset->getNumIndices() - 2; i2 += 3) {
        }
        break;
    case osg::PrimitiveSet::TRIANGLE_STRIP: // look up how tristrips are coded
        break;
        // etc for all the primitive types you expect. EG quads, quadstrips lines line loops....
    }
}*/


/*
void MyVisitor::apply(osg::Geode& node) {
    addNode(node);

    nodes.back().properties.push_back("osg::Geode";
    osg::StateSet* state_set = node.getStateSet();
    if (state_set) {
        handleStateSet(stateset);
    }
    for (unsigned int i = 0; i<node.getNumDrawables(); ++i) {
        osg::Drawable* drawable = node.getDrawable(i);
        if (drawable && drawable->getStateSet()) {
            handleStateSet(drawable->getStateSet())
        }
    }
}

Generic nodes :

void MyVisitor::apply(osg::Node& node) {
    addNode(node);

    nodes.back().properties.push_back("osg::Node";
    osg::StateSet* state_set = node.getStateSet();
    if (state_set) {
        handleStateSet(stateset);
    }
    traverse(node);
}

Now for the handlestateset :
void MyVisitor::handleStateSet(osg::StateSet* stateset) {
    for (unsigned int i = 0; i < stateset.getTextureAttributeList().size(); ++i) {
        osg::Texture* texture =
            dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i, osg::StateAttribute::TEXTURE));
        if (texture) {
            for (unsigned int im = 0; im < texture->getNumImages(); ++im) {
                osg::Image* image = texture->getImage(im);
                //Now you can get the image's filename etc. 
                //image->getFileName 
                //and dump it with osgDB::writeImageFile ... 
            }

        }
    }
}*/

//osg::StateAttribute
/* {
osg::Texture
osg::FrameBufferObject
osgText::Font::GlyphTexture
osg::VertexProgram
osg::FragmentProgram
osg::TexEnvCombine
osg::Light
osg::Stencil
osg::Texture3D
osg::StencilTwoSided
osg::BlendFunc
osg::TextureCubeMap
osg::Texture2D
osg::TextureRectangle
osg::Texture1D
osg::Viewport
osg::Texture2DArray
osg::ClipPlane
osg::Fog
osg::Depth
osg::Scissor
osg::BlendEquation
osg::Multisample
osg::Material
osg::TexGen
osg::AlphaFunc
osg::ColorMask
osg::LogicOp
osg::Point
osg::ClampColor
osg::LightModel
osg::TexEnv
osg::Texture2DMultisample
osg::TexMat
osg::PolygonMode
osg::Hint
osg::BlendColor
osg::CullFace
osg::PolygonOffset
osg::LineStipple
osg::Program
osg::PointSprite
osg::ShadeModel
osg::TexEnvFilter
osg::ColorMatrix
osg::FrontFace
osg::LineWidth
osg::PolygonStipple
and osgFX::Validator
}*/



/*
Opens file for appending.
*/
osg::Log::Log(std::string filename) {
    name = filename;

    logFile.open(name, std::ios::app);

}

/*
Just closes file if it's open.
*/
osg::Log::~Log() {

    if (logFile.is_open())
        logFile.close();

}

/*
If file is closed it is opened and message is appended.
*/
void osg::Log::append(std::string msg, bool print) {

    if (not logFile.is_open())
        logFile.open(name, std::ios::app);


    if (logFile.is_open())
        logFile << msg;

    if (print)
        std::cout << msg;

}


/*
Closes file. for when you won't be needing the file for a while.
*/
void osg::Log::done() {

    logFile.close();
}

osg::OSGTreeAnalyser::OSGTreeAnalyser() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{

    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //create dump file with unique timestamp

    dump_file = std::make_unique<osg::Log>(std::to_string(time)+"-osg.txt");

    //reset static index for multiple passes..
    nodeWrapper::index = 0;
}


osg::OSGTreeAnalyser::OSGTreeAnalyser(std::string filename) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {


    dump_file = std::make_unique<osg::Log>(filename);

}


osg::OSGTreeAnalyser::~OSGTreeAnalyser() {

    //base value is 2
    level = 0;

    dump_file->append(std::string(level++, '\t') + "<osgTree att=\"ignore\">\n");
    dump_file->append(std::string(level, '\t') + "<nodeCount att=\"meta\">" + std::to_string(nodes.size()) + "</nodeCount>\n");


    for (auto &node : nodes) {  

        //if(node.parent_indices.size() > 1)
        //do something

        dump_file->append(std::string(level++, '\t') + "<General att=\"node\">\n");

        dump_file->append(std::string(level, '\t') + "<id att=\"data\" type=\"plain\">" + std::to_string(node.my_index) + "</id>\n");

        //finalize node properties and remove trailing space..
        //needs to be here because children put their ids to parent's vector as they are traversed
        if (node.parent_indices.size() != 0) {

            dump_file->append(std::string(level, '\t') + "<parents att=\"data\" type=\"plain\">");

            std::string parent_string = "";
            for (size_t i = 0; i < node.parent_indices.size(); ++i)
                parent_string.append(std::to_string(node.parent_indices.at(i)) + " ");


            parent_string.replace(parent_string.size() - 1, parent_string.size() - 1, "");

            dump_file->append(parent_string);
            dump_file->append("</parents>\n");
        }

        dump_file->append(std::string(level, '\t') + "<children-count att=\"data\" type=\"plain\">" + std::to_string(node.child_indices.size()) + "</children-count>\n");

        if (node.child_indices.size() != 0) {

            dump_file->append(std::string(level, '\t') + "<children att=\"data\" type=\"plain\">");

            std::string child_string = "";

            for (size_t i = 0; i < node.child_indices.size(); ++i)
                child_string.append(std::to_string(node.child_indices.at(i)) + " ");


            child_string.replace(child_string.size() - 1, child_string.size() - 1, "");
            
            dump_file->append(child_string);
            dump_file->append("</children>\n");

        }

        dump_file->append(std::string(level, '\t') + "<class att=\"data\" type=\"plain\">");
            dump_file->append(node.class_string);
        dump_file->append("</class>\n");





        //finally write all the gathered info to file..
        for (auto &property : node.properties) {

            dump_file->append(property + "\n");

        }
        dump_file->append(std::string(--level, '\t') + "</General>\n");

        dump_file->append("\n");

    }

    dump_file->append("</osgTree>");

}


template<typename T>
void osg::OSGTreeAnalyser::addNode(T &node) {

    //first time applying on given object..
    if (not pre_traverse)
        return;


    Node *baseNode = static_cast<Node *>(&node);
    std::string ptrStr = QString("0x%1").arg((quintptr)baseNode,
        QT_POINTER_SIZE * 2, 16, QChar('0')).toStdString();

    //if node finds itself in its own parents stack -> cycle..  probably doesn't even work.. idk..
    bool cycle = false;

    auto parent_stack_copy = parent_stack;

    for (int i = 0; i < parent_stack_copy.size(); ++i)
    {

        std::string item = parent_stack_copy.top().second;

        if (item.compare(ptrStr) == 0)
        {
            cycle = true;
            break;
        } 

        parent_stack_copy.pop();
    }
    
    //break the cycle
    if (cycle)
        return;



    nodes.push_back(nodeWrapper());
    nodeWrapper *current = &nodes.back();

    if (parent_stack.size() != 0){
        //current->parent_indices.push_back(-1);
    //else {
        current->parent_indices.push_back(parent_stack.top().first);
        nodes.at(parent_stack.top().first).child_indices.push_back(current->my_index);
    }


    if (node.getNumChildren() != 0) {

        //qDebug() << current->my_index << QString::fromStdString(ptrStr) << QString::fromStdString(baseNode->getName());


        parent_stack.push(std::pair<int, std::string>(current->my_index, ptrStr));
        current->hasChildren = true;
    }

    pre_traverse = false;
    
}

/** Stores a set of modes and attributes which represent a set of OpenGL state.
*  Notice that a \c StateSet contains just a subset of the whole OpenGL state.
* <p>In OSG, each \c Drawable and each \c Node has a reference to a
* \c StateSet. These <tt>StateSet</tt>s can be shared between
* different <tt>Drawable</tt>s and <tt>Node</tt>s (that is, several
* <tt>Drawable</tt>s and <tt>Node</tt>s can reference the same \c StateSet).
* Indeed, this practice is recommended whenever possible,
* as this minimizes expensive state changes in the graphics pipeline.
*/


//base case
void osg::OSGTreeAnalyser::apply(Node& node) {
    std::string name;
    if (node.getName().compare("") == 0)
        name = "no_name";
    else
        name = node.getName();

    nodeWrapper *current = &nodes.back();
    int current_node_index = nodes.size() - 1;//reallocation of vector f* with the pointer


    current->properties.push_back(std::string(level++, '\t') + "<Node att=\"node\">");
    tag_stack.push("Node");

    current->properties.push_back(std::string(level, '\t') + "<node-name att=\"data\" type=\"plain\">" + name + "</node-name>");


    //it's typedef'd from unsigned int
    current->properties.push_back(std::string(level, '\t') + "<mask att=\"data\" type=\"plain\">" + int_to_hex(node.getNodeMask()) + "</mask>");



    auto matrix_list = node.getWorldMatrices();
    if (matrix_list.size() != 0) {
        current->properties.push_back(std::string(level++, '\t') + "<world-matrices att=\"data\" type=\"nested\">");

        for (auto matrix : matrix_list) {
            current->properties.push_back(strigifyMatrix(matrix).append("\n"));
        }

        current->properties.back().replace(current->properties.back().size() - 1, current->properties.back().size() - 1, "");
        current->properties.push_back(std::string(--level, '\t') + "</world-matrices>");
    }

    auto state_set = node.getStateSet();
    
    if (state_set != nullptr) {

        current->properties.push_back(std::string(level++, '\t') + "<state-set att=\"label\" type=\"1\">");

        //uniforms aren't used???
        auto uniform_list = state_set->getUniformList();
        
        if (uniform_list.size() != 0) {
            current->properties.push_back(std::string(level++, '\t') + "<uniforms att=\"data\" type=\"plain\">");

            for (auto uniform : uniform_list) {
                uniform.first;
                uniform.second.first;
                uniform.second.second;


            }
            current->properties.push_back(std::string(--level, '\t') + "</uniforms>");
        }



        auto attribute_list = state_set->getAttributeList();
        if (attribute_list.size() != 0) {
            current->properties.push_back(std::string(level++, '\t') + "<attributes att=\"label\" type=\"2\">");

            for (auto attribute : attribute_list) {

                auto type_member_pair = attribute.first;
                auto ref_attribute_pair = attribute.second;

                auto type = translateStateAttributeType(type_member_pair.first);
                int some_number = type_member_pair.second;

                auto state_attribute = ref_attribute_pair.first;       //co s timto..? pretypovat a ziskat dalsi data?
                auto value = translateModeValue(ref_attribute_pair.second);

                current->properties.push_back(std::string(level, '\t') + "<attribute att=\"data\" type=\"plain\">" + translateStateAttributeType(state_attribute->getType()) + " " + translateModeValue(ref_attribute_pair.second) + "</attribute>");

            }
            current->properties.push_back(std::string(--level, '\t') + "</attributes>");

        }

        /** Simple pairing between an attribute type and the member within that attribute type group.*/
       // typedef std::pair<StateAttribute::Type, unsigned int> TypeMemberPair;

        /** Simple pairing between an attribute and its override flag.*/
       // typedef std::pair<ref_ptr<StateAttribute>, StateAttribute::OverrideValue>    RefAttributePair;

        /** a container to map <StateAttribyte::Types,Member> to their respective RefAttributePair.*/
      //  typedef std::map<StateAttribute::TypeMemberPair, RefAttributePair>           AttributeList;
        




        
        current->properties.push_back(std::string(level++, '\t') + "<render-bin att=\"label\" type=\"2\">");

            current->properties.push_back(std::string(level, '\t') + "<name att=\"data\" type=\"plain\">" + state_set->getBinName() + "</name>");
            current->properties.push_back(std::string(level, '\t') + "<number att=\"data\" type=\"plain\">" + std::to_string(state_set->getBinNumber()) + "</number>");
            current->properties.push_back(std::string(level, '\t') + "<mode att=\"data\" type=\"plain\">" + translateRenderbinMode(state_set->getRenderBinMode()) + "</mode>");
            current->properties.push_back(std::string(level, '\t') + "<hint att=\"data\" type=\"plain\">" + translateRenderingHint(state_set->getRenderingHint()) + "</hint>");
            current->properties.push_back(std::string(level, '\t') + "<nested att=\"data\" type=\"plain\">" + translateBool(state_set->getNestRenderBins()) + "</nested>");

        current->properties.push_back(std::string(--level, '\t') + "</render-bin>");


        auto mode_list = state_set->getModeList(); 

        if (mode_list.size() != 0) {
            current->properties.push_back(std::string(level++, '\t') + "<modes att=\"label\" type=\"2\">");

            for (auto mode : mode_list) 
                current->properties.push_back(std::string(level, '\t') + "<mode att=\"data\" type=\"plain\">" + translateStateAttributeMode(mode.first) + " " + translateModeValue(mode.second) + "</mode>");
            
            current->properties.push_back(std::string(--level, '\t') + "</modes>");
        }






        ////////////////////////////////////////////////////////////////////
        int a = state_set->getNumTextureAttributeLists();
        int b = state_set->getNumTextureModeLists();

        auto texture_attribute_list = state_set->getTextureAttributeList();
        auto tex_mode_list = state_set->getTextureModeList();



        /** Simple pairing between an attribute type and the member within that attribute type group.*/
       // typedef std::pair<StateAttribute::Type, unsigned int> TypeMemberPair;

        /** Simple pairing between an attribute and its override flag.*/
       // typedef std::pair<ref_ptr<StateAttribute>, StateAttribute::OverrideValue>    RefAttributePair;

        /** a container to map <StateAttribyte::Types,Member> to their respective RefAttributePair.*/
       // typedef std::map<StateAttribute::TypeMemberPair, RefAttributePair>           AttributeList;

        //typedef std::vector<AttributeList>  TextureAttributeList;


        for (unsigned int i = 0; i < state_set->getTextureAttributeList().size(); ++i) {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(state_set->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            if (texture) {

                
                current->properties.push_back(std::string(level++, '\t') + "<texture att=\"label\" type=\"2\">");



                    current->properties.push_back(std::string(level, '\t') + "<mag-filter att=\"data\" type=\"plain\">" + translateTextureFilterMode(texture->getFilter(osg::Texture::MAG_FILTER)) + "</mag-filter>");
                    current->properties.push_back(std::string(level, '\t') + "<min-filter att=\"data\" type=\"plain\">" + translateTextureFilterMode(texture->getFilter(osg::Texture::MIN_FILTER))  + "</min-filter>");

                    current->properties.push_back(std::string(level, '\t') + "<wrapR att=\"data\" type=\"plain\">" + translateTextureWrapMode(texture->getWrap(osg::Texture::WrapParameter::WRAP_R)) + "</wrapR>");
                    current->properties.push_back(std::string(level, '\t') + "<wrapS att=\"data\" type=\"plain\">" + translateTextureWrapMode(texture->getWrap(osg::Texture::WrapParameter::WRAP_S)) + "</wrapS>");
                    current->properties.push_back(std::string(level, '\t') + "<wrapT att=\"data\" type=\"plain\">" + translateTextureWrapMode(texture->getWrap(osg::Texture::WrapParameter::WRAP_T)) + "</wrapT>");



                    //maybe even leave out source format
                    current->properties.push_back(std::string(level++, '\t') + "<source-format att=\"label\" type=\"3\">");

                        current->properties.push_back(std::string(level, '\t') + "<format att=\"data\" type=\"plain\">" + translateTextureFormat(texture->getSourceFormat()) + "</format>");
                        current->properties.push_back(std::string(level, '\t') + "<type att=\"data\" type=\"plain\">" + translateTextureFormatType(texture->getSourceType()) + "</type>");

                    current->properties.push_back(std::string(--level, '\t') + "</source-format>");


                    current->properties.push_back(std::string(level++, '\t') + "<internal-format att=\"label\" type=\"3\">");

                        current->properties.push_back(std::string(level, '\t') + "<format att=\"data\" type=\"plain\">" + translateTextureFormat(texture->getInternalFormat()) + "</format>");
                        current->properties.push_back(std::string(level, '\t') + "<type att=\"data\" type=\"plain\">" + translateTextureFormatType(texture->getInternalFormatType()) + "</type>");
                        current->properties.push_back(std::string(level, '\t') + "<mode att=\"data\" type=\"plain\">" + translateInternalFormatMode(texture->getInternalFormatMode()) + "</mode>");
 
                    current->properties.push_back(std::string(--level, '\t') + "</internal-format>");


                    current->properties.push_back(std::string(level, '\t') + "<width att=\"data\" type=\"plain\">" + std::to_string(texture->getTextureWidth()) + "</width>");
                    current->properties.push_back(std::string(level, '\t') + "<height att=\"data\" type=\"plain\">" + std::to_string(texture->getTextureHeight()) + "</height>");
                    current->properties.push_back(std::string(level, '\t') + "<depth att=\"data\" type=\"plain\">" + std::to_string(texture->getTextureDepth()) + "</depth>");

                
                    //getNumImages return how many Images can be in this texture
                    for (unsigned int im = 0; im < texture->getNumImages(); ++im) {
                        osg::Image* image = texture->getImage(im);  //can return null if no image is present

                        if (image == nullptr)
                            continue;

                        //image->getMipmapLevels();
                        //image->getMipmapOffset();
                        //image->getNumMipmapLevels();

                       //image->getOrigin();
                        //image->isImageTranslucent();
                            current->properties.push_back(std::string(level, '\t') + "<image-file att=\"data\" type=\"plain\">" + image->getFileName() +"</image-file>");
 
                    }

                current->properties.push_back(std::string(--level, '\t') + "</texture>");

            }

        }
       

        current->properties.push_back(std::string(--level, '\t') + "</state-set>");

    }



    ///////////////////////////////////////////////

    auto data_variance = node.getDataVariance();

    while (not tag_stack.empty()) {
        current->properties.push_back(std::string(--level, '\t') + "</" + tag_stack.top() + ">");
        current->class_string.insert(0, tag_stack.top() + ", ");
        tag_stack.pop();
    }

    current->class_string.replace(current->class_string.size() - 2, current->class_string.size() - 1, "");



    //endpoint of the series of apply functions, anything past this will be on different object
    pre_traverse = true;

    traverse(node);
    

    //previous current pointer could be invalidated with vector resize..
    nodeWrapper *current_again = &nodes.at(current_node_index);


    //if node was parent, now are all its children traversed so remove it from stack
    //if (current->child_indices.size() != 0)   //this doesn't work for some reason
    if (current_again->hasChildren)
    {
        if (!parent_stack.empty())
        {
            parent_stack.pop();
        }
    }

}


//special case1
void osg::OSGTreeAnalyser::apply(Drawable& node) {
    if (pre_traverse) {

        nodes.push_back(nodeWrapper());
        auto current = &nodes.back();

        current->parent_indices.push_back(parent_stack.top().first);
        nodes.at(parent_stack.top().first).child_indices.push_back(current->my_index);

        pre_traverse = false;
    }

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Drawable att=\"node\">");
    tag_stack.push("Drawable");


    bool culling = node.getCullingActive();
    bool display_lists = node.getSupportsDisplayList();
    bool useVBOs = node.getUseVertexBufferObjects();

    nodes.back().properties.push_back(std::string(level, '\t') + "<culling-active att=\"data\" type=\"plain\">" + translateBool(culling) + "</culling-active>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<display-lists att=\"data\" type=\"plain\">" + translateBool(display_lists) + "</display-lists>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<use-VBOs att=\"data\" type=\"plain\">" + translateBool(useVBOs) + "</use-VBOs>");


    auto BB = node.getBoundingBox();

    nodes.back().properties.push_back(std::string(level++, '\t') + "<bounding-box att=\"label\" type=\"1\">");

    nodes.back().properties.push_back(std::string(level, '\t') + "<X att=\"data\" type=\"plain\">" + translateFloat(BB.xMin()) + " " + translateFloat(BB.xMax()) + "</X>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<Y att=\"data\" type=\"plain\">" + translateFloat(BB.yMin()) + " " + translateFloat(BB.yMax()) + "</Y>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<Z att=\"data\" type=\"plain\">" + translateFloat(BB.zMin()) + " " + translateFloat(BB.zMax()) + "</Z>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<center att=\"data\" type=\"plain\">" + translateFloat(BB.center().x()) + " " + translateFloat(BB.center().y()) + " " + translateFloat(BB.center().z()) + "</center>");
        nodes.back().properties.push_back(std::string(level, '\t') + "<radius att=\"data\" type=\"plain\">" + translateFloat(BB.radius()) + "</radius>");

    nodes.back().properties.push_back(std::string(--level, '\t') + "</bounding-box>");

    


    apply(static_cast<Node&>(node));
}

//special case2
void osg::OSGTreeAnalyser::apply(Geometry& node) {

    //first time applying on given object..
    if (pre_traverse) {

        nodes.push_back(nodeWrapper());
        auto current = &nodes.back();

        current->parent_indices.push_back(parent_stack.top().first);
        nodes.at(parent_stack.top().first).child_indices.push_back(current->my_index);

        pre_traverse = false;
    }
   

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Geometry att=\"node\">");
    tag_stack.push("Geometry");


    //node.getArrayList();
    auto vertexArray = node.getVertexArray();
    auto normalArray = node.getNormalArray();
    auto colorArray = node.getColorArray();

    if (vertexArray != nullptr)
        nodes.back().properties.push_back(std::string(level, '\t') + "<vertex-array-binding att=\"data\" type=\"plain\">" + translateBinding(vertexArray->getBinding()) + "</vertex-array-binding>");

    if (normalArray != nullptr)
        nodes.back().properties.push_back(std::string(level, '\t') + "<normal-array-binding att=\"data\" type=\"plain\">" + translateBinding(normalArray->getBinding()) + "</normal-array-binding>");

    if (colorArray != nullptr)
        nodes.back().properties.push_back(std::string(level, '\t') + "<color-array-binding att=\"data\" type=\"plain\">" + translateBinding(colorArray->getBinding()) + "</color-array-binding>");


    node.getVertexAttribArrayList();
    node.getTexCoordArrayList();

    auto primitive_set_list = node.getPrimitiveSetList();
    

    nodes.back().properties.push_back(std::string(level, '\t') + "<primitiveSet-count att=\"data\" type=\"plain\">" + std::to_string(primitive_set_list.size()) + "</primitiveSet-count>");

    

    for (auto primitive_set : primitive_set_list) {
        nodes.back().properties.push_back(std::string(level++, '\t') + "<primitiveSet att=\"label\" type=\"1\">");

        nodes.back().properties.push_back(std::string(level, '\t') + "<mode att=\"data\" type=\"plain\">" + translatePrimitiveSetMode(primitive_set->getMode()) + "</mode>");
        //primitive_set->getBufferIndex()
        nodes.back().properties.push_back(std::string(level, '\t') + "<indices att=\"data\" type=\"plain\">" + std::to_string(primitive_set->getNumIndices()) + "</indices>");
        nodes.back().properties.push_back(std::string(level, '\t') + "<number-of-primitives att=\"data\" type=\"plain\">" + std::to_string(primitive_set->getNumPrimitives()) + "</number-of-primitives>");

        //primitive_set->supportsBufferObject();

        nodes.back().properties.push_back(std::string(--level, '\t') + "</primitiveSet>");

    }



    apply(static_cast<Drawable&>(node));
}

//all the other cases
//////////////////////////////////////////////////////////////////////////////
void osg::OSGTreeAnalyser::apply(Geode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Geode att=\"node\">");
    tag_stack.push("Geode");

    apply(static_cast<Node&>(node)); 
}

void osg::OSGTreeAnalyser::apply(Billboard& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Billboard att=\"node\">");
    tag_stack.push("Billboard");

    apply(static_cast<Geode&>(node));
}

void osg::OSGTreeAnalyser::apply(Group& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Group att=\"node\">");
    tag_stack.push("Group");

    apply(static_cast<Node&>(node));
}

void osg::OSGTreeAnalyser::apply(ProxyNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<ProxyNode att=\"node\">");
    tag_stack.push("ProxyNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(Projection& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Projection att=\"node\">");
    tag_stack.push("Projection");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(CoordinateSystemNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<CoordinateSystemNode att=\"node\">");
    tag_stack.push("CoordinateSystemNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(ClipNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<ClipNode att=\"node\">");
    tag_stack.push("ClipNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(TexGenNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<TexGenNode att=\"node\">");
    tag_stack.push("TexGenNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(LightSource& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<LightSource att=\"node\">");
    tag_stack.push("LightSource");

    auto light = node.getLight();   //////////////////////////////////////SOME STUFF
    
    //
    auto position = light->getPosition();
    auto direction = light->getDirection();


    nodes.back().properties.push_back(std::string(level, '\t') + "<position att=\"data\" type=\"plain\">" + translateFloat(position.x()) + " " + translateFloat(position.y()) + " " + translateFloat(position.z()) + " " + translateFloat(position.w()) + "</position>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<direction att=\"data\" type=\"plain\">" + translateFloat(direction.x()) + " " + translateFloat(direction.y()) + " " + translateFloat(direction.z()) + "</direction>");


    osg::StateAttribute::Type type = light->getType();  //prob light right?
    

    //GL light index
    nodes.back().properties.push_back(std::string(level, '\t') + "<index att=\"data\" type=\"plain\">" + std::to_string(light->getLightNum()) + "</index>");



    auto ambient = light->getAmbient();
    auto diffuse = light->getDiffuse();
    auto specular = light->getSpecular();

    nodes.back().properties.push_back(std::string(level, '\t') + "<ambient att=\"data\" type=\"plain\">" +  translateFloat(ambient.x()) + " "  + translateFloat(ambient.y()) + " "  + translateFloat(ambient.z()) + " "  + translateFloat(ambient.w()) + "</ambient>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<diffuse att=\"data\" type=\"plain\">" +  translateFloat(diffuse.x()) + " "  + translateFloat(diffuse.y()) + " "  + translateFloat(diffuse.z()) + " "  + translateFloat(diffuse.w()) + "</diffuse>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<specular att=\"data\" type=\"plain\">" + translateFloat(specular.x()) + " " + translateFloat(specular.y()) + " " + translateFloat(specular.z()) + " " + translateFloat(specular.w()) + "</specular>");

    nodes.back().properties.push_back(std::string(level++, '\t') + "<attenuation att=\"label\" type=\"1\">");
        nodes.back().properties.push_back(std::string(level, '\t') + "<constant att=\"data\" type=\"plain\">"  + translateFloat(light->getConstantAttenuation()) + "</constant>");
        nodes.back().properties.push_back(std::string(level, '\t') + "<linear att=\"data\" type=\"plain\">"    + translateFloat(light->getLinearAttenuation()) + "</linear>");
        nodes.back().properties.push_back(std::string(level, '\t') + "<quadratic att=\"data\" type=\"plain\">" + translateFloat(light->getQuadraticAttenuation()) + "</quadratic>");
    nodes.back().properties.push_back(std::string(--level, '\t') + "</attenuation>");



    //osg::StateAttribute::ModeUsage usage;
    //light->getModeUsage(usage);
    
    //light->getSpotCutoff();
    //light->getSpotExponent();

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(Transform& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Transform att=\"node\">");
    tag_stack.push("Transform");

    nodes.back().properties.push_back(std::string(level, '\t') + "<rf att=\"data\" type=\"plain\">" + translateReferenceFrame(node.getReferenceFrame()) + "</rf>");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(Camera& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Camera att=\"node\">");
    tag_stack.push("Camera");


    node.containsOccluderNodes();///////////////////////////////////////////
    node.getBufferAttachmentMap(); //investigate

    nodes.back().properties.push_back(std::string(level, '\t') + "<clear-accum att=\"data\" type=\"plain\">" + stringifyVec4(node.getClearAccum()) + "</clear-accum>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<clear-color att=\"data\" type=\"plain\">" + stringifyVec4(node.getClearColor()) + "</clear-color>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<clear-depth att=\"data\" type=\"plain\">" + translateDouble(node.getClearDepth()) + "</clear-depth>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<clear-stencil att=\"data\" type=\"plain\">" + std::to_string(node.getClearStencil()) + "</clear-stencil>");

    nodes.back().properties.push_back(std::string(level, '\t') + "<clear-mask att=\"data\" type=\"plain\">" + int_to_hex(node.getClearMask()) + "</clear-mask>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<color-mask att=\"data\" type=\"plain\">" + int_to_hex(node.getColorMask()) + "</color-mask>");
    nodes.back().properties.push_back(std::string(level, '\t') + "<cull-mask att=\"data\" type=\"plain\">" + int_to_hex(node.getCullMask()) + "</cull-mask>");

    //nodes.back().properties.push_back(std::string(level, '\t') + "<TESTING-SOME-VERY-LONG-NAMESTESTING-SOME-VERY-LONG-NAMES att=\"data\" type=\"plain\">" + int_to_hex(node.getCullMask()) + "</TESTING-SOME-VERY-LONG-NAMESTESTING-SOME-VERY-LONG-NAMES>");


    //node.getCullMaskLeft(); ???

    nodes.back().properties.push_back(std::string(level, '\t') + "<cull-mode att=\"data\" type=\"plain\">" + translateCullMode(node.getCullingMode()) + "</cull-mode>");


    node.getDisplaySettings(); //investigate

   // node.getGraphicsContext()->isCurrent();   ??

    node.getLODScale();
    

    nodes.back().properties.push_back(std::string(level, '\t') + "<render-order att=\"data\" type=\"plain\">" + translateRenderOrder(node.getRenderOrder()) + "</render-order>");

    nodes.back().properties.push_back(std::string(level, '\t') + "<transform-order att=\"data\" type=\"plain\">" + translateTransformOrder(node.getTransformOrder()) + "</transform-order>");


    //node.getView()->getFrameStamp();
    auto view = node.getView();

    
    //what about nested callbacks???

    auto ClampProjectionMatrixCallback  = node.getClampProjectionMatrixCallback();
    auto ComputeBoundingSphereCallback  = node.getComputeBoundingSphereCallback();
    auto CullCallback                   = node.getCullCallback();
    auto EventCallback                  = node.getEventCallback();
    auto FinalDrawCallback              = node.getFinalDrawCallback();
    auto InitialDrawCallback            = node.getInitialDrawCallback();
    auto PostDrawCallback               = node.getPostDrawCallback();
    auto PreDrawCallback                = node.getPreDrawCallback();
    auto UpdateCallback                 = node.getUpdateCallback();

    /*
    if (ClampProjectionMatrixCallback != nullptr) { 
        nodes.back().properties.push_back(std::string(level, '\t') + "<draw-buffer att=\"data\" type=\"plain\">" + ClampProjectionMatrixCallback->className() + "</>");
        nodes.back().properties.push_back(std::string(level, '\t') + "<draw-buffer att=\"data\" type=\"plain\">" + ClampProjectionMatrixCallback->libraryName() 
    }
    */

    //if at least one is valid, then do label... avoid possible empty labels..
    if (ClampProjectionMatrixCallback != nullptr ||
        ComputeBoundingSphereCallback != nullptr ||
        CullCallback != nullptr ||
        EventCallback != nullptr ||
        FinalDrawCallback != nullptr ||
        InitialDrawCallback != nullptr ||
        PostDrawCallback != nullptr ||
        PreDrawCallback != nullptr ||
        UpdateCallback != nullptr) {



        nodes.back().properties.push_back(std::string(level++, '\t') + "<Callbacks att=\"label\" type=\"1\">");


            if (ComputeBoundingSphereCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<bounding-sphere-callback att=\"data\" type=\"plain\">" + ComputeBoundingSphereCallback->getCompoundClassName() + "</bounding-sphere-callback>");

            if (CullCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<cull-callback att=\"data\" type=\"plain\">" + CullCallback->getCompoundClassName() + "</cull-callbac>");

            if (EventCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<event-callback att=\"data\" type=\"plain\">" + EventCallback->getCompoundClassName() + "</event-callback>");

            if (FinalDrawCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<final-draw-callback att=\"data\" type=\"plain\">" + FinalDrawCallback->getCompoundClassName() + "</final-draw-callback>");

            if (InitialDrawCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<initial-draw-callback att=\"data\" type=\"plain\">" + InitialDrawCallback->getCompoundClassName() + "</initial-draw-callback>");

            if (PostDrawCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<post-draw-callback att=\"data\" type=\"plain\">" + PostDrawCallback->getCompoundClassName() + "</post-draw-callback>");

            if (PreDrawCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<pre-draw-callback att=\"data\" type=\"plain\">" + PreDrawCallback->getCompoundClassName() + "</pre-draw-callback>");

            if (UpdateCallback != nullptr)
                nodes.back().properties.push_back(std::string(level, '\t') + "<update-callback att=\"data\" type=\"plain\">" + UpdateCallback->getCompoundClassName() + "</update-callback>");

        nodes.back().properties.push_back(std::string(--level, '\t') + "</Callbacks>");

    }


    nodes.back().properties.push_back(std::string(level, '\t') + "<draw-buffer att=\"data\" type=\"plain\">" + translateBufferComponent(node.getDrawBuffer()) + "</draw-buffer>");



    
    nodes.back().properties.push_back(std::string(level, '\t') + "<draw-buffer att=\"data\" type=\"plain\">" + translateBufferComponent(node.getDrawBuffer()) + "</draw-buffer>");

    auto viewport = node.getViewport();

    if (viewport != nullptr) {

        nodes.back().properties.push_back(std::string(level++, '\t') + "<Viewport att=\"label\" type=\"1\">");

            nodes.back().properties.push_back(std::string(level, '\t') + "<X att=\"data\" type=\"plain\">" + translateDouble(viewport->x()) + "</X>");
            nodes.back().properties.push_back(std::string(level, '\t') + "<Y att=\"data\" type=\"plain\">" + translateDouble(viewport->y()) + "</Y>");

            nodes.back().properties.push_back(std::string(level, '\t') + "<width att=\"data\" type=\"plain\">" + translateDouble(viewport->width()) + "</width>");
            nodes.back().properties.push_back(std::string(level, '\t') + "<height att=\"data\" type=\"plain\">" + translateDouble(viewport->height()) + "</height>");

            nodes.back().properties.push_back(std::string(level, '\t') + "<aspect-ratio att=\"data\" type=\"plain\">" + translateDouble(viewport->aspectRatio()) + "</aspect-ratio>");

        nodes.back().properties.push_back(std::string(--level, '\t') + "</Viewport>");

    }
    //node.getWorldMatrices();  //in Node

    /////////////////////////////////////////////////////////////////LOTS OF STUFF
    apply(static_cast<Transform&>(node));
}

void osg::OSGTreeAnalyser::apply(CameraView& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<CameraView att=\"node\">");
    tag_stack.push("CameraView");

    apply(static_cast<Transform&>(node));
}

void osg::OSGTreeAnalyser::apply(MatrixTransform& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<MatrixTransform att=\"node\">");
    tag_stack.push("MatrixTransform");

    nodes.back().properties.push_back(std::string(level++, '\t') + "<transform att=\"data\" type=\"nested\">");
    
        nodes.back().properties.push_back(strigifyMatrix(node.getMatrix()));

    nodes.back().properties.push_back(std::string(--level, '\t') + "</transform>");


    apply(static_cast<Transform&>(node));
}

void osg::OSGTreeAnalyser::apply(PositionAttitudeTransform& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<PositionAttitudeTransform att=\"node\">");
    tag_stack.push("PositionAttitudeTransform");

    apply(static_cast<Transform&>(node));
}

void osg::OSGTreeAnalyser::apply(Switch& node) {
    addNode(node);
    
    nodes.back().properties.push_back(std::string(level++, '\t') + "<Switch att=\"node\">");
    tag_stack.push("Switch");


    std::string values = "";
    for (size_t i = 0; i < node.getNumChildren(); ++i)
        values.append(translateBoolToOnOff(node.getChildValue(node.getChild(i))) + " ");
    

    if (values.size() != 0) {
        //remove trailing space
        values.replace(values.size() - 1, values.size() - 1, "");

        nodes.back().properties.push_back(std::string(level, '\t') + "<child-values att=\"data\" type=\"plain\">" + values + "</child-values>");
        
    }

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(Sequence& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<Sequence att=\"node\">");
    tag_stack.push("Sequence");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(LOD& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<LOD att=\"node\">");
    tag_stack.push("LOD");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(PagedLOD& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<PagedLOD att=\"node\">");
    tag_stack.push("PagedLOD");

    apply(static_cast<LOD&>(node));
}

void osg::OSGTreeAnalyser::apply(ClearNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<ClearNode att=\"node\">");
    tag_stack.push("ClearNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(OccluderNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<OccluderNode att=\"node\">");
    tag_stack.push("OccluderNode");

    apply(static_cast<Group&>(node));
}

void osg::OSGTreeAnalyser::apply(OcclusionQueryNode& node) {
    addNode(node);

    nodes.back().properties.push_back(std::string(level++, '\t') + "<OcclusionQueryNode att=\"node\">");
    tag_stack.push("OcclusionQueryNode");

    apply(static_cast<Group&>(node));
}
