#===============================================================================
#
# 3Dim Laboratory CMake file
# Copyright (c) 2010-2013 by 3Dim Laboratory s.r.o.
#
#===============================================================================

cmake_minimum_required ( VERSION 3.10 )


message(STATUS "Using CMake version ${CMAKE_VERSION}")

#https://cmake.org/pipermail/cmake/2006-December/012288.html - XCode uses first of these as default..
#Remove MinSizeRel from default configurations
#Must be before project call.. (probably)
set(CMAKE_CONFIGURATION_TYPES Release Debug RelWithDebInfo CACHE TYPE INTERNAL )


# CMAKE_OSX_DEPLOYMENT_TARGET/CMAKE_OSX_ARCHITECTURES must be set before first project call or it gets initialized by MACOSX_DEPLOYMENT_TARGET
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "")

set( CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Choose the type of Mac OS X architecture." )
set_property(CACHE CMAKE_OSX_ARCHITECTURES PROPERTY STRINGS "i386;x86_64")

#Explanation: variants for BUILD_PROJECT_NAME are added later in the script, but before that we need to use some value. Fortunately BUILD_PROJECT_NAME is
# cached so it works. This should add some variant of 3Dim_Something VS project to CMAKE_BINARY_DIR. This allows for configuring and working on multiple 
# projects without fighting for the parent project file which used to be the same for everything.
set(BUILD_PROJECT_NAME "3DimViewer" CACHE STRING "CMake will generate solution for this project.")

# Main project name
project( 3Dim_${BUILD_PROJECT_NAME} )

message(STATUS "  C: '${CMAKE_C_COMPILER}'")
message(STATUS "  C++: '${CMAKE_CXX_COMPILER}'")

# Add paths to custom cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

# Set root dir variable
set( TRIDIM_ROOT_DIR ${CMAKE_SOURCE_DIR} )

#initialize current target stack
set( TRIDIM_CURRENT_TARGET_STACK "") 

set(NUM_SIDE_SLICES_HALF_VALUE 2)

#this should help with 3DV include paths..
#3DV from here.. has oss/
#3DV from oss CMakeLists does not.. (it has empty string..)
#any other project does not have it also.
#For example see 3DimGraph.cmake
set(OSS_SUBFOLDER "")

#this is used only here and in ADD_3DIM_LIB_TARGET
set(OSS_MODULE_PREFIX "oss")

#APPLE is supposedly unreliable
#
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DARWIN TRUE)
endif()

if(${CMAKE_GENERATOR} MATCHES "Xcode")
    set(XCODE TRUE)
endif()


#helper file for creating executables without sources. The sources are added later with 'target_sources' command
if(NOT EXISTS "${CMAKE_BINARY_DIR}/empty.hpp")
    message(STATUS "Creating empty helper file.")
    file(WRITE ${CMAKE_BINARY_DIR}/empty.hpp "")
endif()

# Set default build type
if( NOT MSVC AND NOT XCODE )
      set( CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." )
      set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release;Debug;RelWithDebInfo")
endif()

# https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
#use this instead of adding flags to compiler at many different places..
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON) # adds current source/build directories to module's includes

# Include needed macros
include( ${CMAKE_MODULE_PATH}/AddLibMacros.cmake ) #should be before ScanLibs..
include( ${CMAKE_MODULE_PATH}/ScanLibs.cmake )
include( ${CMAKE_MODULE_PATH}/ApplicationMacros.cmake )
include( ${CMAKE_MODULE_PATH}/LibraryMacros.cmake )
include( ${CMAKE_MODULE_PATH}/PluginMacros.cmake )
include( ${CMAKE_MODULE_PATH}/InstallMacros.cmake )
include( CheckCXXCompilerFlag )


#we always want to install things into the bin folder with built executable..
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/bin CACHE PATH "" FORCE)

#first thing to do is to scan the library folder for all available libraries so the subsequent calls to add_lib_something have the needed variables.
scan_libs()

#don't leave irrelevant stuff if possible..
if(NOT ${SCAN_LIBS_SUCCESS} STREQUAL "TRUE")

    unset(CMAKE_OSX_DEPLOYMENT_TARGET CACHE)
    unset(CMAKE_OSX_ARCHITECTURES CACHE)
    
    return()
