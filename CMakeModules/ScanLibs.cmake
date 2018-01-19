


#lists all subdirectories of given current directory
macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist "${child}")
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

#lists all files in given current directory
macro(FILELIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(filelist "")
  foreach(child ${children})
    if(NOT IS_DIRECTORY ${curdir}/${child})
      list(APPEND filelist "${child}")
    endif()
  endforeach()

  set(${result} ${filelist})
endmacro()


macro(delete_previously_detected_libs)	
    #unset all previously set lib cache variables to prevent adding new ones to preexisting ones which are now irrelevant.
    foreach(variable_name ${previously_detected_libs})
        unset(${variable_name} CACHE)	
    endforeach()
endmacro()


macro(unset_options _options_to_unset)
    foreach(_option ${_options_to_unset})
        unset(${_option} CACHE)	
    endforeach()
endmacro()

macro(version_sort _list)

    #message("input ${${_list}}")
    unset(resulting_list)
    
    #double dereference.. _list was passed only the name of the variable so this macro is able to modify it.
    foreach(item1 ${${_list}})
        
        #message("processing item ${item1}")

        #plain insert of first item..
        if(NOT resulting_list)
            list(APPEND resulting_list ${item1})
            list(APPEND resulting_list "999999999") #stop mark.
            continue()
        endif()
    
        #get only the version string..
        string(FIND ${item1} " " _index)

        math(EXPR _result "${_index} + 1")
        string(SUBSTRING "${item1}" ${_result} -1 version1)
        
        
        set(pointer 0)
        
        foreach(item2 ${resulting_list})
                
            #message("comparing with ${item2}")
        
            string(FIND ${item2} " " _index)
            math(EXPR _result "${_index} + 1")
            string(SUBSTRING ${item2} ${_result} -1 version2)
        
            if("${version2}" VERSION_GREATER "${version1}")
                list(INSERT resulting_list ${pointer} ${item1})
                break()
            endif()
            
            math(EXPR pointer "${pointer} + 1")
        endforeach()

        
    endforeach()
    
    list(REMOVE_ITEM resulting_list "999999999")

    #message("output ${resulting_list}")

    #_list contains name of variable we want to modify
    set(${_list} ${resulting_list})
    
endmacro()

#checks whether the debug_name is present in debug_list
macro(has_debug_version result debug_name filename debug_list)

    #create debug name from filename
    string(REPLACE ".lib" "d.lib" ${debug_name} ${filename})

    
    list(FIND ${debug_list} ${${debug_name}} found)
    
    #message("searched list ${${debug_list}}")
    #message("searched for name ${${debug_name}}")	
    #message("result: ${found}")
    
    if(${found} GREATER "-1")
        set(${result} TRUE)
    else()
        set(${result} FALSE)
    endif()
    
endmacro()
        
#it's a function so I can return() from it
function(scan_libs)

    #prevent running this multiple times in one run.
    if(NOT DEFINED scan_libs_already_run)

        #not cached.. will run at most once each configure..
        set(scan_libs_already_run TRUE PARENT_SCOPE)
    else()
        message(WARNING "Invalid attempt to run scan_libs for the second time in one run of the script. Returning.")
        return()
    endif()

    message(STATUS ${CMAKE_GENERATOR})

    #all_library_names is defined in AddLibsMacros to keep all the names in one place to not overcomplicate maintenance
    if(NOT all_library_names)
        message(FATAL_ERROR "ScanLibs.cmake should be included only after including AddLibsMacros.cmake..")
    endif()
    
    #prevent undefined variable errors and set both regular and capitalized variables to false..
    foreach(name ${all_library_names})
        set(${name}_FOUND FALSE CACHE INTERNAL "")

        string(TOUPPER ${name} capitalized_name)
        set(${capitalized_name}_FOUND FALSE CACHE INTERNAL "")
    endforeach()

    #on Unix/Apple this approach is not applicable
    if(NOT WIN32)
        #message("Not on Windows. We will have to make do with finds.")
        return()
    endif()
    

    #deduce desired architecture from selected generator.
    if(CMAKE_GENERATOR MATCHES "^.*Win64$")
        set (architecture "x64")
        message(STATUS "x64 architecture")
    else()
        set (architecture "x32")
        message(STATUS "x32 architecture")
    endif ()

    #convert to list and pull out compiler version
    string(REPLACE " " ";" list_CMAKE_GENERATOR ${CMAKE_GENERATOR})

    list(GET list_CMAKE_GENERATOR 2 compiler_version)

    message(STATUS "CompilerID ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Compiler version ${compiler_version}")
      
    if(NOT "${CMAKE_GENERATOR_TOOLSET}" STREQUAL "")
        message(STATUS  "User specified toolset ${CMAKE_GENERATOR_TOOLSET}")

        #assuming here that it is something like v141
        string(SUBSTRING ${CMAKE_GENERATOR_TOOLSET} 1 2 toolset_number)

    else()
        message(STATUS  "Generator's toolset ${CMAKE_VS_PLATFORM_TOOLSET}")
        
        string(SUBSTRING ${CMAKE_VS_PLATFORM_TOOLSET} 1 2 toolset_number)
  
    endif()
    
    
    
    #this is without effect when the value is overwritten by the user.
    set(BUILD_PATH_TO_PREBUILT_LIBS "X:/msvc${toolset_number}/${architecture}" CACHE PATH "This is autodetected as X:/msvcX/architecture from selected generator and its (or user given) toolset.")
    


    if(NOT EXISTS "${BUILD_PATH_TO_PREBUILT_LIBS}")
        message(WARNING "Path '${BUILD_PATH_TO_PREBUILT_LIBS}' does not exist!")
        set(scan_libs_already_run FALSE)

        set(SCAN_LIBS_SUCCESS FALSE PARENT_SCOPE)
        return()
    else()
        message("The path to libraries is now ${BUILD_PATH_TO_PREBUILT_LIBS}.")
        set(SCAN_LIBS_SUCCESS TRUE PARENT_SCOPE)

    endif()
    
    #The deduced/given directory should exist or there is no point in continuing.
    if(NOT IS_DIRECTORY ${BUILD_PATH_TO_PREBUILT_LIBS})	
        #delete_previously_detected_libs()
        message(FATAL_ERROR "${BUILD_PATH_TO_PREBUILT_LIBS} is not an existing directory.")
    endif()
    
    
    #List all present library folders and make sure they are alphabetically sorted.
    SUBDIRLIST(SUBDIRS ${BUILD_PATH_TO_PREBUILT_LIBS})

    list(LENGTH SUBDIRS len)
    if(${len} LESS 1)
        message(FATAL_ERROR "${BUILD_PATH_TO_PREBUILT_LIBS} contains no valid folders.")
    endif()
    
    list(SORT SUBDIRS)


    #this creates list of library versions from directory names
    #and puts it into ${current_libname}_property_string variable.
    #Result is list of library names and their property strings.
    set(library_names "")
    set(current_libname "")

    foreach(dirname ${SUBDIRS})

        #extract first part to use as variable name of this lib
        string(REPLACE " " ";" list_dirname ${dirname})
        list(GET list_dirname 0 tmp_libname)
    
        #while the name is the same, add directory as a choice for current library.
        if(tmp_libname STREQUAL current_libname)
        
           list(APPEND ${current_libname}_property_string ${dirname})
      
        #change of libname
        else()
            set(current_libname ${tmp_libname})

            list(APPEND library_names ${current_libname})
        
            set(${current_libname}_property_string ${dirname})

        endif()

        #message(${dirname})
    endforeach()

    
    
    
    set(backup_names ${previously_detected_libs})
    
    foreach(libname ${previously_detected_libs})
        list(APPEND backup_values ${${libname}})
    endforeach()

    #message("backup names ${backup_names}")
    #message("backup values ${backup_values}")

    delete_previously_detected_libs()

    
    
    message(STATUS "Found folders for libraries '${library_names}'")

    #make sure the property strings of given lib variable are sorted and select the last one, with
    #the highest version number, by default.
    foreach(library ${library_names})

        #message("${library}")
        #message("${${library}_property_string}")

        #special version of sort which correctly pust version 1.3 before version 1.11
        version_sort("${library}_property_string")
        
        
        #current detected library had previously a value
        set(previous_value_is_experimental FALSE)
        list(FIND backup_names LIBRARY_${library} index)


        
        if(${index} GREATER "-1")   
            list(GET backup_values ${index} previous_value)
                 
            #check if the previously selected value is experimental
            string(FIND ${previous_value} "experimental" result)

            if(${result} GREATER "-1")
                set(previous_value_is_experimental TRUE)
            endif()       
        endif()
        
        
        list(FIND ${library}_property_string "${previous_value}" index2)
        
        set(missing_previous_value FALSE)
        
        if("${index2}" EQUAL "-1") 
            set(missing_previous_value TRUE)
        endif()
              
              
        #iterate from the highest version - revert the property list - until there are both highest regular
        #and experimental values found if present.
        set(tmp_property_list ${${library}_property_string})
        list(REVERSE tmp_property_list)
        
        foreach(item ${tmp_property_list})
        
            string(FIND ${item} "experimental" result)
               
            if(${result} GREATER "-1" AND NOT new_experimental_value)
                set(new_experimental_value ${item})
            elseif(${result} LESS "0" AND NOT new_regular_value)
                set(new_regular_value ${item})
            endif()
        
            if(new_experimental_value AND new_regular_value)
                break()
            endif()
            
        endforeach()
        
        
            
        #we prefer previously selected value
        if(${missing_previous_value} STREQUAL "FALSE" AND NOT "${previous_value}" STREQUAL "")
            set(final_value ${previous_value})
        else()
            set(final_value ${new_regular_value})           
        endif()
        
        #message("old ${previous_value}")
        #message("new regular ${new_regular_value}")
        #message("new experimental ${new_experimental_value}")

        
        #the message should appear only if 
        #   there is newer regular lib
        #   old is experimental and there is newer regular or experimental one
        
        #get version numbers from items of interest
        string(FIND "${previous_value}" " " _index)
        math(EXPR _result "${_index} + 1")
        string(SUBSTRING "${previous_value}" ${_result} -1 previous_value_version)
        
        string(FIND "${new_regular_value}" " " _index)
        math(EXPR _result "${_index} + 1")
        string(SUBSTRING "${new_regular_value}" ${_result} -1 new_regular_value_version)
        
        string(FIND "${new_experimental_value}" " " _index)
        math(EXPR _result "${_index} + 1")
        string(SUBSTRING "${new_experimental_value}" ${_result} -1 new_experimental_value_version)
             
       
        
        if(NOT "${previous_value}" STREQUAL "" AND NOT "${previous_value_is_experimental}" STREQUAL "TRUE")
        
            if(NOT "${previous_value}" STREQUAL "${new_regular_value}") ## regular - newer regular
                set(newer_libraries_alert TRUE PARENT_SCOPE)
            
                list(APPEND newer_libraries_list "${library}")
                #message("regular - newer regular")
            endif()
        
        elseif(NOT "${previous_value}" STREQUAL "")
        
            if("${previous_value_version}" VERSION_LESS "${new_regular_value_version}")      ## experimental - newer regular
                set(newer_libraries_alert TRUE PARENT_SCOPE)
            
                list(APPEND newer_libraries_list "experimental ${library}")
                #message("experimental - newer regular")

            elseif(NOT "${previous_value}" STREQUAL "${new_experimental_value}" AND "${previous_value_version}" VERSION_LESS "${new_experimental_value_version}")     ## experimental - newer experimental

                set(newer_libraries_alert TRUE PARENT_SCOPE)
            
                list(APPEND newer_libraries_list "${library}")
                #message("experimental - newer experimental")

            endif()

        endif()

        
        #No FORCE flag means that previous values are preserved..
        set(LIBRARY_${library} "${final_value}" CACHE STRING "This library has folder at the searched path.")
        set_property(CACHE LIBRARY_${library} PROPERTY STRINGS "${${library}_property_string}")
        mark_as_advanced(CLEAR LIBRARY_${library})
        unset(previous_value)
        unset(new_regular_value)
        unset(new_experimental_value)

        #build list of all the current created library variables
        list(APPEND detected_libs LIBRARY_${library})
    endforeach()

    
    set(newer_libraries_list "${newer_libraries_list}" PARENT_SCOPE)

    #and keep the list internally.. This lists' variables get unset before creation of new ones.
    set(previously_detected_libs ${detected_libs} CACHE INTERNAL "")



    #Set variables for all the detected libs
    foreach(library ${library_names})
    #	include_directories("${BUILD_PATH_TO_PREBUILT_LIBS}${library}/include")
    #	link_directories("${BUILD_PATH_TO_PREBUILT_LIBS}${library}/lib")

        ## Names are not always capitalized but CMake convention uses capitalized names
        # save some trouble and set both variants..
        set(${library}_FOUND TRUE CACHE INTERNAL "")

        string(TOUPPER ${library} capitalized_name)
        set(${capitalized_name}_FOUND TRUE CACHE INTERNAL "")

        set(${library}_INCLUDE_DIRS "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${library}}/include" CACHE INTERNAL "")
        set(${library}_LIBRARY_DIRS "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${library}}/lib" CACHE INTERNAL "")
        set(${library}_BINARY_DIRS  "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${library}}/bin" CACHE INTERNAL "")
        
        set(${library}_PATH "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${library}}" CACHE INTERNAL "")
        
        #message("${${library}_INCLUDE_DIRS}")
    
        #unused or dynamic versions of lib files can be put into, for example, 'Unused' folder
        FILELIST(tmp ${${library}_LIBRARY_DIRS})
                
        #message(STATUS "All ${tmp}")

        #filter out non .lib files
        foreach(filename ${tmp})
            string(REGEX MATCH "^.*\\.lib$" result ${filename})
    
            if(NOT result STREQUAL "")
                list(APPEND ${library}_all_LIBRARIES ${filename})
            endif()
            
            set(result "")
                
        endforeach()
        
        
        #message(STATUS "All filtered ${${library}_all_LIBRARIES}")

        list(LENGTH ${library}_all_LIBRARIES length)
        if(${length} GREATER "0")
            list(SORT ${library}_all_LIBRARIES) # to make sure they are in sequence
        endif()
        
        foreach(filename ${${library}_all_LIBRARIES})
            #message(${filename})
            string(REGEX MATCH "^.*d\\.lib$" result ${filename})
                
            if(result STREQUAL "")
                list(APPEND ${library}_optimized_LIBRARIES ${filename})
            else()			
                list(APPEND ${library}_debug_LIBRARIES ${filename})
            endif()
        
            #workaround for edge case with libs ending with 'd' and debug versions with 'dd'
            #upon detecting XXXdd.lib the coresponding XXXd.lib must be already in optimized_LIBRARIES
            #so take it from there and place it into correct list.
            string(REGEX REPLACE "^(.*d)d\\.lib$" "\\1" dd_result ${filename})
                #message("${filename} ${dd_result}")

            if(NOT dd_result STREQUAL "" AND NOT ${dd_result} STREQUAL ${filename})
                #message("${filename} ${dd_result}")
                list(REMOVE_ITEM ${library}_debug_LIBRARIES "${dd_result}.lib")
                list(APPEND ${library}_optimized_LIBRARIES "${dd_result}.lib")
            endif()
            
            set(dd_result "")
            set(result "")
            
        endforeach()

        #message("optimized ${${library}_optimized_LIBRARIES}")
        #message(STATUS "debug ${${library}_debug_LIBRARIES}")
    

        #construct the final library list complete with debug/optimized/general keywords before each item
        foreach(filename ${${library}_optimized_LIBRARIES})
    
            #this presumes that having something in debug but not release version doesn't make sense but it does the other way around
            #so.. if optimized does not have debug, give it general keyword. If the debug version is present give each 
            # debug/optimized keyword respectively.
            has_debug_version(result debug_name ${filename} ${library}_debug_LIBRARIES)
        
            if(${result} STREQUAL "FALSE")
                list(APPEND ${library}_LIBRARIES "general" ${filename})		#all configurations
            else()
                list(APPEND ${library}_LIBRARIES "optimized" ${filename})	#all except debug
                list(APPEND ${library}_LIBRARIES "debug" ${debug_name})		#only for debug
            endif()
        
            set(result "")
            set(debug_name "")

        endforeach()
    
        #propagate value to parent scope
        set(${library}_LIBRARIES ${${library}_LIBRARIES} PARENT_SCOPE)
            
        #message(STATUS "${library}: ${${library}_LIBRARIES}")

    endforeach()

endfunction()

## HMMM... do the lists without the file extensions?








