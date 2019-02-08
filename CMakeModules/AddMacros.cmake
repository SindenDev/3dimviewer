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

macro( ADD_TRIDIM_EXECUTABLE _EXECUTABLE_NAME )	

    #this updates TRIDIM_CURRENT_TARGET
    push_target_stack(${_EXECUTABLE_NAME})
    
    set( ${TRIDIM_CURRENT_TARGET}_HEADERS "" )
    set( ${TRIDIM_CURRENT_TARGET}_SOURCES "" )

    set(${TRIDIM_CURRENT_TARGET}_CMAKES
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt    
        ${TRIDIM_OSS_MODULE_PATH}/AddLibMacros.cmake 
        ${TRIDIM_OSS_MODULE_PATH}/ScanLibs.cmake
        ${TRIDIM_OSS_MODULE_PATH}/UtilityMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/AddMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/InstallMacros.cmake 
    )


    set(icon "")

    if(EXISTS ${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc)
        set(icon "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc")
    endif()
    
    #call to this macro is usually among the first things in cmake file
    #so use this opportunity to create new executable for use in targeted commands
    if(WIN32)
        add_executable(${TRIDIM_CURRENT_TARGET} WIN32 ${${TRIDIM_CURRENT_TARGET}_CMAKES} ${icon})
    else()
        add_executable(${TRIDIM_CURRENT_TARGET} "${CMAKE_BINARY_DIR}/empty.hpp")
    endif()

    
    #This eliminates most of the autogen warnings about empty file. Also in Library and plugin macros.
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    #target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE ${${TRIDIM_CURRENT_TARGET}_CMAKES})

    source_group("CMake files" FILES ${${TRIDIM_CURRENT_TARGET}_CMAKES})


    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_HEADER_FILE(${pcheader_h_location})
        ADD_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties(${pcheader_cpp_location}
          PROPERTIES
          COMPILE_FLAGS "/Ycpcheader.h")
        
        target_compile_options(${TRIDIM_CURRENT_TARGET} PRIVATE /Yupcheader.h /FIpcheader.h)

        unset(pcheader_h_location CACHE)
        unset(pcheader_cpp_location CACHE)

    endif()
         
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
           
    RUN_INSTALL_AS_POST_BUILD()

    if(MSVC)
        add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET ${TRIDIM_CURRENT_TARGET} )
    endif()

endmacro()


# Executable name macro - omits WIN32 flag
macro( ADD_TRIDIM_CONSOLE_EXECUTABLE _EXECUTABLE_NAME )	
 
    #this updates TRIDIM_CURRENT_TARGET
    push_target_stack(${_EXECUTABLE_NAME})

    set( ${TRIDIM_CURRENT_TARGET}_HEADERS "" )
    set( ${TRIDIM_CURRENT_TARGET}_SOURCES "" )

    set(${TRIDIM_CURRENT_TARGET}_CMAKES
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt    
        ${TRIDIM_OSS_MODULE_PATH}/AddLibMacros.cmake 
        ${TRIDIM_OSS_MODULE_PATH}/ScanLibs.cmake
        ${TRIDIM_OSS_MODULE_PATH}/UtilityMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/AddMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/InstallMacros.cmake 
    )

    set(icon "")
    
    if(EXISTS ${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc)
        set(icon "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc")	
    endif()
    
    #call to this macro is usually among the first things in cmake file
    #so use this opportunity to create new executable for use in targeted commands
    if(WIN32)
        add_executable(${TRIDIM_CURRENT_TARGET} ${${TRIDIM_CURRENT_TARGET}_CMAKES} ${icon})
    else()
        add_executable(${TRIDIM_CURRENT_TARGET} "${CMAKE_BINARY_DIR}/empty.hpp")
    endif()
    
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    #target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE ${${TRIDIM_CURRENT_TARGET}_CMAKES})

    source_group("CMake files" FILES ${${TRIDIM_CURRENT_TARGET}_CMAKES})


    if (USE_PRECOMPILED_HEADERS AND MSVC)
        
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)
   
        ADD_HEADER_FILE(${pcheader_h_location})
        ADD_SOURCE_FILE(${pcheader_cpp_location})
   
        set_source_files_properties(${pcheader_cpp_location}
          PROPERTIES
          COMPILE_FLAGS "/Ycpcheader.h")
        
       target_compile_options(${TRIDIM_CURRENT_TARGET} PRIVATE /Yupcheader.h /FIpcheader.h)
       
       unset(pcheader_h_location CACHE)
       unset(pcheader_cpp_location CACHE)
        
    endif()    
            
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
           
    RUN_INSTALL_AS_POST_BUILD()

    if(MSVC)
        add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET ${TRIDIM_CURRENT_TARGET} )
    endif()