endif()

#clear the cache of the info variables - filled by adding libs
delete_previous_info_variables()

if(UNIX)
    # query the location of Qt install
    set(LIBRARY_PATHS_Qt_install_path "" CACHE PATH "Up to compiler folder e.g. clang_64.")

    #and others??
    

    #on the first run end now to prevent unnecessary config errors..
    if(NOT FIRST_RUN)
        set(FIRST_RUN "something" CACHE INTERNAL "")
        return()
    endif()
endif()

message("Configuring for project ${BUILD_PROJECT_NAME}.")

#initializes ARCHIVE_OUTPUT_DIRECTORY of all targets https://cmake.org/cmake/help/v3.3/prop_tgt/ARCHIVE_OUTPUT_DIRECTORY.html
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/lib)

if (MSVC)
  option( BUILD_USE_PRECOMPILED_HEADERS "Use precompiled headers?" ON )
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER Autogen_targets)

#Turn on automatic processing of ui and qt object files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # adds current source/build directories to module's includes


set_property(GLOBAL PROPERTY AUTOMOC_SOURCE_GROUP "moc")
set_property(GLOBAL PROPERTY AUTOUIC_SOURCE_GROUP "uih")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/oss/applications/${BUILD_PROJECT_NAME}/ui ${CMAKE_SOURCE_DIR}/applications/${BUILD_PROJECT_NAME}/ui)




#-------------------------------------------------------------------------------


set(BUILD_oss3DimViewer "TRUE")


set( BUILDING_QT_APP TRUE )
set( TRIDIM_LIBRARY_EXT "_viewer" )
set( BUILDING_3DIM_OSS_APP ON)

option( BUILD_PLUGINS "Should plugins be built?" ON )
    
set(TRIDIM_PLUGIN_HOST "PLUGIN_VIEWER_3DIM")

option( BUILD_WITH_GDCM "Use GDCM instead of DCMTK?" OFF )

option( BUILD_RUN_INSTALL_AS_POST_BUILD "This will run the projects install script automatically as post build step." ON)
option( BUILD_RUN_INSTALL_AS_POST_BUILD_EVERYTIME "This adds fake target which touches empty header to always trigger post build scripts. No effect with BUILD_RUN_INSTALL_AS_POST_BUILD OFF" ON)


# !!! when BUILD_UPDATE_TRANSLATIONS is on then ts files are generated from source 
# !!! files and rebuild erases them completely
option( BUILD_UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)" OFF )


#-------------------------------------------------
# 3Dim libraries path
set( TRIDIM_LIBRARIES_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include )
set( TRIDIM_LIBRARIES_SOURCE_PATH ${CMAKE_SOURCE_DIR}/src )

# 3Dim executables and plugins path - misleading naming i think..
set( TRIDIM_APPLICATION_SOURCE_FOLDER_PATH ${CMAKE_SOURCE_DIR}/applications)


set( TRIDIM_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )
set( TRIDIM_OSS_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )


# 3Dim include directories ?????

#include_directories( ${CMAKE_SOURCE_DIR}/include/3dim/qtgui/ )


