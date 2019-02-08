#===============================================================================
# $Id: ApplicationMacros.txt 1283 2011-04-28 11:26:26Z spanel $
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

# Executable name macro
macro( ADD_TRIDIM_EXECUTABLE _EXECUTABLE_NAME )	

    #this updates TRIDIM_CURRENT_TARGET
    push_target_stack(${_EXECUTABLE_NAME})
    
    set( TRIDIM_EXECUTABLE_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_PROJECT_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_HEADERS "" )
    set( TRIDIM_EXECUTABLE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" )

    source_group("CMake files" FILES "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

    set(icon "")

    if(EXISTS ${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc)
        set(icon "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc")
    endif()
    
    #call to this macro is usually among the first things in cmake file
    #so use this opportunity to create new executable for use in targeted commands
    if(WIN32)
        add_executable(${TRIDIM_EXECUTABLE_NAME} WIN32 "${CMAKE_BINARY_DIR}/empty.hpp" "${icon}")
    else()
        add_executable(${TRIDIM_EXECUTABLE_NAME} "${CMAKE_BINARY_DIR}/empty.hpp")
    endif()
    
    #This eliminates most of the autogen warnings about empty file. Also in Library and plugin macros.
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    if (BUILD_USE_PRECOMPILED_HEADERS AND MSVC)
    
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)

        ADD_EXECUTABLE_HEADER_FILE(${pcheader_h_location})
        ADD_EXECUTABLE_SOURCE_FILE(${pcheader_cpp_location})

        set_source_files_properties(${pcheader_cpp_location}
          PROPERTIES
          COMPILE_FLAGS "/Ycpcheader.h")
        
        target_compile_options(${TRIDIM_EXECUTABLE_NAME} PRIVATE /Yupcheader.h /FIpcheader.h)

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
    
    set( TRIDIM_EXECUTABLE_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_PROJECT_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_HEADERS "" )
    set( TRIDIM_EXECUTABLE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" )

    source_group("CMake files" FILES "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

    set(icon "")
    
    if(EXISTS ${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc)
        set(icon "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/appicon.rc")	
    endif()
    
    #call to this macro is usually among the first things in cmake file
    #so use this opportunity to create new executable for use in targeted commands
    if(WIN32)
        add_executable(${TRIDIM_EXECUTABLE_NAME} "${CMAKE_BINARY_DIR}/empty.hpp" "${icon}")
    else()
        add_executable(${TRIDIM_EXECUTABLE_NAME} "${CMAKE_BINARY_DIR}/empty.hpp")
    endif()
    
    set_property(SOURCE ${CMAKE_BINARY_DIR}/empty.hpp PROPERTY SKIP_AUTOGEN ON)

    if (USE_PRECOMPILED_HEADERS AND MSVC)
        
        find_file(pcheader_h_location pcheader.h ${CMAKE_SOURCE_DIR}/oss/include/3dim/ ${CMAKE_SOURCE_DIR}/include/3dim/)
        find_file(pcheader_cpp_location pcheader.cpp ${CMAKE_SOURCE_DIR}/oss/src/ ${CMAKE_SOURCE_DIR}/src/)
   
        ADD_EXECUTABLE_HEADER_FILE(${pcheader_h_location})
        ADD_EXECUTABLE_SOURCE_FILE(${pcheader_cpp_location})
   
        set_source_files_properties(${pcheader_cpp_location}
          PROPERTIES
          COMPILE_FLAGS "/Ycpcheader.h")
        
       target_compile_options(${TRIDIM_EXECUTABLE_NAME} PRIVATE Yupcheader.h FIpcheader.h)
       
       unset(pcheader_h_location CACHE)
       unset(pcheader_cpp_location CACHE)
        
    endif()    
            
    set_target_properties(${TRIDIM_CURRENT_TARGET} PROPERTIES AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${BUILD_PROJECT_NAME}/Autogen/${TRIDIM_CURRENT_TARGET})
           
    RUN_INSTALL_AS_POST_BUILD()

    if(MSVC)
        add_dependencies(${BUILD_PROJECT_NAME}_CUSTOM_INSTALL_TARGET ${TRIDIM_CURRENT_TARGET} )
    endif()

endmacro()


# Add source file
macro( ADD_EXECUTABLE_SOURCE_FILE )
    set( TRIDIM_EXECUTABLE_SOURCES ${TRIDIM_EXECUTABLE_SOURCES} ${ARGV})
endmacro( ADD_EXECUTABLE_SOURCE_FILE )

# Add header file to the EXECUTABLE
macro( ADD_EXECUTABLE_HEADER_FILE )
    set( TRIDIM_EXECUTABLE_HEADERS ${TRIDIM_EXECUTABLE_HEADERS} ${ARGV})
endmacro( ADD_EXECUTABLE_HEADER_FILE )

# Add sources directory - adds all source files from the directory
macro( ADD_EXECUTABLE_SOURCE_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_EXECUTABLE_SOURCES ${_DIR}/*.c ${_DIR}/*.cpp ${_DIR}/*.cc )
    list( APPEND TRIDIM_EXECUTABLE_SOURCES ${_TRIDIM_EXECUTABLE_SOURCES} )
endmacro( ADD_EXECUTABLE_SOURCE_DIRECTORY )
                                                                                
# Add include directory - adds all headers from the directory
macro( ADD_EXECUTABLE_HEADER_DIRECTORY _DIR )
    file( GLOB_RECURSE _TRIDIM_EXECUTABLE_HEADERS ${_DIR}/*.h ${_DIR}/*.hxx ${_DIR}/*.hpp )
list(LENGTH _TRIDIM_EXECUTABLE_HEADERS LENGTH)
    if( LENGTH GREATER 0 )
        list( APPEND TRIDIM_EXECUTABLE_HEADERS ${_TRIDIM_EXECUTABLE_HEADERS} )
        include_directories( ${_DIR} )
    endif( LENGTH GREATER 0 )
endmacro( ADD_EXECUTABLE_HEADER_DIRECTORY )

# Add dependency
macro( ADD_EXECUTABLE_DEPENDENCY _LIB )
    target_link_libraries( ${TRIDIM_EXECUTABLE_NAME} ${_LIB} )
endmacro( ADD_EXECUTABLE_DEPENDENCY )

# Build macro
macro( TRIDIM_EXECUTABLE_BUILD )

message("TRIDIM_EXECUTABLE_BUILD se prece jen pouziva..")

    set_target_properties( ${TRIDIM_EXECUTABLE_NAME} PROPERTIES
                           LINKER_LANGUAGE CXX
                           PROJECT_LABEL ${TRIDIM_EXECUTABLE_PROJECT_NAME}
                           DEBUG_POSTFIX d
                           LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
#                           RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           #RUNTIME_OUTPUT_DIRECTORY_DEBUG "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_EXECUTABLE_NAME}/bin"
                           #RUNTIME_OUTPUT_DIRECTORY_RELEASE "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_EXECUTABLE_NAME}/bin"
                           #RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_EXECUTABLE_NAME}/bin"
                           #RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_EXECUTABLE_NAME}/bin"                           
)
#    if( MSVC )
#        set_target_properties(${TRIDIM_EXECUTABLE_NAME} PROPERTIES PREFIX "../" )
#    endif( MSVC )
endmacro()


#-------------------------------------------------------------------------------
# Adding 3Dim library

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
#Use only after target_sources! or it won't have desired effect..
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

# second version
#macro( ADD_SOURCE_GROUPS2 _DIR_INCLUDE _DIR_SOURCE _DIR_INCLUDE2 _DIR_SOURCE2 )
#  source_group( "${_DIR_INCLUDE}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE}/[^/]*\\.(h|hxx|hpp)$" )
#  source_group( "${_DIR_SOURCE}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE}/[^/]*\\.(c|cpp)$" )
#
#  foreach( arg ${ARGN} )
#    source_group( "${_DIR_INCLUDE2}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE2}/${arg}/[^/]*\\.(h|hxx|hpp)$" )
#    source_group( "${_DIR_SOURCE2}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE2}/${arg}/[^/]*\\.(c|cpp)$" )
#  endforeach( arg )
#endmacro( ADD_SOURCE_GROUPS2 )


#-------------------------------------------------------------------------------
# Add copy multiple files as a post build command - additional parameter is mask

macro( COPY_FILES _TARGET _DIR_SOURCE _DIR_DESTINATION )
  foreach( arg ${ARGN} )
    set( MASK ${arg} )
    set( SOURCE_DIR "${CMAKE_SOURCE_DIR}${_DIR_SOURCE}" )
    set( DESTINATION_DIR "${PROJECT_BINARY_DIR}${_DIR_DESTINATION}" )
    
    # Create file list
    file( GLOB FILES_LIST RELATIVE ${SOURCE_DIR} "${SOURCE_DIR}${MASK}" )
    
    # Create destination directory, if it does not exist
    if( NOT EXISTS ${DESTINATION_DIR} )
      file(MAKE_DIRECTORY ${DESTINATION_DIR} )
    endif( NOT EXISTS ${DESTINATION_DIR} )
    
    # Add copy command
    foreach( filename ${FILES_LIST} )
      # Create native paths
      file( TO_NATIVE_PATH ${SOURCE_DIR}${filename} native_src )
      file( TO_NATIVE_PATH ${DESTINATION_DIR}${filename} native_dst )
      
      # Add custom command
      add_custom_command( TARGET ${_TARGET}
                          POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${native_src} ${native_dst}
#                          COMMENT "Copying file, if it is needed: ${filename}"
                          )
    endforeach( filename )
  endforeach( arg )
endmacro(COPY_FILES)


#-------------------------------------------------------------------------------
# Add copy multiple files from any directory as a post build command - additional paramter is mask

#macro( COPY_ANY_FILES _TARGET _DIR_SOURCE _DIR_DESTINATION _COPY_TARGET )
macro( COPY_ANY_FILES _TARGET _DIR_SOURCE _DIR_DESTINATION )
  foreach( arg ${ARGN} )
    set( MASK ${arg} )
    set( SOURCE_DIR ${_DIR_SOURCE} )
    set( DESTINATION_DIR ${_DIR_DESTINATION} )

    # Create file list
    file( GLOB FILES_LIST RELATIVE ${SOURCE_DIR} ${SOURCE_DIR}${MASK} )
    
    # Create destination directory, if it does not exist
    if( NOT EXISTS ${DESTINATION_DIR} )
      file(MAKE_DIRECTORY ${DESTINATION_DIR} )
    endif( NOT EXISTS ${DESTINATION_DIR} )

    # Add copy command
    foreach( filename ${FILES_LIST} )
      # Create native paths
      file( TO_NATIVE_PATH ${SOURCE_DIR}/${filename} native_src )
      file( TO_NATIVE_PATH ${DESTINATION_DIR}/${filename} native_dst )
    
      # Add custom command
      add_custom_command( TARGET ${_TARGET}
                          POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${native_src} ${native_dst}
                          COMMENT "Copying file, if it is needed: ${filename}"
#                          DEPENDS ${_COPY_TARGET}
                          )
    endforeach( filename )
  endforeach( arg )
endmacro(COPY_ANY_FILES)

#-------------------------------------------------------------------------------
# QT related macros

macro(QTX_WRAP_CPP)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()
   
    QT5_WRAP_CPP(${MYARGS})
 
    set(MYARGS)  
endmacro()

macro(QTX_WRAP_UI)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()
    
    QT5_WRAP_UI(${MYARGS})
  
    set(MYARGS)
endmacro()

macro(QTX_ADD_RESOURCES)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()

    QT5_ADD_RESOURCES(${MYARGS})

    set(MYARGS)
endmacro()   

macro(qtx_create_translation)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()
    
    qt5_create_translation(${MYARGS})
    
    set(MYARGS)      
endmacro()  
    
macro(qtx_add_translation)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()
    
    qt5_add_translation(${MYARGS})

    set(MYARGS)         
endmacro()

macro(QTX_GENERATE_MOC)
    foreach(f ${ARGN})
        list(APPEND MYARGS ${f})
    endforeach()
  
    QT5_GENERATE_MOC(${MYARGS})
       
    set(MYARGS)       
endmacro()


#-------------------------------------------------------------------------------

macro( INCLUDE_BASIC_HEADERS )

    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_SOURCE_DIR}/oss/include/
        ${CMAKE_SOURCE_DIR}/include/3dim/
        ${CMAKE_SOURCE_DIR}/oss/include/3dim/
        ${CMAKE_SOURCE_DIR}/oss/include/3dim/core/
        ${CMAKE_SOURCE_DIR}/oss/include/3dim/graph/
#    ${CMAKE_SOURCE_DIR}/include/3dim/graph${TRIDIM_LIBRARY_EXT}/
    )
    
    ##z jakeho duvodu to nema byt?
    if( NOT BUILD_BlueSkyPlan )
        target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/include/3dim/adv/)
    endif()
  
    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/core${TRIDIM_LIBRARY_EXT}/)
    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graph${TRIDIM_LIBRARY_EXT}/)

endmacro()

macro( INCLUDE_MEDICORE_HEADERS )

    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/oss/include/3dim/coremedi/)
    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/oss/include/3dim/graphmedi/)
    
    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/coremedi${TRIDIM_LIBRARY_EXT}/)
    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graphmedi${TRIDIM_LIBRARY_EXT}/)

endmacro()

macro( INCLUDE_BASIC_OSS_HEADERS )

    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/core/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/core${TRIDIM_LIBRARY_EXT}/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graph/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graph${TRIDIM_LIBRARY_EXT}/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/geometry/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/geometry${TRIDIM_LIBRARY_EXT}/ )
    
endmacro()

macro( INCLUDE_MEDICORE_OSS_HEADERS )

    target_include_directories( ${TRIDIM_CURRENT_TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/coremedi/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/coremedi${TRIDIM_LIBRARY_EXT}/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graphmedi/
        ${CMAKE_SOURCE_DIR}/${OSS_SUBFOLDER}include/3dim/graphmedi${TRIDIM_LIBRARY_EXT}/ )
    
endmacro()

macro(ADD_NON_OSS_PLUGINS)

  if(NOT BUILD_WITH_GDCM)
    ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DATAEXPRESS_PLUGIN} )
  endif()
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_AUTOSEGANDFILTERING_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MANUALSEG_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MANUALSEG3D_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MODELCREATE_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_LOOKUPTABLEEDITOR_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_SEGTOSMALLREGIONS_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_REGIONANDMODELCONTROL_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DATASHIFT_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_EXPERIMENTALSEG_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_REGIONBOOLEANOPERATIONS_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MULTICLASSAUTOSEG_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_SEGREFINEMENTANDINTERPOLATION_PLUGIN} )
  
  if(BUILD_WITH_PYTHON)
      ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_PYTHONSCRIPTING_PLUGIN} )
      if(BUILD_WITH_DEEPLEARNING)
        ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DEEPLEARNING_PLUGIN} )
      endif()
  endif(BUILD_WITH_PYTHON)

endmacro(ADD_NON_OSS_PLUGINS)


