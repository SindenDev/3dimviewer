#===============================================================================
# $Id: InstallMacros.txt 1283 2011-04-28 11:26:26Z spanel $
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
# Install shaders

macro( INSTALL_SHADERS )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders/*.frag" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders/*.rgba" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders/*.gray" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders/*.vert")
  install(FILES ${files} DESTINATION Debug/shaders CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION RelWithDebInfo/shaders CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION Release/shaders CONFIGURATIONS Release)
endmacro()

#-------------------------------------------------------------------------------
# Install shaders 2

macro( INSTALL_SHADERS2 )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders2/*.frag" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders2/*.rgba" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders2/*.gray" "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/shaders2/*.vert")
  install(FILES ${files} DESTINATION Debug/shaders2 CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION RelWithDebInfo/shaders2 CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION Release/shaders2 CONFIGURATIONS Release)
endmacro()

#-------------------------------------------------------------------------------
# Install qt translations

macro( INSTALL_TRANSLATIONS ) 

    unset(qm_files)
    FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/translations/*.ts")
   
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

endmacro()


#-------------------------------------------------------------------------------
# Install images

macro( INSTALL_IMAGES )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/images/?*.*" )  
  install(FILES ${files} DESTINATION  Debug/images CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/images CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/images CONFIGURATIONS Release)

endmacro()

#-------------------------------------------------------------------------------
# Install models

macro( INSTALL_MODELS )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/models/?*.*" )
  install(FILES ${files} DESTINATION  Debug/models CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/models CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/models CONFIGURATIONS Release)

endmacro()

#-------------------------------------------------------------------------------
# Install icons

macro( INSTALL_ICONS )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/icons/?*.*" )
  install(FILES ${files} DESTINATION  Debug/icons CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/icons CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/icons CONFIGURATIONS Release)

endmacro()

#-------------------------------------------------------------------------------
# Install python interpret


macro( INSTALL_PYTHON )
  #install( DIRECTORY "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}" DESTINATION Debug/python CONFIGURATIONS Debug )
  #install( DIRECTORY "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}" DESTINATION RelWithDebInfo/python CONFIGURATIONS RelWithDebInfo )
  #install( DIRECTORY "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}" DESTINATION Release/python CONFIGURATIONS Release )
endmacro()

#-------------------------------------------------------------------------------
# Install documentation

macro( INSTALL_DOCS )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/doc/?*.*" )
  install(FILES ${files} DESTINATION  Debug/doc CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/doc CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/doc CONFIGURATIONS Release)

endmacro()

#-------------------------------------------------------------------------------
# Install in-app help

macro( INSTALL_HELP )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/help/?*.*" )
  install(FILES ${files} DESTINATION  Debug/help CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/help CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/help CONFIGURATIONS Release)
endmacro()

#-------------------------------------------------------------------------------
# Install fonts

macro( INSTALL_FONTS )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/fonts/?*.*" )
  install(FILES ${files} DESTINATION  Debug/fonts CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/fonts CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/fonts CONFIGURATIONS Release)

endmacro()

macro( INSTALL_STYLES )
  FILE( GLOB files "${TRIDIM_APPLICATION_SOURCE_FOLDER_PATH}/${TRIDIM_CURRENT_TARGET}/styles/?*.*" )
  install(FILES ${files} DESTINATION  Debug/styles CONFIGURATIONS Debug)
  install(FILES ${files} DESTINATION  RelWithDebInfo/styles CONFIGURATIONS RelWithDebInfo)
  install(FILES ${files} DESTINATION  Release/styles CONFIGURATIONS Release)
endmacro()


macro( INSTALL_FILES_TO_BIN files_to_install debug_files_to_install subfolder)

    install(FILES ${debug_files_to_install} DESTINATION Debug/${subfolder}			CONFIGURATIONS Debug)
    install(FILES ${files_to_install}		DESTINATION RelWithDebInfo/${subfolder} CONFIGURATIONS RelWithDebInfo)
    install(FILES ${files_to_install}		DESTINATION Release/${subfolder}		CONFIGURATIONS Release)
        
endmacro()

macro( INSTALL_FILES_TO_BIN_NO_DEBUG files_to_install subfolder)

    install(FILES ${files_to_install}       DESTINATION Debug/${subfolder}			CONFIGURATIONS Debug)
    install(FILES ${files_to_install}		DESTINATION RelWithDebInfo/${subfolder} CONFIGURATIONS RelWithDebInfo)
    install(FILES ${files_to_install}		DESTINATION Release/${subfolder}		CONFIGURATIONS Release)
        
endmacro()

#general macro suitable for small libs where you don't want to pick and choose which dlls get installed
#separates all dlls into two lists and if debug versions are present it uses that or if it's not
# it uses the release ones instead
function(INSTALL_LIB libname)

    if(NOT "${${libname}_FOUND}" STREQUAL "TRUE")
        message("Library ${libname} is not present and cannot be installed.")
        return()
    endif()
       
    
    FILELIST(files "${${libname}_BINARY_DIRS}")
    
    
    #filter out non dll files
    foreach(file ${files})
    
        string(REGEX MATCH "^.*.dll$" result ${file})

        if(NOT result STREQUAL "")
            list(APPEND dll_files ${file})
        endif()

    endforeach()

    #message("${files}")
    
    #find if it has debug or is it only release dll that is present..
    foreach(file ${dll_files})
    
        string(REGEX MATCH "^.*d\\.dll$" result ${file})
                
        if(result STREQUAL "")
            list(APPEND optimized_libs ${file})
        else()			
            list(APPEND debug_libs ${file})
        endif()
    
        #workaround for edge case with libs ending with 'd' and debug versions with 'dd'
        #upon detecting XXXdd.lib the coresponding XXXd.lib must be already in optimized_libs
        #so take it from there and place it into correct list.
        string(REGEX REPLACE "^(.*d)d\\.lib$" "\\1" dd_result ${file})
            #message("${filename} ${dd_result}")

        if(NOT dd_result STREQUAL "" AND NOT ${dd_result} STREQUAL ${file})
            #message("${filename} ${dd_result}")
            list(REMOVE_ITEM debug_libs "${dd_result}.dll")
            list(APPEND optimized_libs "${dd_result}.dll")
        endif()
            
        set(dd_result "")
        set(result "")
            
    endforeach()
    
    list(LENGTH debug_libs debug_length)
    list(LENGTH optimized_libs optimized_length)

    #message("debug ${debug_libs}")
    #message("optimized ${optimized_libs}")

    #If there are debug version but their count is not on par with release version, raise a red flag.
    if(NOT "${debug_length}" EQUAL "${optimized_length}" AND "${debug_length}" GREATER 0)
        message(WARNING "Counts of debug and optimized dlls of library ${libname} are different from each other..")
    endif()
    
    if("${debug_length}" GREATER 0)   
    
        foreach(item ${debug_libs})
            install(FILES ${${libname}_BINARY_DIRS}/${item} DESTINATION Debug			CONFIGURATIONS Debug)
        endforeach()
        
        foreach(item ${optimized_libs})
            install(FILES ${${libname}_BINARY_DIRS}/${item} DESTINATION RelWithDebInfo  CONFIGURATIONS RelWithDebInfo)
            install(FILES ${${libname}_BINARY_DIRS}/${item}	DESTINATION Release 		CONFIGURATIONS Release) 
        endforeach()
     
    else()

        foreach(item ${optimized_libs})
            install(FILES ${${libname}_BINARY_DIRS}/${item} DESTINATION Debug			CONFIGURATIONS Debug)
            install(FILES ${${libname}_BINARY_DIRS}/${item}	DESTINATION RelWithDebInfo  CONFIGURATIONS RelWithDebInfo)
            install(FILES ${${libname}_BINARY_DIRS}/${item}	DESTINATION Release 		CONFIGURATIONS Release) 
        endforeach()
        
    endif()
endfunction()

function(INSTALL_REQUIRED_SYSTEM_LIBS_TO_BIN)

    if(NOT WIN32)
        return()
    endif()

    #Install basic ms redistributable stuff everytime..
    if(NOT BUILD_INSTALL_ALSO_REQUIRED_SYSTEM_LIBS)
    
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)    #prevent immediate install
        set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)            #to not leave out openMP library

        include(InstallRequiredSystemLibraries)             #this does the work

        message(STATUS "Found required system libraries: ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")

        string(REPLACE "\\" "/" fixed_CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")

        INSTALL_FILES_TO_BIN_NO_DEBUG("${fixed_CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" .)

        return()
    endif()
    
    
    
    
    
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)    #prevent immediate install
    set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)            #to not leave out openMP library
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)              #this catches all the tiny ucrt libraries
        
    include(InstallRequiredSystemLibraries)             #this does the work
        
    message(STATUS "Found required system libraries: ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
        
    # https://braintrekking.wordpress.com/2013/04/27/dll-hell-how-to-include-microsoft-redistributable-runtime-libraries-in-your-cmakecpack-project/    
    # https://cmake.org/cmake/help/v3.9/module/InstallRequiredSystemLibraries.html
    
    
    ## in the case that the location was not found and was supplied by the user, let's make sure, that the given path is in correct format
    string(REPLACE "\\" "/" fixed_CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")

    #message(${fixed_CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

    #aaaaand install them..
    INSTALL_FILES_TO_BIN_NO_DEBUG("${fixed_CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" .)
        
    #naplni MSVC_REDIST_DIR a WINDOWS_KITS_DIR - bere to z registru podle msvc instalace..
        
        
endfunction()
    
macro( GET_OSG_FILES osg_dll_list osg_debug_dll_list files_list files_debug_list )

    #gather all dlls including those in plugins subfolder
    file(GLOB_RECURSE osg_dlls "${${OSG_LIB_NAME}_BINARY_DIRS}/*.dll")

    #select only those dll items that match something in dll_list while ignoring version prefixes
    foreach(item ${osg_dll_list})

        foreach(dll ${osg_dlls})
        
            string(REGEX MATCH ".*${item}" result ${dll})
            
            if("${result}" STRGREATER "")
                break()
            endif()
        endforeach()
      
        list(APPEND ${files_list} ${result})
    endforeach()
    
    foreach(item ${osg_debug_dll_list})

        foreach(dll ${osg_dlls})
        
            string(REGEX MATCH ".*${item}" result ${dll})
            
            if("${result}" STRGREATER "")
                break()
            endif()       
        endforeach()
        
        list(APPEND ${files_debug_list} ${result})
    endforeach()
    
endmacro()

#if this function is in any project, it's install script gets run after succesfull build..
function(RUN_INSTALL_AS_POST_BUILD)

    if(NOT BUILD_RUN_INSTALL_AS_POST_BUILD)
        return()
    endif()

    #CMAKE_INSTALL_CONFIG_NAME is a thing that gets tested in install script and we have to set it correctly.. 
    #CMAKE_BUILD_TYPE gets tested too but cannot be set by parameter
    set(project_install_script_path "${CMAKE_BINARY_DIR}/${TRIDIM_CURRENT_TARGET}/cmake_install.cmake")
    
    set(command_string_release "${CMAKE_COMMAND}" -D CMAKE_INSTALL_CONFIG_NAME:STRING=Release -P \"${project_install_script_path}\" )
    set(command_string_debug "${CMAKE_COMMAND}" -D CMAKE_INSTALL_CONFIG_NAME:STRING=Debug -P \"${project_install_script_path}\" )
    set(command_string_relwithdebinfo "${CMAKE_COMMAND}" -D CMAKE_INSTALL_CONFIG_NAME:STRING=RelWithDebInfo -P \"${project_install_script_path}\" )
   
    # http://www.easydos.com/dosindex.html

if (WIN32)
    add_custom_command(
        TARGET ${TRIDIM_CURRENT_TARGET} POST_BUILD
        COMMAND if $<CONFIG:Release> neq 0 ${command_string_release}
        COMMAND if $<CONFIG:Debug> neq 0 ${command_string_debug}
        COMMAND if $<CONFIG:RelWithDebInfo> neq 0 ${command_string_relwithdebinfo}
        USES_TERMINAL
    )
else()
    add_custom_command(
        TARGET ${TRIDIM_CURRENT_TARGET} POST_BUILD
        COMMAND ${command_string_release}
        COMMAND ${command_string_debug}
        COMMAND ${command_string_relwithdebinfo}
        USES_TERMINAL
    )   
endif()

    
    if(NOT BUILD_RUN_INSTALL_AS_POST_BUILD_EVERYTIME)
        return()
    endif()
    
    # http://nickdademo.blogspot.cz/2015/03/cmake-always-running-post-build-step.html
    #this makes fake dependency which always touches empty.hpp which triggers post build events everytime but does not cause linking to happen..
    add_custom_target(ALWAYS_DO_POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E echo touching empty.hpp
        COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/empty.hpp"
    )
    
    set_target_properties(ALWAYS_DO_POST_BUILD PROPERTIES FOLDER CMakePredefinedTargets)
    
    add_dependencies(${TRIDIM_CURRENT_TARGET} ALWAYS_DO_POST_BUILD)
    
endfunction()