endmacro()

macro( ADD_3DIM_TEST_TARGET _LIB_NAME )
    set(TEST_NAME "${_LIB_NAME}Test")
    
    if( NOT _${TEST_NAME}_INCLUDED )
        set( _${TEST_NAME}_INCLUDED 1 )
                
        include( "${CMAKE_SOURCE_DIR}/src/cmake/${OSS_MODULE_PREFIX}${TEST_NAME}.cmake" )
        
        #add definitions indicating used libs for testing in precompiled headers
        string(TOUPPER ${TEST_NAME} capitalized_libname)
        target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE USED_LIB_${capitalized_libname})        
    endif()
endmacro()

macro( ADD_TRIDIM_TEST TEST_NAME )

    if(NOT "${GTest_FOUND}" STREQUAL "TRUE")
        message("Gtest framework wasn't found cannot be created test.")
        return()
    endif()
    
    push_target_stack(${TEST_NAME})
    
    find_file(test_path "${OSS_MODULE_PREFIX}${TEST_NAME}.cmake" ${CMAKE_SOURCE_DIR}/src/cmake/ ${CMAKE_SOURCE_DIR}/oss/src/cmake/ NO_DEFAULT_PATH )

    if(NOT test_path)
        message(WARNING "Test's cmake ${TEST_NAME}.cmake was not found.")
    endif()
    
    set( ${TRIDIM_CURRENT_TARGET}_HEADERS "" )
    set( ${TRIDIM_CURRENT_TARGET}_SOURCES "${test_path}" )
    set( ${TRIDIM_CURRENT_TARGET}_MOC_SOURCES "" )
    unset(test_path CACHE)
    

    add_executable(${TRIDIM_CURRENT_TARGET} "${CMAKE_BINARY_DIR}/empty.hpp")

    ADD_LIB_GTEST()

    #This eliminates most of the autogen warnings about empty file. Also in Library and plugin macros.
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)
    
    source_group("CMake files" FILES "${library_test_path}")
  
    unset(library_test_path CACHE)
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})

    
endmacro()

macro( ADD_TRIDIM_LIBRARY _LIBRARY_NAME )

    push_target_stack(${_LIBRARY_NAME})

    
    find_file(library_test_path "${OSS_MODULE_PREFIX}${_LIBRARY_NAME}.cmake" ${CMAKE_SOURCE_DIR}/src/cmake/ ${CMAKE_SOURCE_DIR}/oss/src/cmake/ NO_DEFAULT_PATH )

    if(NOT library_test_path)
        message(WARNING "Library's cmake ${_LIBRARY_NAME}.cmake was not found.")
    endif()
    
    set( ${TRIDIM_CURRENT_TARGET}_HEADERS "" )
    set( ${TRIDIM_CURRENT_TARGET}_SOURCES "" )
    set( ${TRIDIM_CURRENT_TARGET}_MOC_SOURCES "" )

    set(${TRIDIM_CURRENT_TARGET}_CMAKES
        ${library_test_path}    
        ${TRIDIM_OSS_MODULE_PATH}/AddLibMacros.cmake 
        ${TRIDIM_OSS_MODULE_PATH}/ScanLibs.cmake
        ${TRIDIM_OSS_MODULE_PATH}/UtilityMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/AddMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/InstallMacros.cmake 
    )

    if("${ARGV1}" STREQUAL "SHARED")
        add_library( ${TRIDIM_CURRENT_TARGET} SHARED "${CMAKE_BINARY_DIR}/empty.hpp")
    else()
        add_library( ${TRIDIM_CURRENT_TARGET} STATIC ${${TRIDIM_CURRENT_TARGET}_CMAKES})
    endif()
    
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)
  
    #target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE ${${TRIDIM_CURRENT_TARGET}_CMAKES})

    source_group("CMake files" FILES ${${TRIDIM_CURRENT_TARGET}_CMAKES})


    unset(library_test_path CACHE)

            
    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_HEADER_FILE(${pcheader_h_location})
        ADD_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties(${pcheader_cpp_location}
        PROPERTIES
        COMPILE_FLAGS "/Ycpcheader.h")
        
        target_compile_options(${TRIDIM_CURRENT_TARGET} PRIVATE /Yupcheader.h /FIpcheader.h)
        
        unset(pcheader_h_location CACHE)
        unset(pcheader_cpp_location CACHE)
        
    endif()  

    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
    
