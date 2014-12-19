#===============================================================================
# $Id: CMakeLists.txt 1292 2011-05-15 17:13:39Z spanel $
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

# Project name
project( 3DimViewer )

#-------------------------------------------------------------------------------
# Begin executable project

TRIDIM_EXECUTABLE( 3DimViewer )

#-------------------------------------------------------------------------------
# Find required 3rd party libraries

ADD_LIB_QT()
ADD_LIB_OSG()
ADD_LIB_VPL()
ADD_LIB_EIGEN()
ADD_LIB_DCMTK()
ADD_LIB_GLEW()
ADD_LIB_OPENSSL()
ADD_LIB_OPENMESH()
ADD_LIB_ICONV()
if (APPLE)
  ADD_LIB_ZLIB()
  find_library(APP_SERVICES ApplicationServices)
  find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
else(APPLE)
  if ( BUILD_WITH_QT5 )
    ADD_LIB_ZLIB()
  endif( BUILD_WITH_QT5 )
endif(APPLE)

#-------------------------------------------------------------------------------
# Options

set( QT3DIMVIEWER_PATH ${TRIDIM_EXECUTABLES_PATH}/3DimViewer )
set( QT3DIMVIEWER_INCLUDE include )
set( QT3DIMVIEWER_SRC src )
set( QT3DIMVIEWER_UI ui )

#-------------------------------------------------------------------------------
# Some settings

add_definitions( -D_UNICODE -DUNICODE )

include_directories( ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_INCLUDE} )

# required definitions for 3rd party libraries
if( WIN32 )
  add_definitions ( -DNOMINMAX /wd4005 /wd4099 )
endif()

# Fix for bad include
include_directories( ${TRIDIM_LIBRARIES_INCLUDE_PATH}/3dim/qtgui )
include_directories( ${TRIDIM_LIBRARIES_INCLUDE_PATH}/3dim/qtgui_viewer )


#-------------------------------------------------------------------------------
# Add files

# header files
# GraphicsWindowQt included explicitly as it doesn't have an extension
ADD_EXECUTABLE_HEADER_DIRECTORY( ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_INCLUDE} )
if(BUILDING_3DIM_OSS_APP)
  ADD_EXECUTABLE_HEADER_DIRECTORY( ${CMAKE_SOURCE_DIR}/oss/include/3dim/qtplugin )
else()
  ADD_EXECUTABLE_HEADER_DIRECTORY( ${TRIDIM_LIBRARIES_INCLUDE_PATH}/3dim/qtplugin )
endif()

# source files
ADD_EXECUTABLE_SOURCE_DIRECTORY( ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_SRC} )
if(BUILDING_3DIM_OSS_APP)
  ADD_EXECUTABLE_SOURCE_DIRECTORY( ${CMAKE_SOURCE_DIR}/oss/src/qtplugin )
else()
  ADD_EXECUTABLE_SOURCE_DIRECTORY( ${TRIDIM_LIBRARIES_SOURCE_PATH}/qtplugin )
endif()

