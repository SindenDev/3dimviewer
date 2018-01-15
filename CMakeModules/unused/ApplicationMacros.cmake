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



message(FATAL_ERROR "Tohle se asi nepouziva")

# Executable name macro
macro( ADD_TRIDIM_EXECUTABLE _EXECUTABLE_NAME )
    set( TRIDIM_EXECUTABLE_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_PROJECT_NAME ${_EXECUTABLE_NAME} )
    set( TRIDIM_EXECUTABLE_HEADERS "" )
    set( TRIDIM_EXECUTABLE_SOURCES "" )

	#call to this macro is usually among the first things in cmake file
	#so use this opportunity to create new executable for use in targeted commands
	if(WIN32)
		add_executable(${TRIDIM_EXECUTABLE_NAME} WIN32 "empty.cpp")
	else()
		add_executable(${TRIDIM_EXECUTABLE_NAME} "empty.cpp")
	endif()
#    if( TRIDIM_MULTIPLE_PROJECTS )
#        set( TRIDIM_TARGET_PREFIX "${_EXECUTABLE_NAME}_" )
#    else( TRIDIM_MULTIPLE_PROJECTS )
#        set( TRIDIM_TARGET_PREFIX "" )
#    endif( TRIDIM_MULTIPLE_PROJECTS )                                          
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
    if( WIN32 )
        add_executable( ${TRIDIM_EXECUTABLE_NAME} WIN32 ${TRIDIM_EXECUTABLE_SOURCES} ${TRIDIM_EXECUTABLE_HEADERS} )
    else(WIN32)
        add_executable( ${TRIDIM_EXECUTABLE_NAME} ${TRIDIM_EXECUTABLE_SOURCES} ${TRIDIM_EXECUTABLE_HEADERS} )
    endif(WIN32)
    set_target_properties( ${TRIDIM_EXECUTABLE_NAME} PROPERTIES
                           LINKER_LANGUAGE CXX
                           PROJECT_LABEL ${TRIDIM_EXECUTABLE_PROJECT_NAME}
                           DEBUG_POSTFIX d
                           LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
#                           RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/bin"
                           )
#    if( MSVC )
#        set_target_properties(${TRIDIM_EXECUTABLE_NAME} PROPERTIES PREFIX "../" )
#    endif( MSVC )
endmacro( TRIDIM_EXECUTABLE_BUILD )


#-------------------------------------------------------------------------------
# Adding 3Dim library

macro( ADD_3DIM_LIB_TARGET _LIB_NAME )
    if( NOT _${_LIB_NAME}_INCLUDED )
        set( _${_LIB_NAME}_INCLUDED 1 )
        include( "${CMAKE_SOURCE_DIR}/src/${_LIB_NAME}.cmake" )
    endif( NOT _${_LIB_NAME}_INCLUDED )
endmacro( ADD_3DIM_LIB_TARGET )


#-------------------------------------------------------------------------------
# Adding 3Dim plugin

macro( ADD_3DIM_PLUGIN_TARGET _PLUGIN_NAME )
    if( NOT _${_PLUGIN_NAME}_INCLUDED )
        set( _${_PLUGIN_NAME}_INCLUDED 1 )
        string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )
        include( "${CMAKE_SOURCE_DIR}/plugins/${_SHORT_PLUGIN_NAME}.cmake" )
    endif( NOT _${_PLUGIN_NAME}_INCLUDED )
endmacro( ADD_3DIM_PLUGIN_TARGET )

#-------------------------------------------------------------------------------
# Adding 3Dim plugin - QT version

macro( ADD_3DIM_QTPLUGIN_TARGET _PLUGIN_NAME )
    if( NOT _${_PLUGIN_NAME}_INCLUDED )
        set( _${_PLUGIN_NAME}_INCLUDED 1 )
        string( REPLACE Plugin "" _SHORT_PLUGIN_NAME ${_PLUGIN_NAME} )
        include( "${CMAKE_SOURCE_DIR}/plugins/${_SHORT_PLUGIN_NAME}.cmake" )
#        add_subdirectory( ${CMAKE_SOURCE_DIR}/plugins/${_SHORT_PLUGIN_NAME} ${_SHORT_PLUGIN_NAME} )
    endif( NOT _${_PLUGIN_NAME}_INCLUDED )