endmacro()


macro( ADD_TRIDIM_PLUGIN _PLUGIN_NAME )

    push_target_stack(${_PLUGIN_NAME})

    string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )
    
    find_file(plugin_test_path "${_SHORT_PLUGIN_NAME}.cmake" ${CMAKE_SOURCE_DIR}/plugins ${CMAKE_SOURCE_DIR}/oss/plugins NO_DEFAULT_PATH )

    if(NOT plugin_test_path)
        message(WARNING "Plugin's cmake ${_SHORT_PLUGIN_NAME}.cmake was not found.")
    endif()
    
    set( ${TRIDIM_CURRENT_TARGET}_HEADERS "" )
    set( ${TRIDIM_CURRENT_TARGET}_SOURCES "" )

    set(${TRIDIM_CURRENT_TARGET}_CMAKES
        ${plugin_test_path}
        ${TRIDIM_OSS_MODULE_PATH}/AddLibMacros.cmake 
        ${TRIDIM_OSS_MODULE_PATH}/ScanLibs.cmake
        ${TRIDIM_OSS_MODULE_PATH}/UtilityMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/AddMacros.cmake
        ${TRIDIM_OSS_MODULE_PATH}/InstallMacros.cmake 
    )
    
    set( ${TRIDIM_CURRENT_TARGET}_UI_FILES "" )
    set( ${TRIDIM_CURRENT_TARGET}_RC_FILES "" )
    set( ${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES "" )
    set( ${TRIDIM_CURRENT_TARGET}_MOC_SOURCES "" )
    set( ${TRIDIM_CURRENT_TARGET}_UI_SOURCES "" )
    set( ${TRIDIM_CURRENT_TARGET}_RC_SOURCES "" )

  
    add_library( ${TRIDIM_CURRENT_TARGET} SHARED "${CMAKE_BINARY_DIR}/empty.hpp" )

    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)


    target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE ${${TRIDIM_CURRENT_TARGET}_CMAKES})

    source_group("CMake files" FILES ${${TRIDIM_CURRENT_TARGET}_CMAKES})

    unset(plugin_test_path CACHE)

    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_HEADER_FILE(${pcheader_h_location})
        ADD_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties( ${pcheader_cpp_location}
            PROPERTIES
            COMPILE_FLAGS "/Ycpcheader.h")
     
        target_compile_options(${TRIDIM_CURRENT_TARGET} PRIVATE /Yupcheader.h /FIpcheader.h)

        unset(pcheader_h_location CACHE)
        unset(pcheader_cpp_location CACHE)
        
    endif()
    
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOMOC_MOC_OPTIONS -I${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/)

    add_dependencies(${BUILD_PROJECT_NAME} ${TRIDIM_CURRENT_TARGET} )
    
    #on other systems the target doesn't exist
    if(MSVC)
        #dependency of install target on plugins should ensure that install is done after plugins have been built.
        add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET ${TRIDIM_CURRENT_TARGET} )
    endif()

endmacro()


macro( ADD_3DIM_LIB_TARGET _LIB_NAME )
    if( NOT _${_LIB_NAME}_INCLUDED )
        set( _${_LIB_NAME}_INCLUDED 1 )

        include( "${CMAKE_SOURCE_DIR}/src/cmake/${OSS_MODULE_PREFIX}${_LIB_NAME}.cmake" )
        
        #add definitions indicating used libs for testing in precompiled headers
        string(TOUPPER ${_LIB_NAME} capitalized_libname)
        target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE USED_LIB_${capitalized_libname})        
    endif()
endmacro()


#-------------------------------------------------------------------------------
# Adding 3Dim plugin

macro( ADD_3DIM_PLUGIN_TARGET _PLUGIN_NAME )
    if( NOT _${_PLUGIN_NAME}_INCLUDED )
        set( _${_PLUGIN_NAME}_INCLUDED 1 )

        string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )

        include( "${CMAKE_SOURCE_DIR}/plugins/${_SHORT_PLUGIN_NAME}.cmake" )
    endif()
    
    #make shared libs and plugins dependants of CUSTOM_INSTALL_TARGET, so the install will trigger only after they are also built.
    #this should avoid problem when finished build of main project will trigger install without plugins being ready.
    #add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET "${_PLUGIN_NAME}" )

