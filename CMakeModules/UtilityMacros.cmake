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
# Add copy multiple files as a post build command - additional parameter is mask

#macro( COPY_FILES _TARGET _DIR_SOURCE _DIR_DESTINATION )
#    foreach( arg ${ARGN} )
#        set( MASK ${arg} )
#        set( SOURCE_DIR "${CMAKE_SOURCE_DIR}${_DIR_SOURCE}" )
#        set( DESTINATION_DIR "${PROJECT_BINARY_DIR}${_DIR_DESTINATION}" )
#    
#        # Create file list
#        file( GLOB FILES_LIST RELATIVE ${SOURCE_DIR} "${SOURCE_DIR}${MASK}" )
#    
#        # Create destination directory, if it does not exist
#        if( NOT EXISTS ${DESTINATION_DIR} )
#            file(MAKE_DIRECTORY ${DESTINATION_DIR} )
#        endif()
#    
#        # Add copy command
#        foreach( filename ${FILES_LIST} )
#            # Create native paths
#            file( TO_NATIVE_PATH ${SOURCE_DIR}${filename} native_src )
#            file( TO_NATIVE_PATH ${DESTINATION_DIR}${filename} native_dst )
#      
#            # Add custom command
#            add_custom_command( TARGET ${_TARGET}
#                                POST_BUILD
#                                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${native_src} ${native_dst}
#                #               COMMENT "Copying file, if it is needed: ${filename}"
#                                )
#        endforeach()
#    endforeach()
#endmacro()


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
        endif()

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
        endforeach()
    endforeach()
endmacro()

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
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MACHINESEGMENTATION_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MODELCREATE_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_LOOKUPTABLEEDITOR_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_SEGTOSMALLREGIONS_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_REGIONANDMODELCONTROL_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DATASHIFT_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_EXPERIMENTALSEG_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_REGIONBOOLEANOPERATIONS_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_MULTICLASSAUTOSEG_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_SEGREFINEMENTANDINTERPOLATION_PLUGIN} )
  ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_GRAPHCUT_PLUGIN} )
  
  if(BUILD_WITH_PYTHON)
    ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_PYTHONSCRIPTING_PLUGIN} )
    if(BUILD_WITH_DEEPLEARNING)
        ADD_3DIM_QTPLUGIN_TARGET( ${TRIDIM_DEEPLEARNING_PLUGIN} )
    endif()
  endif()

endmacro()