# note: qt doesn't list headers if not specified together with sources
FILE( GLOB 3DIMVIEWERQT_FORMS ${QT3DIMVIEWER_PATH}/${QT3DIMVIEWER_UI}/*.ui )
SET( 3DIMVIEWERQT_RESOURCES ${QT3DIMVIEWER_PATH}/resources.qrc )


#-------------------------------------------------------------------------------
# Create source groups

SOURCE_GROUP( "Header Files" REGULAR_EXPRESSION "^dummyrule$" )
SOURCE_GROUP( "Source Files" REGULAR_EXPRESSION "^dummyrule$" )

SOURCE_GROUP( "moc" REGULAR_EXPRESSION ".*/moc_[^/]*\\.cxx$" )
SOURCE_GROUP( "ui" REGULAR_EXPRESSION ".*/ui_[^/]*\\.h$" )

ADD_SOURCE_GROUPS2( ${QT3DIMVIEWER_INCLUDE}
                    ${QT3DIMVIEWER_SRC}
                    ${QT3DIMVIEWER_INCLUDE}/3dim
                    ${QT3DIMVIEWER_SRC}
                    qtplugin                   
                    )

#-------------------------------------------------------------------------------
# QT related settings

# process headers by MOC and generate list of resulting source files
QTX_WRAP_CPP( 3DIMVIEWERQT_INCLUDES_MOC ${TRIDIM_EXECUTABLE_HEADERS} )

# UI files are processed to headers and sources
QTX_WRAP_UI( 3DIMVIEWERQT_FORMS_HEADERS ${3DIMVIEWERQT_FORMS} )

# same applies to resources
QTX_ADD_RESOURCES( 3DIMVIEWERQT_RESOURCES_RCC ${3DIMVIEWERQT_RESOURCES} )

# add QT include directories and definitions
# QT_USE_FILE sets also definitions QT_DEBUG and QT_NO_DEBUG
if ( BUILD_WITH_QT5 )
  include_directories( ${Qt5OpenGL_INCLUDE_DIRS} )
else ( BUILD_WITH_QT5 )
  INCLUDE( ${QT_USE_FILE} )
  include_directories( ${QT_QTOPENGL_INCLUDE_DIR} )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )
endif( BUILD_WITH_QT5 )   

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

# !!! when UPDATE_TRANSLATIONS is on then ts files are generated from source 
# !!! files and rebuild erases them completely
option( UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)" OFF )

# specify/create source translation files
#file( GLOB TRANSLATIONS_FILES ${CMAKE_SOURCE_DIR}/translations/*.ts )
set( TRANSLATIONS_FILES ${QT3DIMVIEWER_PATH}/translations/cs_cz.ts )

set( QT3DIMVIEWER_QM_FILES "" )
if( UPDATE_TRANSLATIONS )
  message( warning " creating translations ${TRANSLATIONS_FILES}" )
  qtx_create_translation( QT3DIMVIEWER_QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES} )
else( UPDATE_TRANSLATIONS )
  qtx_add_translation( QT3DIMVIEWER_QM_FILES ${TRANSLATIONS_FILES} )
endif( UPDATE_TRANSLATIONS )

# add directory with intermediate files
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )


#-------------------------------------------------------------------------------
# Add required 3Dim libraries

# Add 3Dim libraries to the project
ADD_3DIM_LIB_TARGET( ${TRIDIM_CORE_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_COREMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPH_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GRAPHMEDI_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQT_LIB} )
ADD_3DIM_LIB_TARGET( ${TRIDIM_GUIQTMEDI_LIB} )


#-------------------------------------------------------------------------------
# Add required 3Dim plugins

option( BUILD_PLUGINS "Should plugins be built?" ON )

if( BUILD_PLUGINS )

# Define host application
add_definitions( -DPLUGIN_VIEWER_3DIM )
# add_definitions(-DNUM_SIDE_SLICES_HALF=0)

# Add plugins
ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_GAUGE_PLUGIN} )
ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DEMO_PLUGIN} )

endif( BUILD_PLUGINS )

#-------------------------------------------------------------------------------
# Finalize

IF(WIN32)
  # enable large address aware (>2GB)
  if( CMAKE_SIZEOF_VOID_P MATCHES 4 )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE" )
    message( STATUS "- MSVC: Enabled large address awareness" )
  endif()

  ADD_EXECUTABLE(3DimViewer WIN32
                 ${TRIDIM_EXECUTABLE_SOURCES}
                 ${TRIDIM_EXECUTABLE_HEADERS}
                 ${3DIMVIEWERQT_INCLUDES_MOC}
                 ${3DIMVIEWERQT_FORMS_HEADERS}
                 ${3DIMVIEWERQT_RESOURCES_RCC}
                 ${QT3DIMVIEWER_QM_FILES}
                 viewericon.rc
                 )