endmacro()

#-------------------------------------------------------------------------------
# Adding 3Dim plugin - QT version

macro( ADD_3DIM_QTPLUGIN_TARGET _PLUGIN_NAME )

    if( NOT _${_PLUGIN_NAME}_INCLUDED )
        set( _${_PLUGIN_NAME}_INCLUDED 1 )

        string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )

        find_path(${_PLUGIN_NAME}_plugin_path NAMES "${_SHORT_PLUGIN_NAME}.cmake" PATHS ${CMAKE_SOURCE_DIR}/plugins/ ${CMAKE_SOURCE_DIR}/oss/plugins/ NO_DEFAULT_PATH )
        
        include( "${${_PLUGIN_NAME}_plugin_path}/${_SHORT_PLUGIN_NAME}.cmake" )

        unset(${_PLUGIN_NAME}_plugin_path CACHE)
#        add_subdirectory( ${CMAKE_SOURCE_DIR}/qtplugins/${_SHORT_PLUGIN_NAME} ${_SHORT_PLUGIN_NAME} )
    endif()

    #make shared libs and plugins dependants of CUSTOM_INSTALL_TARGET, so the install will trigger only after they are also built.
    #this should avoid problem when finished build of main project will trigger install without plugins being ready.
    #add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET "${_PLUGIN_NAME}")
endmacro()

#-------------------------------------------------------------------------------
# Adding source groups



#creates source groups in solution.. Keep in mind, that order of the subfolders matters..
#Files are assigned to the last group they matched so the most specific paths should be last.
#Use only after call to !target_sources! or it won't have desired effect..
macro( ADD_SOURCE_GROUPS _DIR_INCLUDE _DIR_SOURCE )

    string(FIND "${_DIR_INCLUDE}" "include" result)

    if(${result} GREATER "-1")
        set(include_group_name_postfix "")
    else()
        set(include_group_name_postfix " headers")
    endif()


    string(FIND "${_DIR_SOURCE}" "src" result)

    if(${result} GREATER "-1")
        set(src_group_name_postfix "")
    else()
        set(src_group_name_postfix " sources")
    endif()


    source_group( "${_DIR_INCLUDE}${include_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_INCLUDE}/[^/]*\\.(h|hxx|hpp)$" )
    source_group( "${_DIR_SOURCE}${src_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_SOURCE}/[^/]*\\.(c|cpp)$" )
    
    foreach( subfolder ${ARGN} )
        source_group( "${_DIR_INCLUDE}/${subfolder}${include_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_INCLUDE}/${subfolder}.*\\.(h|hxx|hpp)$" )
        source_group( "${_DIR_SOURCE}/${subfolder}${src_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_SOURCE}/${subfolder}.*\\.(c|cpp)$" )
    endforeach()

endmacro()

macro( ADD_SOURCE_GROUPS_WITH_TESTS _DIR_INCLUDE _DIR_SOURCE _DIR_TEST )

    string(FIND "${_DIR_INCLUDE}" "include" result)

    if(${result} GREATER "-1")
        set(include_group_name_postfix "")
    else()
        set(include_group_name_postfix " headers")
    endif()


    string(FIND "${_DIR_SOURCE}" "src" result)

    if(${result} GREATER "-1")
        set(src_group_name_postfix "")
    else()
        set(src_group_name_postfix " sources")
    endif()
    
    string(FIND "${_DIR_SOURCE}" "tests" result)

    set(test_group_name_postfix "")


    source_group( "${_DIR_INCLUDE}${include_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_INCLUDE}/[^/]*\\.(h|hxx|hpp)$" )
    source_group( "${_DIR_SOURCE}${src_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_SOURCE}/[^/]*\\.(c|cpp)$" )
    source_group( "${_DIR_TEST}${test_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_TEST}/[^/]*\\.(c|cpp)$" )
    
    foreach( subfolder ${ARGN} )
        source_group( "${_DIR_INCLUDE}/${subfolder}${include_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_INCLUDE}/${subfolder}.*\\.(h|hxx|hpp)$" )
        source_group( "${_DIR_SOURCE}/${subfolder}${src_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_SOURCE}/${subfolder}.*\\.(c|cpp)$" )
        source_group( "${_DIR_TEST}/${subfolder}${test_group_name_postfix}" REGULAR_EXPRESSION "^.*/${_DIR_TEST}/${subfolder}.*\\.(c|cpp)$" )
    endforeach()

