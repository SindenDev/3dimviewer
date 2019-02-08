#===============================================================================
# $Id$
#
# 3DimViewer
# Lightweight 3D DICOM viewer.
#
# Copyright 2008-2012 3Dim Laboratory s.r.o.
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

# Create plugin
ADD_TRIDIM_PLUGIN( DemoPlugin )

# Options
set( TRIDIM_THIS_PLUGIN_PATH ${TRIDIM_OSS_PLUGINS_PATH}/Demo )
set( TRIDIM_THIS_PLUGIN_INCLUDE "" )
set( TRIDIM_THIS_PLUGIN_SRC "" )

INCLUDE_BASIC_OSS_HEADERS()
INCLUDE_MEDICORE_OSS_HEADERS()

# Include directory
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${TRIDIM_THIS_PLUGIN_PATH} )

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/qtgui${TRIDIM_LIBRARY_EXT} )
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/qtgui)
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/qtguimedi/ )

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graphmedi${TRIDIM_LIBRARY_EXT} )

#AppConfigure.h
target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE  "${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}applications/${BUILD_PROJECT_NAME}/include" )

#-------------------------------------------------------------------------------
# Find required 3rd party libraries

ADD_LIB_OPENMESH()
ADD_LIB_QT()
ADD_LIB_VPL()
ADD_LIB_GLAD()
ADD_LIB_DCMTK()
ADD_LIB_OSG()
ADD_LIB_FLANN()
ADD_LIB_EIGEN()
ADD_LIB_OPENGL()
ADD_LIB_ZLIB()

#-------------------------------------------------------------------------------
# Add Headers and Sources

set( TRIDIM_INCLUDE_DIR "${TRIDIM_ROOT_DIR}/include/3dim")
set( TRIDIM_SOURCE_DIR  "${TRIDIM_ROOT_DIR}/src")


ADD_HEADER_DIRECTORY( ${TRIDIM_THIS_PLUGIN_PATH}${TRIDIM_THIS_PLUGIN_INCLUDE} )
ADD_SOURCE_DIRECTORY( ${TRIDIM_THIS_PLUGIN_PATH}${TRIDIM_THIS_PLUGIN_SRC} )

#-------------------------------------------------------------------------------
# Some qt-related stuff

ADD_UI_DIRECTORY( ${TRIDIM_THIS_PLUGIN_PATH} )
ADD_RC_FILE( ${TRIDIM_THIS_PLUGIN_PATH}/demoplugin.qrc )
ADD_TRANSLATION_FILE( ${TRIDIM_THIS_PLUGIN_PATH}/translations/demoplugin-cs_cz.ts )

# !!! when UPDATE_TRANSLATIONS is on then ts files are generated
# from source files and rebuild erases them completely !!!
#set( UPDATE_TRANSLATIONS 1 )

#-------------------------------------------------------------------------------
# Create source groups

SOURCE_GROUP( "Header Files" REGULAR_EXPRESSION "^dummyrule$" )
SOURCE_GROUP( "Source Files" REGULAR_EXPRESSION "^dummyrule$" )

ADD_SOURCE_GROUPS( ${${TRIDIM_CURRENT_TARGET}_HEADERS}
                   ${${TRIDIM_CURRENT_TARGET}_SOURCES}
                   )

#-------------------------------------------------------------------------------
# Add required 3Dim libraries

ADD_3DIM_LIB_TARGET( ${TRIDIM_CORE_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_COREMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPH_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPHMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQT_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQTMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GEOMETRY_LIB} )

#-------------------------------------------------------------------------------
# Finalize plugin

# Build library 
# Build library 
# process headers by MOC and generate list of resulting source files
# Somehow moc has trouble locating included files (doesn't share include directories with project)
# so we have to give it include path at which it will succeed. The relevant file is PluginInterface.h
#QT5_WRAP_CPP( ${TRIDIM_CURRENT_TARGET}_MOC_SOURCES ${${TRIDIM_CURRENT_TARGET}_HEADERS} OPTIONS -I ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/)

