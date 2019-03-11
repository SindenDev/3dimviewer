#===============================================================================
#
# 3Dim Laboratory CMake file
# Copyright (c) 2010-2013 by 3Dim Laboratory s.r.o.
#
#===============================================================================

#-------------------------------------------------------------------------------
# Header and basic settings

cmake_minimum_required ( VERSION 3.12 )

message(STATUS "Using CMake version ${CMAKE_VERSION}")

#https://cmake.org/pipermail/cmake/2006-December/012288.html - XCode uses first of these as default..
#Remove MinSizeRel from default configurations
#Must be before project call.. (probably)
set(CMAKE_CONFIGURATION_TYPES Release Debug CACHE TYPE INTERNAL )


# CMAKE_OSX_DEPLOYMENT_TARGET/CMAKE_OSX_ARCHITECTURES must be set before first project call or it gets initialized by MACOSX_DEPLOYMENT_TARGET
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "")
set_property(CACHE CMAKE_OSX_DEPLOYMENT_TARGET PROPERTY STRINGS "10.9;10.10;10.11;10.12")

set( CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Type of Mac OS X architecture." )

#Explanation: variants for BUILD_PROJECT_NAME are added later in the script, but before that we need to use some value. Fortunately BUILD_PROJECT_NAME is
# cached so it works. This should add some variant of 3Dim_Something VS project to CMAKE_BINARY_DIR. This allows for configuring and working on multiple 
# projects without fighting for the parent project file which used to be the same for everything.
set(BUILD_PROJECT_NAME "3DimViewer" CACHE STRING "CMake will generate solution for this project.")

# Main project name
project( 3Dim_${BUILD_PROJECT_NAME} )

message(STATUS "  C: '${CMAKE_C_COMPILER}'")
message(STATUS "  C++: '${CMAKE_CXX_COMPILER}'")

# Add paths to custom cmake scripts

set( TRIDIM_OSS_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )
set( TRIDIM_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

# Set root dir variable
set( TRIDIM_ROOT_DIR ${CMAKE_SOURCE_DIR} )

set(dependency_manifest_path "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/bin/dependency_manifest.txt")

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
      set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release;Debug")
endif()


# https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
#use this instead of adding flags to compiler at many different places..
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)       #CMAKE_CXX_EXTENSIONS OFF on mac adds flag -std=c++11 instead of -std=gnu++11

# Include needed macros
include( ${CMAKE_MODULE_PATH}/AddLibMacros.cmake ) #should be before ScanLibs..
include( ${CMAKE_MODULE_PATH}/ScanLibs.cmake )
include( ${CMAKE_MODULE_PATH}/UtilityMacros.cmake )
include( ${CMAKE_MODULE_PATH}/AddMacros.cmake )
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


#cmake prefix path to root of qt..
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
#set(CMAKE_AUTOGEN_VERBOSE ON)

set_property(GLOBAL PROPERTY AUTOMOC_SOURCE_GROUP "moc")
set_property(GLOBAL PROPERTY AUTOUIC_SOURCE_GROUP "uih")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/applications/${BUILD_PROJECT_NAME}/ui ${CMAKE_SOURCE_DIR}/oss/applications/${BUILD_PROJECT_NAME}/ui)


#------------------------------------------------------------------------
# Project specific configuration options
if(MSVC)
    option( BUILD_RUN_INSTALL_AS_POST_BUILD "This will run the projects install script automatically as post build step." ON)
    set(options_to_unset "${options_to_unset};BUILD_RUN_INSTALL_AS_POST_BUILD" CACHE INTERNAL "")	     
endif()

set(BUILD_oss3DimViewer "TRUE")

set( BUILDING_QT_APP TRUE )
set( TRIDIM_LIBRARY_EXT "_viewer" )
set( BUILDING_3DIM_OSS_APP ON)  #??

#set(OSS_SUBFOLDER "oss/")

option( BUILD_PLUGINS "Should plugins be built?" ON )
option( BUILD_WITH_GDCM "Use GDCM instead of DCMTK?" OFF )
#option( BUILD_WITH_PYTHON "Enable python support (Deep Learning plugin will be available only if python is enabled)?" OFF )

set(options_to_unset "${options_to_unset};BUILD_PLUGINS;BUILD_WITH_GDCM;BUILD_WITH_PYTHON;INSTALL_INTERPRET;INSTALL_VPLSWIG")