endmacro( ADD_3DIM_QTPLUGIN_TARGET )

#-------------------------------------------------------------------------------
# Adding source groups

macro( ADD_SOURCE_GROUPS _DIR_INCLUDE _DIR_SOURCE )
  source_group( "${_DIR_INCLUDE}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE}/[^/]*\\.(h|hxx|hpp)$" )
  source_group( "${_DIR_SOURCE}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE}/[^/]*\\.(c|cpp)$" )

  foreach( arg ${ARGN} )
    source_group( "${_DIR_INCLUDE}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE}/${arg}/[^/]*\\.(h|hxx|hpp)$" )
    source_group( "${_DIR_SOURCE}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE}/${arg}/[^/]*\\.(c|cpp)$" )
  endforeach( arg )
endmacro( ADD_SOURCE_GROUPS )

# second version
macro( ADD_SOURCE_GROUPS2 _DIR_INCLUDE _DIR_SOURCE _DIR_INCLUDE2 _DIR_SOURCE2 )
  source_group( "${_DIR_INCLUDE}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE}/[^/]*\\.(h|hxx|hpp)$" )
  source_group( "${_DIR_SOURCE}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE}/[^/]*\\.(c|cpp)$" )

  foreach( arg ${ARGN} )
    source_group( "${_DIR_INCLUDE2}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_INCLUDE2}/${arg}/[^/]*\\.(h|hxx|hpp)$" )
    source_group( "${_DIR_SOURCE2}/${arg}" REGULAR_EXPRESSION ".*/${_DIR_SOURCE2}/${arg}/[^/]*\\.(c|cpp)$" )
  endforeach( arg )
endmacro( ADD_SOURCE_GROUPS2 )


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
     # make_directory( ${DESTINATION_DIR} ) #deprecated
		file(MAKE_DIRECTORY ${DESTINATION_DIR})
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
		file(MAKE_DIRECTORY ${DESTINATION_DIR})
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

	if ( BUILD_WITH_QT5 )
		QT5_WRAP_CPP(${MYARGS})
	else()
		QT4_WRAP_CPP(${MYARGS})   
	endif()
	
	set(MYARGS)       
endmacro(QTX_WRAP_CPP)

macro(QTX_WRAP_UI)
	foreach(f ${ARGN})
		list(APPEND MYARGS ${f})
	endforeach()

	if ( BUILD_WITH_QT5 )
		QT5_WRAP_UI(${MYARGS})
	else()
		QT4_WRAP_UI(${MYARGS})
	endif()
	
	set(MYARGS)
endmacro(QTX_WRAP_UI)

macro(QTX_ADD_RESOURCES)
	foreach(f ${ARGN})
		list(APPEND MYARGS ${f})
	endforeach()

	if ( BUILD_WITH_QT5 )
		QT5_ADD_RESOURCES(${MYARGS})
	else()
		QT4_ADD_RESOURCES(${MYARGS})
	endif()

	set(MYARGS)
endmacro(QTX_ADD_RESOURCES)   

macro(qtx_create_translation)
	foreach(f ${ARGN})
		list(APPEND MYARGS ${f})
	endforeach()

	if ( BUILD_WITH_QT5 )
		qt5_create_translation(${MYARGS})
	else()
		qt4_create_translation(${MYARGS})
	endif()
	
	set(MYARGS)      
endmacro(qtx_create_translation)  
    
macro(qtx_add_translation)
	foreach(f ${ARGN})
		list(APPEND MYARGS ${f})
	endforeach()

	if ( BUILD_WITH_QT5 )
		qt5_add_translation(${MYARGS})
	else()
		qt4_add_translation(${MYARGS})
	endif()
	set(MYARGS)         
endmacro(qtx_add_translation)

macro(QTX_GENERATE_MOC)
	foreach(f ${ARGN})
		list(APPEND MYARGS ${f})
	endforeach()

	if ( BUILD_WITH_QT5 )
		QT5_GENERATE_MOC(${MYARGS})
	else()
		QT4_GENERATE_MOC(${MYARGS})
	endif()
	
	set(MYARGS)       
endmacro(QTX_GENERATE_MOC)

#-------------------------------------------------------------------------------