endmacro()



macro( ADD_SOURCE_FILE )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_SOURCES ${${TRIDIM_CURRENT_TARGET}_SOURCES} ${ARGV})
endmacro()

macro( ADD_HEADER_FILE )

    get_filename_component(file_path ${ARGV} DIRECTORY)
    get_filename_component(file_name ${ARGV} NAME)

    target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${file_path} )

    list( APPEND ${TRIDIM_CURRENT_TARGET}_HEADERS ${${TRIDIM_CURRENT_TARGET}_HEADERS} ${ARGV})

endmacro()

macro( ADD_SOURCE_DIRECTORY _DIR )
  if(APPLE)
    file( GLOB_RECURSE _TRIDIM_SOURCES ${_DIR}/*.c ${_DIR}/*.cpp ${_DIR}/*.cc ${_DIR}/*.mm )
  else()
    file( GLOB_RECURSE _TRIDIM_SOURCES ${_DIR}/*.c ${_DIR}/*.cpp ${_DIR}/*.cc )
  endif(APPLE)
    list( APPEND ${TRIDIM_CURRENT_TARGET}_SOURCES ${_TRIDIM_SOURCES} )
endmacro()
                                                                                
macro( ADD_HEADER_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_HEADERS ${_DIR}/*.h ${_DIR}/*.hxx ${_DIR}/*.hpp )

    list(LENGTH _TRIDIM_HEADERS LENGTH)

    if( LENGTH GREATER 0 )
        list( APPEND ${TRIDIM_CURRENT_TARGET}_HEADERS ${_TRIDIM_HEADERS} )
        
        target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${_DIR} )
    endif()

endmacro()

macro( ADD_UI_FILE )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_UI_FILES ${${TRIDIM_CURRENT_TARGET}_UI_FILES} ${ARGV} )
endmacro()

macro( ADD_RC_FILE )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_RC_FILES ${${TRIDIM_CURRENT_TARGET}_RC_FILES} ${ARGV} )
endmacro()

macro( ADD_TRANSLATION_FILE )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES ${${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES} ${ARGV} )
endmacro()

macro( ADD_UI_DIRECTORY _DIR )
    file( GLOB_RECURSE _${TRIDIM_CURRENT_TARGET}_UI_FILES ${_DIR}/*.ui )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_UI_FILES ${_${TRIDIM_CURRENT_TARGET}_UI_FILES} )
endmacro()

macro( ADD_RC_DIRECTORY _DIR )
    file( GLOB_RECURSE _${TRIDIM_CURRENT_TARGET}_RC_FILES ${_DIR}/*.rc )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_RC_FILES ${_${TRIDIM_CURRENT_TARGET}_RC_FILES} )
endmacro()

macro( ADD_TRANSLATION_DIRECTORY _DIR )
    file( GLOB_RECURSE _${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES ${_DIR}/*.ts )
    list( APPEND ${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES ${_${TRIDIM_CURRENT_TARGET}_TRANSLATION_FILES} )
endmacro()

# add library documentation resources
macro( ADD_DOCUMENTATION_RESOURCES )
    
    #include paths are the same as target/library names but without leading 3Dim in some cases and always lowercase.
    string(REPLACE "3Dim" "" result "${TRIDIM_CURRENT_TARGET}")
    string(TOLOWER "${result}" result)
    
    
    SET_PROPERTY(GLOBAL APPEND PROPERTY DOC_RESOURCES ${CMAKE_SOURCE_DIR}/${DOC_DIR}/3dim/${result}/desc.dox 
                                                      ${CMAKE_SOURCE_DIR}/${${TRIDIM_CURRENT_TARGET}_HEADERS}
                                                      ${CMAKE_SOURCE_DIR}/${${TRIDIM_CURRENT_TARGET}_SOURCES})
    
    SET_PROPERTY(GLOBAL APPEND PROPERTY DOC_IMAGES ${CMAKE_SOURCE_DIR}/${DOC_DIR}/3dim/${result}/images)
    
endmacro()