if (MSVC)
	option( BUILD_USE_PRECOMPILED_HEADERS "Use precompiled headers?" ON )
	option( BUILD_INSTALL_ALSO_REQUIRED_SYSTEM_LIBS "This will install all the ucrt system dlls. Not required most of the time." OFF)
	
	set(options_to_unset "${options_to_unset};BUILD_USE_PRECOMPILED_HEADERS;BUILD_INSTALL_ALSO_REQUIRED_SYSTEM_LIBS" CACHE INTERNAL "")	
endif()

# !!! when BUILD_UPDATE_TRANSLATIONS is on then ts files are generated from source 
# !!! files and rebuild erases them completely
option( BUILD_UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)" OFF )


#-------------------------------------------------
# 3Dim libraries path
set( TRIDIM_LIBRARIES_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include )
set( TRIDIM_LIBRARIES_SOURCE_PATH ${CMAKE_SOURCE_DIR}/src )

# Path to applications folder with resources and project specific sources
set( TRIDIM_APPLICATION_SOURCE_FOLDER_PATH ${CMAKE_SOURCE_DIR}/applications)


if( BUILDING_QT_APP )
    set( TRIDIM_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )
    set( TRIDIM_OSS_PLUGINS_PATH ${CMAKE_SOURCE_DIR}/plugins )
endif()

#generate debug symbols for release too..  so for everything
if(MSVC)
    #https://msdn.microsoft.com/en-us/library/8f8h5cxt.aspx  -optimization flags

    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Zi" ) # plus /map file

    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /MAP" ) 

elseif(XCODE)
    #https://clang.llvm.org/docs/UsersManual.html#controlling-size-of-debug-information
    #http://stackoverflow.com/questions/15548023/clang-optimization-levels

    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O2" )

elseif(CMAKE_COMPILER_IS_GNUCXX)

    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g3 -O2" )

endif()

# set global compiler options depending on platform and capabilities
#-------------------------------------------------------------------------------
if(DARWIN)

    add_definitions( -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE )
    add_definitions( -Wno-undef )

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual -Wno-inconsistent-missing-override -Wno-reorder -Wno-unused-parameter -Wno-unused-variable -Wno-switch -Wno-deprecated -Wno-unused-function -Wno-unused-local-typedef" )

    # based on https://iscinumpy.gitlab.io/post/omp-on-high-sierra/
    option(MAC_OPENMP_ENABLED "Enable OpenMP support" OFF)
    if (MAC_OPENMP_ENABLED)
      message(STATUS "OpenMP support requires that you install libomp via brew/port")
      if (EXISTS "/opt/local/include/libomp")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I/opt/local/include/libomp")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/opt/local/include/libomp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "/opt/local/lib/libomp/libomp.dylib")
      else()
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "/usr/local/opt/libomp/lib/libomp.dylib")
      endif()
    endif()


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

    #I'm setting them at the top but these are relevant only for Mac so unset them.
    unset(CMAKE_OSX_DEPLOYMENT_TARGET CACHE)
    unset(CMAKE_OSX_ARCHITECTURES CACHE)
endif()

# VVVVVV todle pryc..? rozstrkat do jednotlivych projektu..? vrazit do nejakeho makra..? nebo to jde povazovat za spolecne?

if( CMAKE_COMPILER_IS_GNUCXX )

  add_definitions( -Wno-unknown-pragmas )

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual -Wno-inconsistent-missing-override -Wno-reorder -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fexceptions -fno-check-new -fno-common -fstrict-aliasing" )

  check_cxx_compiler_flag( "-Wno-variadic-macros" COMPILER_SUPPORT_WNOVARIADICMACRO )
  if( COMPILER_SUPPORT_WNOVARIADICMACRO )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros" )
  endif()

  check_cxx_compiler_flag( "-Wextra" COMPILER_SUPPORT_WEXTRA )
  if( COMPILER_SUPPORT_WEXTRA )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra" )
  endif()

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-unused-variable -Wno-switch" )



  check_cxx_compiler_flag("-fopenmp" COMPILER_SUPPORT_OPENMP)
  if( COMPILER_SUPPORT_OPENMP )
    option( TRIDIM_OPENMP_ENABLED "Enables OpenMP support." ON )
    if( TRIDIM_OPENMP_ENABLED )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp" )
      message( STATUS "Enabling OpenMP support" )
    endif()
  endif()
endif()


