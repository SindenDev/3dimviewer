#===============================================================================
# $Id: PluginMacros.txt 1292 2011-05-15 17:13:39Z spanel $
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
macro( ADD_TRIDIM_PLUGIN _PLUGIN_NAME )

    push_target_stack(${_PLUGIN_NAME})

    set( TRIDIM_PLUGIN_NAME ${_PLUGIN_NAME} )
    string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )
    set( TRIDIM_PLUGIN_PROJECT_NAME plugin${_SHORT_PLUGIN_NAME} )
    set( TRIDIM_PLUGIN_HEADERS "" )
    
    find_file(plugin_test_path "${_SHORT_PLUGIN_NAME}.cmake" ${CMAKE_SOURCE_DIR}/plugins ${CMAKE_SOURCE_DIR}/oss/plugins NO_DEFAULT_PATH )

    if(NOT plugin_test_path)
        message(WARNING "Plugin's cmake ${_SHORT_PLUGIN_NAME}.cmake was not found.")
    endif()
    

    set( TRIDIM_PLUGIN_SOURCES "${plugin_test_path}" )
    set( TRIDIM_PLUGIN_UI_FILES "" )
    set( TRIDIM_PLUGIN_RC_FILES "" )
    set( TRIDIM_PLUGIN_TRANSLATION_FILES "" )
    set( TRIDIM_PLUGIN_MOC_SOURCES "" )
    set( TRIDIM_PLUGIN_UI_SOURCES "" )
    set( TRIDIM_PLUGIN_RC_SOURCES "" )


    
    add_library( ${TRIDIM_PLUGIN_NAME} SHARED "${CMAKE_BINARY_DIR}/empty.hpp" )

    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    source_group("CMake files" FILES ${plugin_test_path})

    unset(plugin_test_path CACHE)

    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_PLUGIN_HEADER_FILE(${pcheader_h_location})
        ADD_PLUGIN_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties( ${pcheader_cpp_location}
            PROPERTIES
            COMPILE_FLAGS "/Ycpcheader.h")
     
        target_compile_options(${TRIDIM_PLUGIN_NAME} PRIVATE /Yupcheader.h /FIpcheader.h)

        unset(pcheader_h_location CACHE)
        unset(pcheader_cpp_location CACHE)
        
    endif()
    
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOMOC_MOC_OPTIONS -I${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/)


endmacro()


# Add plugin source file
macro( ADD_PLUGIN_SOURCE_FILE )
    set( TRIDIM_PLUGIN_SOURCES ${TRIDIM_PLUGIN_SOURCES} ${ARGV} )
endmacro()

# Add header file to the plugin
macro( ADD_PLUGIN_HEADER_FILE )
    set( TRIDIM_PLUGIN_HEADERS ${TRIDIM_PLUGIN_HEADERS} ${ARGV} )
endmacro()

# Add UI file to the plugin
macro( ADD_PLUGIN_UI_FILE )
    set( TRIDIM_PLUGIN_UI_FILES ${TRIDIM_PLUGIN_UI_FILES} ${ARGV} )
endmacro()

# Add RC file to the plugin
macro( ADD_PLUGIN_RC_FILE )
    set( TRIDIM_PLUGIN_RC_FILES ${TRIDIM_PLUGIN_RC_FILES} ${ARGV} )
endmacro()

# Add translation file to the plugin
macro( ADD_PLUGIN_TRANSLATION_FILE )
    set( TRIDIM_PLUGIN_TRANSLATION_FILES ${TRIDIM_PLUGIN_TRANSLATION_FILES} ${ARGV} )
endmacro()


