#===============================================================================
# $Id: CMakeLists.txt 1292 2011-05-15 17:13:39Z spanel $
#
# 3DimViewer
# Lightweight 3D DICOM viewer.
#
# Copyright 2008-2016 3Dim Laboratory s.r.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#===============================================================================

#This is the Open Source version of 3DV

# Project name
project( 3DimViewer )

#-------------------------------------------------------------------------------
# Begin executable project

ADD_TRIDIM_EXECUTABLE( 3DimViewer )


target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE _UNICODE UNICODE)

add_definitions(-DPLUG_APP_SIGNATURE="OpenSource")    #if this is not defined, there is default "OpenSource" value. - see PluginInterface.h



# required definitions for 3rd party libraries
if( WIN32 )
# disabled
# 4005    'identifier' : macro redefinition
# 4099    PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info
# 4481    nonstandard extension used: override specifier 'keyword'
# 4189    local variable is initialized but not referenced
# 4996    This function or variable may be unsafe. Consider using...
# treat as errors
# 4552    operator has no effect; expected operator with side-effect
# 4701    potentially uninitialized local variable used
# 4715    not all control paths return a value
# 4716    function must return a value
  add_definitions ( /wd4005 /wd4099 /wd4481 /wd4189 /wd4996 /we4552 /we4701 /we4715 /we4716 ) 
endif()

  
#-------------------------------------------------------------------------------
# Options

set( QT3DIMVIEWER_PATH ${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/3DimViewer )
set( QT3DIMVIEWER_INCLUDE include )
set( QT3DIMVIEWER_SRC src )
set( QT3DIMVIEWER_UI ui )
 
#-------------------------------------------------------------------------------
# Some settings

INCLUDE_BASIC_OSS_HEADERS()
INCLUDE_MEDICORE_OSS_HEADERS()

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_INCLUDE} )

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/include/3dim/qtgui${TRIDIM_LIBRARY_EXT} )
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/include/3dim/qtgui )

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/plugins/Notes )

#-------------------------------------------------------------------------------
# Find required 3rd party libraries

set( QT_BASIC "TRUE")

ADD_LIB_QT()
ADD_LIB_OSG()
ADD_LIB_VPL()
ADD_LIB_EIGEN()

if( BUILD_WITH_GDCM )
    ADD_LIB_GDCM()
else()
    ADD_LIB_DCMTK()
endif()

ADD_LIB_GLEW()
ADD_LIB_OPENSSL()
ADD_LIB_OPENMESH()
ADD_LIB_ICONV()
ADD_LIB_OPENCTM()
ADD_LIB_ZLIB()
ADD_LIB_OPENGL()
ADD_LIB_FLANN()

if(BUILD_WITH_PYTHON)
    ADD_LIB_3DIMPYTHON()
endif(BUILD_WITH_PYTHON)


if(DARWIN)
    FIND_LIBRARY( MATH_LIBRARY m )

    find_library(APP_SERVICES ApplicationServices)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
  
    target_link_libraries( ${TRIDIM_CURRENT_TARGET} PRIVATE
        ${MATH_LIBRARY}
        ${APP_SERVICES}
        ${CORE_FOUNDATION_LIBRARY}
        )   
endif()
    

if( WIN32 )  
    target_link_libraries( ${TRIDIM_CURRENT_TARGET} PRIVATE Qt5::WinMain)
endif()

#-------------------------------------------------------------------------------
# Add files

# header files
# GraphicsWindowQt included explicitly as it doesn't have an extension
ADD_EXECUTABLE_HEADER_DIRECTORY( ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_INCLUDE} )

ADD_EXECUTABLE_HEADER_DIRECTORY( ${CMAKE_SOURCE_DIR}/include/3dim/qtplugin )



# source files
ADD_EXECUTABLE_SOURCE_DIRECTORY( ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_SRC} )