if( MSVC )

    #FLAGS
    #option(WARNINGS_EHsc "/EH (Exception Handling Model) - sc - catches C++ exceptions only and tells the compiler to assume
    #		  that functions declared as extern 'C' never throw a C++ exception." ON)
    add_compile_options("/EHsc")
    
    # Visual C++ default stack size is 1MB. This is not enough 
    # set stack reserved size to ~10MB
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
    
    check_cxx_compiler_flag("/openmp" COMPILER_SUPPORT_OPENMP)
    if( COMPILER_SUPPORT_OPENMP )
        option( BUILD_OPENMP_ENABLED "Enables OpenMP support." ON )
        if( BUILD_OPENMP_ENABLED )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp" )
            message( STATUS "Enabling OpenMP support" )
        endif()
    endif()
    
    check_cxx_compiler_flag("/MP" COMPILER_SUPPORT_MULTICORE_COMPILATION)
    if( COMPILER_SUPPORT_MULTICORE_COMPILATION )
        option( BUILD_MPCOMPILATION_ENABLED "Enables multicore compilation." ON )
        if( BUILD_MPCOMPILATION_ENABLED )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
            message( STATUS "Enabling multicore compilation support" )
        endif()
    endif()    
    
    
    
    
    # "The /guard:cf option causes the compiler to analyze control flow for
    # indirect call targets at compile time, and then to insert code to
    # verify the targets at runtime."
    # "The /guard:cf option must be passed to both the compiler and linker
    # to build code that uses the CFG exploit mitigation technique."
    # (incompatible with /ZI)   
    #check_cxx_compiler_flag("/guard:cf" COMPILER_SUPPORT_CONTROL_FLOW_GUARD)
    #if( COMPILER_SUPPORT_CONTROL_FLOW_GUARD )
    if (NOT (MSVC_VERSION LESS 1900))
        option( BUILD_CONTROL_FLOW_GUARD_ENABLED "Enables control flow guard." ON )
        if( BUILD_CONTROL_FLOW_GUARD_ENABLED )   
            #add_compile_options(/guard:cf) 
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /guard:cf" )
            set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /guard:cf" )
            set( CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /guard:cf" )
            set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /guard:cf" )
            set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /guard:cf" )
            message( STATUS "Enabling control flow guard support" )
        endif()
    endif()    

    
    #this is for only 32bit stuff...   sooo.. probably ok to remove
    if( NOT CMAKE_CL_64 )
        check_cxx_compiler_flag("/arch:SSE2" COMPILER_SUPPORT_SSE2)
        if( COMPILER_SUPPORT_SSE2 )
            option( BUILD_SSE2_ENABLED "Enables SSE2 support." ON )
            if( BUILD_SSE2_ENABLED )
                set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2" )
                message( STATUS "Enabling SSE2 support" )
            endif()
        endif()    
    endif( NOT CMAKE_CL_64 )
    
    # remove NDEBUG flag when release asserts are needed
    option( BUILD_RELEASE_ASSERTS "Enable asserts in release configuration." OFF)
    if (BUILD_RELEASE_ASSERTS)
      string(REPLACE "NDEBUG" "NNDEBUG" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
      string(REPLACE "NDEBUG" "NNDEBUG" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})      
      # for all targets enable qt asserts
      add_compile_options(-D QT_FORCE_ASSERTS)
    endif()

endif()


#-------------------------------------------------------------------------------
# Names of all 3Dim libraries and plugins

set(DOXYGEN_EXECUTABLE ${${DOXYGEN_LIB_NAME}_BINARY_DIRS}/doxygen.exe)
set(DOC_DIR doc)

# 3Dim libraries
set( TRIDIM_CORE_LIB 3DimCore )
set( TRIDIM_COREMEDI_LIB 3DimCoreMedi )
set( TRIDIM_GEOMETRY_LIB 3DimGeometry )
set( TRIDIM_GRAPH_LIB 3DimGraph )
set( TRIDIM_GRAPHMEDI_LIB 3DimGraphMedi )
set( TRIDIM_GUIQT_LIB 3DimGuiQt )
set( TRIDIM_GUIQTMEDI_LIB 3DimGuiQtMedi )
set( TRIDIM_DEEPLEARNING_LIB 3DimDeepLearning )

# 3Dim plugins
set( TRIDIM_DEMO_PLUGIN DemoPlugin )
set( TRIDIM_GAUGE_PLUGIN GaugePlugin )
set( TRIDIM_NOTES_PLUGIN NotesPlugin )



#-------------------------------------------------------------------------------
# Continue with subdirectories
#note: includes don't change the current directory of CMake.., subdirectories do.. to keep it simple dont use CMAKE_CURRENT_SOURCE_DIR
add_subdirectory( "${CMAKE_SOURCE_DIR}/applications/${BUILD_PROJECT_NAME}/" ${BUILD_PROJECT_NAME})


#-------------------------------------------------------------------------------
# Finalize

file(WRITE ${dependency_manifest_path} "")

#remove unused LIBRARY_xxx and INFO_whatever_xxx items by hiding them into advanced
foreach(name ${all_library_names})

    string(TOUPPER ${name} capitalized_name)

    GET_PROPERTY(value GLOBAL PROPERTY USED_LIB_${capitalized_name})

    if("${value}" STREQUAL "TRUE")
       
        file(APPEND ${dependency_manifest_path} "${LIBRARY_${name}}\n")

        continue()
    endif()
    
    mark_as_advanced(FORCE LIBRARY_${name})
    mark_as_advanced(FORCE INFO_LIBRARY-DIRS_${name})
    mark_as_advanced(FORCE INFO_INCLUDE-DIRS_${name})
    mark_as_advanced(FORCE INFO_LIBRARY-LIST_${name})

    if(${name} STREQUAL "Qt")
        mark_as_advanced(FORCE INFO_LIBRARY-LIST_${name}_qmake)
    endif()


    # try to remove these from newer lib alert as they are not needed for this project
    list(FIND newer_libraries_list "${name}" index)

    if(${index} GREATER "-1")
        list(REMOVE_ITEM newer_libraries_list "${name}")
    endif()
    
endforeach()


INSTALL_FILES_TO_BIN_NO_DEBUG(${dependency_manifest_path} ".")

#in the case that we want to use libraries specified in our library_manifest, it is not a good idea to overwrite it in repository
if(NOT ${BUILD_USE_LIBRARIES_FROM_MANIFEST})
    file(COPY ${dependency_manifest_path} DESTINATION ${CMAKE_SOURCE_DIR}/applications/${BUILD_PROJECT_NAME})
endif()

#add utility cmake scripts to main project for easier access
target_sources(${BUILD_PROJECT_NAME} PRIVATE ${CMAKE_MODULE_PATH}/AddLibMacros.cmake 
                                 ${CMAKE_MODULE_PATH}/ScanLibs.cmake
                                 ${CMAKE_MODULE_PATH}/UtilityMacros.cmake
                                 ${CMAKE_MODULE_PATH}/AddMacros.cmake
                                 ${CMAKE_MODULE_PATH}/InstallMacros.cmake )

source_group("CMake files" REGULAR_EXPRESSION  "[a-zA-Z]*\.cmake")







#Sets the startup project to selected build project instead of ALL_BUILD
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "${BUILD_PROJECT_NAME}")
endif()