#generate debug symbols for release too..  so for everything
if(MSVC)
    #https://msdn.microsoft.com/en-us/library/8f8h5cxt.aspx  -optimization flags

    set( CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG} /Zi" )
    set( CMAKE_CXX_FLAGS_RELWITHDEBINFO     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Zi")
    set( CMAKE_CXX_FLAGS_RELEASE            "${CMAKE_CXX_FLAGS_RELEASE} /Zi" ) # plus /map file

    set( CMAKE_EXE_LINKER_FLAGS_RELEASE     "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /MAP" ) 

elseif(XCODE)
    #https://clang.llvm.org/docs/UsersManual.html#controlling-size-of-debug-information
    #http://stackoverflow.com/questions/15548023/clang-optimization-levels

    set( CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG} -g" )
    set( CMAKE_CXX_FLAGS_RELWITHDEBINFO     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")
    set( CMAKE_CXX_FLAGS_RELEASE            "${CMAKE_CXX_FLAGS_RELEASE} -g -O2" )

elseif(CMAKE_COMPILER_IS_GNUCXX)

    set( CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG} -g3" )
    set( CMAKE_CXX_FLAGS_RELWITHDEBINFO     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g3")
    set( CMAKE_CXX_FLAGS_RELEASE            "${CMAKE_CXX_FLAGS_RELEASE} -g3 -O2" )

endif()



include ("${CMAKE_MODULE_PATH}/Warnings.cmake")


# set global compiler options depending on platform and capabilities
#-------------------------------------------------------------------------------
if(DARWIN)

    add_definitions( -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE )
    add_definitions( -Wno-undef )

    #this should be taken care of with setting CMAKE_CXX_STANDARD
    #option( USE_LIBC "Use libc++?" ON )
    #if (USE_LIBC)
    #    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
    #    add_definitions( "-stdlib=libc++" )
    #    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" )
    #
    #    if(${CMAKE_OSX_DEPLOYMENT_TARGET} VERSION_LESS "10.7")
    #        message(FATAL_ERROR "Using libc++, requires MacOSX 10.7 or higher")
    #    endif()
    #
    #endif()
else()

    #I'm setting them at the top but these are relevant only for Mac so nset them.
    unset(CMAKE_OSX_DEPLOYMENT_TARGET CACHE)
    unset(CMAKE_OSX_ARCHITECTURES CACHE)
endif()



#-------------------------------------------------------------------------------
# Names of all 3Dim libraries and plugins

# 3Dim libraries
set( TRIDIM_CORE_LIB 3DimCore )
set( TRIDIM_COREMEDI_LIB 3DimCoreMedi )
set( TRIDIM_GEOMETRY_LIB 3DimGeometry )
set( TRIDIM_GRAPH_LIB 3DimGraph )
set( TRIDIM_GRAPHMEDI_LIB 3DimGraphMedi )
set( TRIDIM_GUIQT_LIB 3DimGuiQt )
set( TRIDIM_GUIQTMEDI_LIB 3DimGuiQtMedi )

# 3Dim plugins
set( TRIDIM_DEMO_PLUGIN DemoPlugin )
set( TRIDIM_GAUGE_PLUGIN GaugePlugin )
set( TRIDIM_NOTES_PLUGIN NotesPlugin )


add_subdirectory( applications/3DimViewer 3DimViewer )


#-------------------------------------------------------------------------------
# Finalize


#remove unused LIBRARY_xxx and INFO_whatever_xxx items by hiding them into advanced
foreach(name ${all_library_names})

    string(TOUPPER ${name} capitalized_name)

    GET_PROPERTY(value GLOBAL PROPERTY USED_LIB_${capitalized_name})

    if("${value}" STREQUAL "TRUE")
        continue()
    endif()
    
    mark_as_advanced(FORCE LIBRARY_${name})
    mark_as_advanced(FORCE INFO_LIBRARY-DIRS_${name})
    mark_as_advanced(FORCE INFO_INCLUDE-DIRS_${name})
    mark_as_advanced(FORCE INFO_LIBRARY-LIST_${name})

    if(${name} STREQUAL "Qt")
        mark_as_advanced(FORCE INFO_LIBRARY-LIST_${name}_qmake)
    endif()
    
    
endforeach()

#Sets the startup project to selected instead of ALL_BUILD
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "${BUILD_PROJECT_NAME}")
endif()

#deduplicate collected flags - shouldn't be necessary..
string(REPLACE " " ";" list_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


list(REMOVE_DUPLICATES list_CMAKE_CXX_FLAGS)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${list_CMAKE_CXX_FLAGS}")

if(newer_libraries_alert)
    message(AUTHOR_WARNING "There are some new versions (or old missing) of libraries since last Configure was run. Consider deleting cache for the newest versions of everything or you can verify if these libraries are in order: ${newer_libraries_list}")
endif()

#check for wrong stack size - to catch oversights
list(LENGTH TRIDIM_CURRENT_TARGET_STACK length)

if(${length} GREATER "0")
    message(FATAL_ERROR "Target stack at the end of root CMakeList file is not empty: ${TRIDIM_CURRENT_TARGET_STACK}")
endif()