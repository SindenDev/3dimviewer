cmake_minimum_required ( VERSION 2.6 )

# Add path to custom cmake scripts
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

# Set default build type
if( NOT MSVC )
  if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING 
         "Choose the type of build, options are: Release and Debug." FORCE )
  endif( NOT CMAKE_BUILD_TYPE )
endif( NOT MSVC )

# Include needed macros
include( ${CMAKE_SOURCE_DIR}/CMakeModules/Required3rdPartyLibraries.txt )
include( ${CMAKE_SOURCE_DIR}/CMakeModules/ApplicationMacros.txt )
include( ${CMAKE_SOURCE_DIR}/CMakeModules/LibraryMacros.txt )
include( ${CMAKE_SOURCE_DIR}/CMakeModules/PluginMacros.txt )
include( ${CMAKE_SOURCE_DIR}/CMakeModules/InstallMacros.txt )

#-------------------------------------------------------------------------------
# Choose applications you want to compile
option( BUILD_3DimViewer "Should 3DimViewer be built?" ON )

set( _num_of_apps 0 )

if( BUILD_3DimViewer )
    math( EXPR _num_of_apps "${_num_of_apps} + 1" )
    set( BUILDING_QT_APP TRUE )
    set( TRIDIM_LIBRARY_EXT "_viewer" )
endif( BUILD_3DimViewer )

#-------------------------------------------------------------------------------
# Configuration step

include( ${CMAKE_SOURCE_DIR}/Setup.cmake )

#-------------------------------------------------------------------------------
# Basic definitions

macro( INCLUDE_BASIC_HEADERS )
  include_directories(
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/3dim/
    ${CMAKE_SOURCE_DIR}/include/3dim/core/
    ${CMAKE_SOURCE_DIR}/include/3dim/core${TRIDIM_LIBRARY_EXT}/
    ${CMAKE_SOURCE_DIR}/include/3dim/graph/
    ${CMAKE_SOURCE_DIR}/include/3dim/graph${TRIDIM_LIBRARY_EXT}/
    )
endmacro( INCLUDE_BASIC_HEADERS )

macro( INCLUDE_MEDICORE_HEADERS )
include_directories(
    ${CMAKE_SOURCE_DIR}/include/3dim/coremedi/
    ${CMAKE_SOURCE_DIR}/include/3dim/coremedi${TRIDIM_LIBRARY_EXT}/
    ${CMAKE_SOURCE_DIR}/include/3dim/graphmedi/
    ${CMAKE_SOURCE_DIR}/include/3dim/graphmedi${TRIDIM_LIBRARY_EXT}/
    )
endmacro( INCLUDE_MEDICORE_HEADERS )


INCLUDE_BASIC_HEADERS()
INCLUDE_MEDICORE_HEADERS()

if( BUILD_3DimViewer )
  if( APPLE )
    add_definitions( "-std=c++11" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
  else()
    include( CheckCXXCompilerFlag )
    if( CMAKE_COMPILER_IS_GNUCXX )
      check_cxx_compiler_flag( "-std=c++0x" COMPILER_SUPPORT_CPP0X )
      if( COMPILER_SUPPORT_CPP0X )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
      else()
        message( SEND_ERROR "Your compiler does not support c++0x standard!" )
      endif()
    endif()
  endif()
endif( BUILD_3DimViewer )

# 3Dim include directories
if( BUILDING_QT_APP )
    include_directories( ${CMAKE_SOURCE_DIR}/include/3dim/qtgui/ )
endif( BUILDING_QT_APP )

# 3Dim libraries path
set( TRIDIM_LIBRARIES_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include )
set( TRIDIM_LIBRARIES_SOURCE_PATH ${CMAKE_SOURCE_DIR}/src )

# 3Dim executables and plugins path
set( TRIDIM_EXECUTABLES_PATH ${CMAKE_SOURCE_DIR}/applications)
if( BUILDING_QT_APP )
  set( TRIDIM_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )
  set( TRIDIM_OSS_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )
else( BUILDING_QT_APP )
  # ${CMAKE_SOURCE_DIR}/plugins is not available anymore
endif( BUILDING_QT_APP )

#-------------------------------------------------------------------------------
# Names of all 3Dim libraries and plugins

# 3Dim libraries
set( TRIDIM_CORE_LIB 3DimCore )
set( TRIDIM_COREMEDI_LIB 3DimCoreMedi )
set( TRIDIM_GRAPH_LIB 3DimGraph )
set( TRIDIM_GRAPHMEDI_LIB 3DimGraphMedi )
set( TRIDIM_GUIQT_LIB 3DimGuiQt )
set( TRIDIM_GUIQTMEDI_LIB 3DimGuiQtMedi )

# 3Dim plugins
set( TRIDIM_DEMO_PLUGIN DemoPlugin )
set( TRIDIM_GAUGE_PLUGIN GaugePlugin )

macro(ADD_NON_OSS_PLUGINS)
endmacro(ADD_NON_OSS_PLUGINS)

if( BUILD_3DimViewer )
    add_subdirectory( applications/3DimViewer 3DimViewer )
endif( BUILD_3DimViewer )