#deduplicate collected flags - shouldn't be necessary..
string(REPLACE " " ";" list_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


list(REMOVE_DUPLICATES list_CMAKE_CXX_FLAGS)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${list_CMAKE_CXX_FLAGS}")


list(LENGTH newer_libraries_list length)

if(newer_libraries_alert AND ${length} GREATER "0")
    message(AUTHOR_WARNING "There are some new versions (or old missing) of libraries since last Configure was run. Consider deleting cache for the newest versions of everything or you can verify if these libraries are in order: ${newer_libraries_list}")
endif()


list(LENGTH invalid_library_list length)

#if(invalid_library_list AND ${length} GREATER "0")
#    message(STATUS "Every library folder has to have its name defined in AddLibMacros, in all_library_names list variable and have its ADD_LIB macro.")
#    message(STATUS "Most of the time this means someone added a library into prebuilt libs but uses it only in own branch.")
#    foreach(line ${invalid_library_list})
#        message(STATUS ${line})
#    endforeach()
#
#    message(STATUS "These libraries won't show in cmake gui for selection.")
#endif()


#check for wrong stack size - to catch oversights
list(LENGTH TRIDIM_CURRENT_TARGET_STACK length)

if(${length} GREATER "0")
    message(FATAL_ERROR "Target stack at the end of root CMakeList file is not empty: ${TRIDIM_CURRENT_TARGET_STACK}")
endif()