ADD_EXECUTABLE_SOURCE_DIRECTORY( ${CMAKE_SOURCE_DIR}/src/qtplugin )


# note: qt doesn't list headers if not specified together with sources
FILE( GLOB 3DIMVIEWERQT_FORMS ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_UI}/*.ui )
SET( 3DIMVIEWERQT_RESOURCES ${QT3DIMVIEWER_PATH}/resources.qrc )


#-------------------------------------------------------------------------------
# Create source groups

SOURCE_GROUP( "Header Files" REGULAR_EXPRESSION "^dummyrule$" )
SOURCE_GROUP( "Source Files" REGULAR_EXPRESSION "^dummyrule$" )

SOURCE_GROUP( "moc" REGULAR_EXPRESSION ".*/moc_[^/]*\\.cxx$" )
SOURCE_GROUP( "uih" REGULAR_EXPRESSION ".*/ui_[^/]*\\.h$" )

source_group( "ui" REGULAR_EXPRESSION ".*/ui/[^/]*\\.ui$" )
source_group( "qm" REGULAR_EXPRESSION ".*/[^/]*\\.qm$" )

ADD_SOURCE_GROUPS( ${QT3DIMVIEWER_INCLUDE}
                    ${QT3DIMVIEWER_SRC}                    
                    zip
                    )

ADD_SOURCE_GROUPS2( ${QT3DIMVIEWER_INCLUDE}
                    ${QT3DIMVIEWER_SRC}
                    ${QT3DIMVIEWER_INCLUDE}/3dim
                    ${QT3DIMVIEWER_SRC}
                    qtplugin                   
                    )

#-------------------------------------------------------------------------------
# QT related settings

# process headers by MOC and generate list of resulting source files
#QTX_WRAP_CPP( 3DIMVIEWERQT_INCLUDES_MOC ${TRIDIM_EXECUTABLE_HEADERS} )

# UI files are processed to headers and sources
#QTX_WRAP_UI( 3DIMVIEWERQT_FORMS_HEADERS ${3DIMVIEWERQT_FORMS} )

# same applies to resources
QTX_ADD_RESOURCES( 3DIMVIEWERQT_RESOURCES_RCC ${3DIMVIEWERQT_RESOURCES} )


# add additional required QT modules
SET( QT_USE_QTOPENGL TRUE )

#-------------------------------------------------------------------------------
# setup translations
# http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
# http://doc-snapshot.qt-project.org/5.0/qtdoc/cmake-manual.html
# http://www.kdab.com/using-cmake-with-qt-5/
# http://qt.developpez.com/doc/5.0-snapshot/cmake-manual/

# files to translate
SET( FILES_TO_TRANSLATE ${TRIDIM_EXECUTABLE_SOURCES} ${3DIMVIEWERQT_FORMS} )


# specify/create source translation files
#file( GLOB TRANSLATIONS_FILES ${CMAKE_SOURCE_DIR}/translations/*.ts )
set( TRANSLATIONS_FILES ${QT3DIMVIEWER_PATH}/translations/cs_cz.ts )

set( QT3DIMVIEWER_QM_FILES "" )
if( BUILD_UPDATE_TRANSLATIONS )
  message( warning " creating translations ${TRANSLATIONS_FILES}" )
  qtx_create_translation( QT3DIMVIEWER_QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES} )
else()
  qtx_add_translation( QT3DIMVIEWER_QM_FILES ${TRANSLATIONS_FILES} )
endif()

# add directory with intermediate files
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )


#-------------------------------------------------------------------------------
# Add required 3Dim libraries

# Add 3Dim libraries to the project
ADD_3DIM_LIB_TARGET( ${TRIDIM_CORE_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_COREMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GEOMETRY_LIB} ) 
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPH_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPHMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQT_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQTMEDI_LIB} )