# Add sources directory - adds all source files from the directory
macro( ADD_PLUGIN_SOURCE_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_PLUGIN_SOURCES ${_DIR}/*.c ${_DIR}/*.cpp ${_DIR}/*.cc )
    list( APPEND TRIDIM_PLUGIN_SOURCES ${_TRIDIM_PLUGIN_SOURCES} )
endmacro()

# Add include directory - adds all headers from the directory
macro( ADD_PLUGIN_HEADER_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_PLUGIN_HEADERS ${_DIR}/*.h ${_DIR}/*.hxx ${_DIR}/*.hpp )
    list( APPEND TRIDIM_PLUGIN_HEADERS ${_TRIDIM_PLUGIN_HEADERS} )
endmacro()

# Adds all UI files from the directory
macro( ADD_PLUGIN_UI_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_PLUGIN_UI_FILES ${_DIR}/*.ui )
    list( APPEND TRIDIM_PLUGIN_UI_FILES ${_TRIDIM_PLUGIN_UI_FILES} )
endmacro()

# Adds all RC files from the directory
macro( ADD_PLUGIN_RC_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_PLUGIN_RC_FILES ${_DIR}/*.rc )
    list( APPEND TRIDIM_PLUGIN_RC_FILES ${_TRIDIM_PLUGIN_RC_FILES} )
endmacro()

# Adds all translation files from the directory
macro( ADD_PLUGIN_TRANSLATION_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_PLUGIN_TRANSLATION_FILES ${_DIR}/*.ts )
    list( APPEND TRIDIM_PLUGIN_TRANSLATION_FILES ${_TRIDIM_PLUGIN_TRANSLATION_FILES} )
endmacro()


# Add dependency
macro( ADD_PLUGIN_DEPENDENCY _LIB )
    target_link_libraries( ${TRIDIM_CURRENT_TARGET} PRIVATE ${_LIB} )
endmacro()


# Build macro
macro( TRIDIM_PLUGIN_BUILD )

    message(FATAL_ERROR "TRIDIM_PLUGIN_BUILD")

    

    target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE ${TRIDIM_PLUGIN_SOURCES} ${TRIDIM_PLUGIN_HEADERS})

    set_target_properties( ${TRIDIM_CURRENT_TARGET} PROPERTIES
                           RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/pluginsd/"
                           RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/" 
                           RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/" 
                           LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/pluginsd/" 
                           LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/" 
                           LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/plugins/"
                           PROJECT_LABEL ${TRIDIM_PLUGIN_PROJECT_NAME}
                           DEBUG_POSTFIX d
                           LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
                           )
endmacro()

macro(INSTALL_PLUGIN_TRANSLATIONS)

    string( REPLACE Plugin "" folder_name ${TRIDIM_CURRENT_TARGET} )

    find_path(plugin_test_path "${folder_name}" ${CMAKE_SOURCE_DIR}/plugins ${CMAKE_SOURCE_DIR}/oss/plugins NO_DEFAULT_PATH )


    if(NOT plugin_test_path)
        message(WARNING "Plugin's folder ${folder_name} was not found either at ${CMAKE_SOURCE_DIR}/plugins or ${CMAKE_SOURCE_DIR}/oss/plugins.")
    endif()

    
    FILE( GLOB files "${plugin_test_path}/${folder_name}/translations/*.ts")
   
    #message("${plugin_test_path}")
    #message("${files}")

    foreach(file ${files})    
        string(REPLACE ".ts" ".qm" tmp ${file})
        
        get_filename_component(qm_file ${tmp} NAME)
        
        list(APPEND qm_files "${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/${qm_file}")
    endforeach()


    #message("${qm_files}")

    #these files can't be GLOBBED for the first time the script is run because they don't exist.. Instead work around it by taking the names that will exist so it works even the first time.
    #FILE( GLOB files "${CMAKE_BINARY_DIR}/${TRIDIM_CURRENT_TARGET}/?*.qm" )  #processed qm files are output into binary dir from where they have to be moved

    install(FILES ${qm_files} DESTINATION Debug/locale CONFIGURATIONS Debug)
    install(FILES ${qm_files} DESTINATION RelWithDebInfo/locale CONFIGURATIONS RelWithDebInfo)
    install(FILES ${qm_files} DESTINATION Release/locale CONFIGURATIONS Release)
    
    unset(plugin_test_path CACHE)
    unset(qm_files)

endmacro()

# Install macro
macro( TRIDIM_PLUGIN_INSTALL )
    if( MSVC )
        install( TARGETS ${TRIDIM_CURRENT_TARGET} RUNTIME DESTINATION Debug/pluginsd CONFIGURATIONS Debug)
        install( TARGETS ${TRIDIM_CURRENT_TARGET} RUNTIME DESTINATION RelWithDebInfo/plugins CONFIGURATIONS RelWithDebInfo)
        install( TARGETS ${TRIDIM_CURRENT_TARGET} RUNTIME DESTINATION Release/plugins CONFIGURATIONS Release)
    endif()
endmacro()
    