ELSE(WIN32)
  #regarding icon see http://developer.qt.nokia.com/doc/qt-4.8/appicon.html
  ADD_EXECUTABLE(3DimViewer
                 ${TRIDIM_EXECUTABLE_SOURCES}
                 ${TRIDIM_EXECUTABLE_HEADERS}
                 ${3DIMVIEWERQT_INCLUDES_MOC}
                 ${3DIMVIEWERQT_FORMS_HEADERS}
                 ${3DIMVIEWERQT_RESOURCES_RCC}
                 ${QT3DIMVIEWER_QM_FILES}
                 )
ENDIF(WIN32)

if ( BUILD_WITH_QT5 )
  USE_LIB_QT5()
endif( BUILD_WITH_QT5 )

# Set debug postfix, output directories, etc.
set_target_properties( 3DimViewer PROPERTIES
                       LINKER_LANGUAGE CXX
                       PROJECT_LABEL ${TRIDIM_EXECUTABLE_PROJECT_NAME}
                       DEBUG_POSTFIX d
                       LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
                       RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                       )


# Set debug postfix
#set_target_properties(3DimViewer PROPERTIES DEBUG_POSTFIX d)

# Add dependencies
add_dependencies( 3DimViewer
                  ${TRIDIM_GUIQT_LIB}
                  ${TRIDIM_GUIQTMEDI_LIB}
                  ${TRIDIM_GRAPH_LIB}
                  ${TRIDIM_GRAPHMEDI_LIB}
                  ${TRIDIM_CORE_LIB}
                  ${TRIDIM_COREMEDI_LIB}
                  )

# Add libraries
target_link_libraries( 3DimViewer
                       ${TRIDIM_GUIQT_LIB}
                       ${TRIDIM_GUIQTMEDI_LIB}
                       ${TRIDIM_GRAPH_LIB}
                       ${TRIDIM_GRAPHMEDI_LIB}
                       ${TRIDIM_CORE_LIB}
                       ${TRIDIM_COREMEDI_LIB}
                       ${QT_LINKED_LIBS}
                       ${QT_QTMAIN_LIBRARY}
                       ${OSG_LINKED_LIBS}
                       ${VPL_LINKED_LIBS}
                       ${DCMTK_LINKED_LIBS}
                       ${GLEW_LINKED_LIBS}
                       ${OPENSSL_LINKED_LIBS}
                       ${OPENMESH_LINKED_LIBS}
                       ${ICONV_LINKED_LIBS}
                       )

if ( APPLE )
FIND_PACKAGE( OpenGL )
FIND_LIBRARY( MATH_LIBRARY m )
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )

target_link_libraries( 3DimViewer 
			${ZLIB_LINKED_LIBS}
			${OPENGL_LIBRARIES}
			${MATH_LIBRARY}
			${APP_SERVICES}
			${CORE_FOUNDATION_LIBRARY}
			)
endif()

# Windows specific library
if( WIN32 )
  target_link_libraries( 3DimViewer opengl32.lib )
  if ( BUILD_WITH_QT5 )
    target_link_libraries( 3DimViewer Qt5::WinMain)
    target_link_libraries( 3DimViewer	${ZLIB_LINKED_LIBS} )
  endif( BUILD_WITH_QT5 )    
endif( WIN32 )

# Copy Qt translations
COPY_ANY_FILES( 3DimViewer ${CMAKE_CURRENT_BINARY_DIR} ${QT3DIMVIEWER_PATH}/locale "/?*.qm" )


#-------------------------------------------------------------------------------
# Installation

# Copy needed resources
INSTALL_IMAGES()
INSTALL_MODELS()
INSTALL_ICONS()
INSTALL_DOCS()
INSTALL_FONTS()
INSTALL_TRANSLATIONS()

# Install application
install( TARGETS 3DimViewer RUNTIME DESTINATION . )