if( BUILD_PLUGINS )

    # Define host application
    add_definitions(-DPLUGIN_VIEWER_3DIM )
    add_definitions(-DPLUGIN_HOST_3DIM="3DimViewer")

    # Add plugins
    ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_GAUGE_PLUGIN} )
    ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DEMO_PLUGIN} )
    ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_NOTES_PLUGIN} )
    
    if(BUILD_WITH_PYTHON)
        ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_SCRIPTVOLUME_PLUGIN} )
        ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_PYTHON_PLUGIN} )
    endif(BUILD_WITH_PYTHON)
    
endif()

#-------------------------------------------------------------------------------
# Finalize

IF(WIN32)
  # enable large address aware (>2GB)
  if( CMAKE_SIZEOF_VOID_P MATCHES 4 )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE" )
    message( STATUS "- MSVC: Enabled large address awareness" )
  endif()
endif()

target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE
                ${TRIDIM_EXECUTABLE_SOURCES}
                ${TRIDIM_EXECUTABLE_HEADERS}
                #${3DIMVIEWERQT_INCLUDES_MOC}
                #${3DIMVIEWERQT_FORMS_HEADERS}
                ${3DIMVIEWERQT_RESOURCES_RCC}
                ${QT3DIMVIEWER_QM_FILES}
                ${3DIMVIEWERQT_FORMS}
                )




# Set debug postfix, output directories, etc.
set_target_properties( ${TRIDIM_CURRENT_TARGET} PROPERTIES
                       LINKER_LANGUAGE CXX
                       PROJECT_LABEL ${TRIDIM_EXECUTABLE_PROJECT_NAME}
                       DEBUG_POSTFIX d
                       LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
                       #RUNTIME_OUTPUT_DIRECTORY "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/bin"
                       #RUNTIME_OUTPUT_DIRECTORY_DEBUG "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/bin"
                       #RUNTIME_OUTPUT_DIRECTORY_RELEASE "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/bin"
                       #RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/bin"
                       #RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/bin"
                       )


# Add dependencies
add_dependencies( ${TRIDIM_CURRENT_TARGET}
                  ${TRIDIM_GUIQT_LIB}
                  ${TRIDIM_GUIQTMEDI_LIB}
                  ${TRIDIM_GEOMETRY_LIB}
                  ${TRIDIM_GRAPH_LIB}
                  ${TRIDIM_GRAPHMEDI_LIB}
                  ${TRIDIM_CORE_LIB}
                  ${TRIDIM_COREMEDI_LIB}
                  )

if( BUILD_PLUGINS )
  add_dependencies( ${TRIDIM_CURRENT_TARGET}
                    #${TRIDIM_DATAEXPRESS_PLUGIN}
                    #${TRIDIM_AUTOSEGANDFILTERING_PLUGIN}
                    #${TRIDIM_MANUALSEG_PLUGIN}
                    #${TRIDIM_MANUALSEG3D_PLUGIN}
                    #${TRIDIM_MODELCREATE_PLUGIN}
                    #${TRIDIM_REGIONANDMODELCONTROL_PLUGIN}
                    #${TRIDIM_LOOKUPTABLEEDITOR_PLUGIN}
                    #${TRIDIM_SEGTOSMALLREGIONS_PLUGIN}
                    #${TRIDIM_DATASHIFT_PLUGIN}
                    ${TRIDIM_DEMO_PLUGIN}
                    ${TRIDIM_GAUGE_PLUGIN}
                    ${TRIDIM_NOTES_PLUGIN}
                    #${TRIDIM_EXPERIMENTALSEG_PLUGIN}
                    )
    
    if(BUILD_WITH_PYTHON)
        add_dependencies( 3DimViewer ${TRIDIM_SCRIPTVOLUME_PLUGIN})
        add_dependencies( 3DimViewer ${TRIDIM_PYTHON_PLUGIN})
    endif(BUILD_WITH_PYTHON)
    
endif()