# UI files are processed to headers and sources
#QT5_WRAP_UI( ${TRIDIM_CURRENT_TARGET}_UI_SOURCES ${${TRIDIM_CURRENT_TARGET}_UI_FILES} )
    
# same applies to resources
QT5_ADD_RESOURCES( ${TRIDIM_CURRENT_TARGET}_RC_SOURCES ${${TRIDIM_CURRENT_TARGET}_RC_FILES} )
    
#-------------------------------------------------------------------------------
# setup translations
# http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
# http://doc-snapshot.qt-project.org/5.0/qtdoc/cmake-manual.html
    
# files to translate
set( FILES_TO_TRANSLATE ${${TRIDIM_CURRENT_TARGET}_SOURCES} ${${TRIDIM_CURRENT_TARGET}_UI_FILES} )

# !!! when BUILD_UPDATE_TRANSLATIONS is on then ts files are generated
# !!! from source files and rebuild erases them completely
set( QM_FILES "" )
if( BUILD_UPDATE_TRANSLATIONS )
    message(WARNING " creating translations ${${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES}")
    qt5_create_translation( QM_FILES ${FILES_TO_TRANSLATE} ${${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES} )
else()
    qt5_add_translation( QM_FILES ${${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES} )
endif()

# add defition saying that this is a plugin
ADD_DEFINITIONS( -DQT_PLUGIN )
ADD_DEFINITIONS( -DQT_SHARED )

# add include directory
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )



target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE 
                ${${TRIDIM_CURRENT_TARGET}_SOURCES} 
                ${${TRIDIM_CURRENT_TARGET}_HEADERS} 
                ${${TRIDIM_CURRENT_TARGET}_UI_FILES} 
                ${${TRIDIM_CURRENT_TARGET}_RC_SOURCES}
                ${QM_FILES} 
                #${${TRIDIM_CURRENT_TARGET}_MOC_SOURCES} 
                )
 
set_target_properties( ${TRIDIM_CURRENT_TARGET} PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/pluginsd/"
                        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/"
                        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/"
                        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/pluginsd/"
                        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/"
                        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/"
                        PROJECT_LABEL plugin${TRIDIM_CURRENT_TARGET}
                        DEBUG_POSTFIX d
                        LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
                        )

# Add dependencies
add_dependencies( DemoPlugin
                  ${TRIDIM_CORE_LIB}
                  ${TRIDIM_COREMEDI_LIB}
                  ${TRIDIM_GRAPH_LIB}
                  ${TRIDIM_GRAPHMEDI_LIB}
                  ${TRIDIM_GUIQT_LIB}
                  ${TRIDIM_GUIQTMEDI_LIB}
                  ${TRIDIM_GEOMETRY_LIB}
                  )

# Add libraries
target_link_libraries( DemoPlugin PRIVATE
                       #${QT_LIBRARIES}
                       #${QT_QTMAIN_LIBRARY}
                       ${TRIDIM_CORE_LIB}
                       ${TRIDIM_COREMEDI_LIB}
                       ${TRIDIM_GRAPH_LIB}
                       ${TRIDIM_GRAPHMEDI_LIB}
                       ${TRIDIM_ADVQT_LIB}
                       ${TRIDIM_GUIQT_LIB}
                       ${TRIDIM_GUIQTMEDI_LIB}
                       ${TRIDIM_GEOMETRY_LIB}
                       #${OSG_LINKED_LIBS}
                       #${VPL_LINKED_LIBS}
                       #${DCMTK_LINKED_LIBS}
                       #${GLEW_LINKED_LIBS}
                       #${OPENMESH_LINKED_LIBS}
                       #${QT_LINKED_LIBS}
                       #${QT_QTMAIN_LIBRARY}
                       )

if( WIN32 )
  target_link_libraries( DemoPlugin PRIVATE opengl32.lib )
endif( )  


#-------------------------------------------------------------------------------
# Installation
INSTALL_PLUGIN_TRANSLATIONS()

TRIDIM_PLUGIN_INSTALL()

pop_target_stack()
