#===============================================================================
# $Id: LibraryMacros.txt 1283 2011-04-28 11:26:26Z spanel $
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

#-------------------------------------------------------------------------------
# Used macros

# Library name macro
macro( ADD_TRIDIM_LIBRARY _LIBRARY_NAME )

    push_target_stack(${_LIBRARY_NAME})

    set( TRIDIM_LIBRARY_NAME ${_LIBRARY_NAME} )
    set( TRIDIM_LIBRARY_PROJECT_NAME lib${_LIBRARY_NAME} )
    set( TRIDIM_LIBRARY_HEADERS "" )
    
    find_file(library_test_path "${OSS_MODULE_PREFIX}${_LIBRARY_NAME}.cmake" ${CMAKE_SOURCE_DIR}/src/cmake/ ${CMAKE_SOURCE_DIR}/oss/src/cmake/ NO_DEFAULT_PATH )

    if(NOT library_test_path)
        message(WARNING "Library's cmake ${_LIBRARY_NAME}.cmake was not found.")
    endif()
    
    set( TRIDIM_LIBRARY_SOURCES "${library_test_path}" )
    set( TRIDIM_PLUGIN_MOC_SOURCES "" )


    add_library( ${TRIDIM_CURRENT_TARGET} STATIC "${CMAKE_BINARY_DIR}/empty.hpp")
    
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    source_group("CMake files" FILES "${library_test_path}")
  
    unset(library_test_path CACHE)

            
    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_LIBRARY_HEADER_FILE(${pcheader_h_location})
        ADD_LIBRARY_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties(${pcheader_cpp_location}
        PROPERTIES
        COMPILE_FLAGS "/Ycpcheader.h")
        
        target_compile_options(${TRIDIM_LIBRARY_NAME} PRIVATE /Yupcheader.h /FIpcheader.h)
        
        unset(pcheader_h_location CACHE)
        unset(pcheader_cpp_location CACHE)
        
    endif()  

    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
    
endmacro()

macro( ADD_TRIDIM_DLL_LIBRARY _LIBRARY_NAME )

    message(FATAL_ERROR "ADD_TRIDIM_DLL_LIBRARY")
    
    push_target_stack(${_LIBRARY_NAME})

    set( TRIDIM_LIBRARY_NAME ${_LIBRARY_NAME} )
    set( TRIDIM_LIBRARY_PROJECT_NAME lib${_LIBRARY_NAME} )
    set( TRIDIM_LIBRARY_HEADERS "" )
    set( TRIDIM_LIBRARY_SOURCES "" )
    set( TRIDIM_PLUGIN_MOC_SOURCES "" )

    add_library( ${TRIDIM_CURRENT_TARGET} SHARED "${CMAKE_BINARY_DIR}/empty.hpp")
    
endmacro()

# Add library source file
macro( ADD_LIBRARY_SOURCE_FILE )
    set( TRIDIM_LIBRARY_SOURCES ${TRIDIM_LIBRARY_SOURCES} ${ARGV})
endmacro()

# Add header file to the library
macro( ADD_LIBRARY_HEADER_FILE )
    set( TRIDIM_LIBRARY_HEADERS ${TRIDIM_LIBRARY_HEADERS} ${ARGV})
endmacro()

# Add sources directory - adds all source files from the directory
macro( ADD_LIBRARY_SOURCE_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_LIBRARY_SOURCES ${_DIR}/*.c ${_DIR}/*.cpp ${_DIR}/*.cc )
    list( APPEND TRIDIM_LIBRARY_SOURCES ${_TRIDIM_LIBRARY_SOURCES} )
endmacro()

# Add include directory - adds all headers from the directory
macro( ADD_LIBRARY_HEADER_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_LIBRARY_HEADERS ${_DIR}/*.h ${_DIR}/*.hxx ${_DIR}/*.hpp )
    list( APPEND TRIDIM_LIBRARY_HEADERS ${_TRIDIM_LIBRARY_HEADERS} )
endmacro()