# Add libraries
target_link_libraries( ${TRIDIM_CURRENT_TARGET} PRIVATE
                       ${TRIDIM_GUIQT_LIB}
                       ${TRIDIM_GUIQTMEDI_LIB}
                       ${TRIDIM_GEOMETRY_LIB}
                       ${TRIDIM_GRAPH_LIB}
                       ${TRIDIM_GRAPHMEDI_LIB}
                       ${TRIDIM_CORE_LIB}
                       ${TRIDIM_COREMEDI_LIB}
                       )


# Windows specific library
if( WIN32 )

  add_custom_command(
      TARGET ${TRIDIM_CURRENT_TARGET}
      POST_BUILD COMMAND
        mt.exe -manifest \"${CMAKE_SOURCE_DIR}/applications/3DimViewer/manifest.xml\" "-inputresource:\"$<TARGET_FILE:3DimViewer>\";#1" -outputresource:\"$<TARGET_FILE:3DimViewer>\";#1
      COMMENT
        "Appending manifest."
  )  
endif()

# Copy Qt translations
COPY_ANY_FILES( ${TRIDIM_CURRENT_TARGET} ${CMAKE_CURRENT_BINARY_DIR} ${QT3DIMVIEWER_PATH}/locale "/?*.qm" )
#macro( COPY_ANY_FILES _TARGET           _DIR_SOURCE                 _DIR_DESTINATION )


#-------------------------------------------------------------------------------
# Installation


set(qt_debug_stuff
"Qt5Cored.dll"
"Qt5Cored.pdb"
"Qt5Guid.dll"
"Qt5Guid.pdb"
"Qt5Networkd.dll"
"Qt5Networkd.pdb"
"Qt5OpenGLd.dll"
"Qt5OpenGLd.pdb"
"Qt5Multimediad.dll"
"Qt5Multimediad.pdb"
"Qt5MultimediaWidgetsd.dll"
"Qt5MultimediaWidgetsd.pdb"

"Qt5Positioningd.dll"
"Qt5Positioningd.pdb"
"Qt5PrintSupportd.dll"
"Qt5PrintSupportd.pdb"
"Qt5Qmld.dll"
"Qt5Qmld.pdb"
"Qt5Quickd.dll"
"Qt5Quickd.pdb"
"Qt5Sensorsd.dll"
"Qt5Sensorsd.pdb"
#"Qt5Scriptd.dll"
#"Qt5Scriptd.pdb"
"Qt5Sqld.dll"
"Qt5Sqld.pdb"
"Qt5Svgd.dll"
"Qt5Svgd.pdb"
"Qt5WebChanneld.dll"
"Qt5WebChanneld.pdb"
#"Qt5WebEngineCored.dll"
#"Qt5WebEngineCored.pdb"
#"Qt5WebEngined.dll"
#"Qt5WebEngined.pdb"
#"Qt5WebEngineWidgetsd.dll"
#"Qt5WebEngineWidgetsd.pdb"
"Qt5Widgetsd.dll"
"Qt5Widgetsd.pdb"
"Qt5Xmld.dll"
"Qt5Xmld.pdb"
)

set(qt_stuff
"Qt5Core.dll"
"Qt5Gui.dll"
"Qt5Network.dll"
"Qt5OpenGL.dll"
#"Qt5Positioning.dll"
"Qt5PrintSupport.dll"
#"Qt5Qml.dll"
#"Qt5Quick.dll"
#"Qt5Script.dll"
#"Qt5Sql.dll"
#"Qt5WebChannel.dll"
#"Qt5WebEngineCore.dll"
#"Qt5WebEngine.dll"
#"Qt5WebEngineWidgets.dll"
"Qt5Widgets.dll"
)

set(qt_imageformats 
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qjpeg.dll"
  #  "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qgif.dll"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qico.dll"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qsvg.dll" )
    
set(qt_debug_imageformats 
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qjpegd.dll"
  #  "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qgifd.dll"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qicod.dll"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/imageformats/qsvgd.dll" )
    
set(qt_translations
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qtbase_cs.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qtbase_de.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qtbase_es.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qtbase_fr.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qtbase_it.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qt_pt.qm"
    "${${QT_LIB_NAME}_BINARY_DIRS}/../translations/qt_zh_CN.qm"    
    )          
    
set(osg_debug_stuff
"OpenThreadsd.dll"
"osgd.dll"
"osgdb_freetyped.dll"
"osgdb_pngd.dll"
"osgdb_bmpd.dll"
"osgdb_stld.dll"
"osgDBd.dll"
"osgGAd.dll"
"osgManipulatord.dll"
"osgTextd.dll"
"osgUtild.dll"
"osgViewerd.dll"
"osgWidgetd.dll")

set(osg_stuff
"OpenThreads.dll"
"osg.dll"
"osgdb_freetype.dll"
"osgdb_png.dll"
"osgdb_bmp.dll"
"osgdb_stl.dll"
"osgDB.dll"
"osgGA.dll"
"osgManipulator.dll"
"osgText.dll"
"osgUtil.dll"
"osgViewer.dll"
"osgWidget.dll")



#set up for install all dynamic libraries needed to run hte application..
if(WIN32)

    #gather files first..
    set(files "")
    set(filesd "")

    #qt
    foreach(item ${qt_stuff})
        list(APPEND files ${${QT_LIB_NAME}_BINARY_DIRS}/${item})
    endforeach()
    
    foreach(item ${qt_debug_stuff})
        list(APPEND filesd ${${QT_LIB_NAME}_BINARY_DIRS}/${item})
    endforeach()
    
    
    INSTALL_FILES_TO_BIN("${qt_imageformats}" "${qt_debug_imageformats}" imageformats)
    
    INSTALL_FILES_TO_BIN("${qt_translations}" "${qt_translations}" locale_qt)    
    
    INSTALL_FILES_TO_BIN("${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/iconengines/qsvgicon.dll" "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/iconengines/qsvgicond.dll" iconengines)
                                             
    INSTALL_FILES_TO_BIN("${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/platforms/qwindows.dll" "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/platforms/qwindowsd.dll" platforms)
                                    
    #INSTALL_FILES_TO_BIN("${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/sqldrivers/qsqlite.dll" "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/sqldrivers/qsqlited.dll" sqldrivers)
    #
    #INSTALL_FILES_TO_BIN("${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/bearer/qgenericbearer.dll;${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/bearer/qnativewifibearer.dll"
    #    "${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/bearer/qgenericbearerd.dll;${${QT_LIB_NAME}_BINARY_DIRS}/../plugins/bearer/qnativewifibearerd.dll" bearer)


    #osg - it's more work because some are prefixed by osg130- or similar and some are not
    GET_OSG_FILES("${osg_stuff}" "${osg_debug_stuff}" files filesd)

    
    INSTALL_LIB(${OPENSSL_LIB_NAME})
    INSTALL_LIB(${ZLIB_LIB_NAME})
    INSTALL_LIB(${GLEW_LIB_NAME})
    INSTALL_LIB(${CURL_LIB_NAME})
    INSTALL_LIB(${ICONV_LIB_NAME})
    INSTALL_LIB(${PNG_LIB_NAME})
    INSTALL_LIB(${PHYSFS_LIB_NAME})
    INSTALL_LIB(${OPENCTM_LIB_NAME})

    
    #install needed files to run..    
    INSTALL_FILES_TO_BIN("${files}" "${filesd}" .)

    INSTALL_REQUIRED_SYSTEM_LIBS_TO_BIN()

endif()

# Copy needed resources
INSTALL_IMAGES()
INSTALL_MODELS()
INSTALL_ICONS()
INSTALL_DOCS()
INSTALL_FONTS()
INSTALL_TRANSLATIONS()

RUN_INSTALL_AS_POST_BUILD()

# Install application
#install( TARGETS 3DimViewer RUNTIME DESTINATION . )

pop_target_stack()
