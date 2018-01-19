

#note scan libs should be included only after this file..

#Values for constructing derived variables pertaining to this library.. These strings must be the same as 
# the first word in the name of a folder in which given library was placed.
set(QT_LIB_NAME			"Qt")
set(OSG_LIB_NAME		"OSG")
set(GLU_LIB_NAME		"GLU")
set(VPL_LIB_NAME		"VPL")
set(VTK_LIB_NAME		"VTK")
set(PNG_LIB_NAME		"Libpng")
set(XML_LIB_NAME		"Libxml")
set(GLEW_LIB_NAME		"GLEW")
set(GDCM_LIB_NAME		"GDCM")
set(GLAD_LIB_NAME		"glad")
set(GLFW_LIB_NAME		"GLFW")
set(CURL_LIB_NAME		"CURL")
set(ZLIB_LIB_NAME		"Zlib")
set(HDF5_LIB_NAME		"HDF5")
set(XLAB_LIB_NAME		"Xlab")
set(FLANN_LIB_NAME		"Flann")
set(DCMTK_LIB_NAME		"DCMTK")
set(ICONV_LIB_NAME		"Iconv")
set(EIGEN_LIB_NAME		"Eigen")
set(BOOST_LIB_NAME		"Boost")
set(CARES_LIB_NAME		"Cares")
set(BULLET_LIB_NAME		"Bullet")
set(OPENCV_LIB_NAME		"OpenCV")
set(OPENGL_LIB_NAME		"OpenGL") 
set(PHYSFS_LIB_NAME		"PhysFS")
set(OPENSSL_LIB_NAME	"OpenSSL")
set(OPENCTM_LIB_NAME	"OpenCTM")
set(OPENMESH_LIB_NAME	"OpenMesh")
set(STATISMO_LIB_NAME	"Statismo")
set(FREEGLUT_LIB_NAME   "Freeglut")
set(LEAPMOTION_LIB_NAME "LeapMotion")
set(PYTHONCPP_LIB_NAME  "PythonCppInterop")
set(PYTHON_LIB_NAME     "PythonLibs")




#clear the list
unset(all_library_names)

#for finds mostly.. In scan libs all the regular and capitalized names are set to ${NAME}_FOUND FALSE
list(APPEND all_library_names ${QT_LIB_NAME})
list(APPEND all_library_names ${OSG_LIB_NAME})
list(APPEND all_library_names ${GLU_LIB_NAME})
list(APPEND all_library_names ${VPL_LIB_NAME})
list(APPEND all_library_names ${VTK_LIB_NAME})
list(APPEND all_library_names ${PNG_LIB_NAME})
list(APPEND all_library_names ${XML_LIB_NAME})
list(APPEND all_library_names ${GLEW_LIB_NAME})
list(APPEND all_library_names ${GDCM_LIB_NAME})
list(APPEND all_library_names ${GLAD_LIB_NAME})
list(APPEND all_library_names ${GLFW_LIB_NAME})
list(APPEND all_library_names ${CURL_LIB_NAME})
list(APPEND all_library_names ${ZLIB_LIB_NAME})
list(APPEND all_library_names ${HDF5_LIB_NAME})
list(APPEND all_library_names ${XLAB_LIB_NAME})
list(APPEND all_library_names ${FLANN_LIB_NAME})
list(APPEND all_library_names ${DCMTK_LIB_NAME})
list(APPEND all_library_names ${ICONV_LIB_NAME})
list(APPEND all_library_names ${EIGEN_LIB_NAME})
list(APPEND all_library_names ${BOOST_LIB_NAME})
list(APPEND all_library_names ${CARES_LIB_NAME})
list(APPEND all_library_names ${BULLET_LIB_NAME})
list(APPEND all_library_names ${OPENCV_LIB_NAME})
list(APPEND all_library_names ${OPENGL_LIB_NAME})
list(APPEND all_library_names ${PHYSFS_LIB_NAME})
list(APPEND all_library_names ${OPENSSL_LIB_NAME})
list(APPEND all_library_names ${OPENCTM_LIB_NAME})
list(APPEND all_library_names ${OPENMESH_LIB_NAME})
list(APPEND all_library_names ${STATISMO_LIB_NAME})
list(APPEND all_library_names ${FREEGLUT_LIB_NAME})
list(APPEND all_library_names ${LEAPMOTION_LIB_NAME})
list(APPEND all_library_names ${PYTHONCPP_LIB_NAME})
list(APPEND all_library_names ${PYTHON_LIB_NAME})



macro(delete_previous_info_variables)
    foreach(variable_name ${info_variable_list})
        unset(${variable_name} CACHE)	
    endforeach()
endmacro()


#removes top of TRIDIM_CURRENT_TARGET_STACK and updates TRIDIM_CURRENT_TARGET to the new value
macro(pop_target_stack)

    list( LENGTH TRIDIM_CURRENT_TARGET_STACK length)
    
    math(EXPR index "${length} - 1")
    math(EXPR new_index "${length} - 2")

    
    if(${length} GREATER "0")
        list(REMOVE_AT TRIDIM_CURRENT_TARGET_STACK ${index})
        
        if(${new_index} GREATER "-1")
            list(GET TRIDIM_CURRENT_TARGET_STACK ${new_index} TRIDIM_CURRENT_TARGET)
            message("Target changed back to ${TRIDIM_CURRENT_TARGET}.")

        else()
             set(TRIDIM_CURRENT_TARGET "")
        endif()
        
    else()
        set(TRIDIM_CURRENT_TARGET "")
        
        #check.. this shouldn't appear
        message(FATAL_ERROR "Popping empty target stack. Something is most definitely wrong.")
    endif()

endmacro()

#adds target_name to the top of TRIDIM_CURRENT_TARGET_STACK and updates TRIDIM_CURRENT_TARGET to this value
macro(push_target_stack target_name)

    list(APPEND TRIDIM_CURRENT_TARGET_STACK ${target_name})
    set( TRIDIM_CURRENT_TARGET ${target_name} )

    message("Target changed to ${TRIDIM_CURRENT_TARGET}.")

endmacro()


#generic macro for adding include/link directories.
#Because all of this is already available under variables derived from library name, the only thing needed is that name.
macro(ADD_LIB LIBNAME)
    
    set(arguments ${ARGN})
    list(LENGTH arguments num_extra_args)
      
    
    if("${num_extra_args}" GREATER "0")
        message(STATUS "Adding ${LIBNAME} to target ${TRIDIM_CURRENT_TARGET}. ${arguments} components only. TODO")
    else()
        message(STATUS "Adding ${LIBNAME} to target ${TRIDIM_CURRENT_TARGET}.")
    endif()
    
    string(TOUPPER ${LIBNAME} capitalized_libname)
  
    #add definitions indicating used libs for testing in precompiled headers
    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE USED_LIB_${capitalized_libname})
    
    #All without this property are hidden by marking them as advanced.
    #It is a property because one scope level up (with PARENT_SCOPE) doesn't have to be enough.
    SET_PROPERTY(GLOBAL PROPERTY USED_LIB_${capitalized_libname} "TRUE")

    #undefined libname_found means it is not in folder..
    if(NOT ${LIBNAME}_FOUND OR ${${LIBNAME}_FOUND} STREQUAL "FALSE")
        
        
        #scan libs didn't find this lib or platform is not windows.. either way try find
        message(STATUS "Looking for library ${LIBNAME}.")

        find_package(${LIBNAME})
        
        if(${LIBNAME}_FOUND)
            message("Success.")
        else()
            message(FATAL_ERROR "${LIBNAME} was not found. Check the library folder.")
        endif()      
        
    endif()
       
    #check for empty lists before adding include/link directories..
    if(NOT "${${LIBNAME}_INCLUDE_DIRS}" STREQUAL "")
        #message(STATUS "Include dirs ${${LIBNAME}_INCLUDE_DIRS}")
        target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${${LIBNAME}_INCLUDE_DIRS})
        
        set( INFO_INCLUDE-DIRS_${LIBNAME} ${${LIBNAME}_INCLUDE_DIRS} CACHE STRING "${LIBNAME} include dirs." FORCE)
        
        list(APPEND info_variable_list INFO_INCLUDE-DIRS_${LIBNAME})
    endif()
    
    
    #
    if(NOT ${${LIBNAME}_LIBRARY_DIRS} STREQUAL "")	
        #message(STATUS "Library dirs ${${LIBNAME}_LIBRARY_DIRS}")
        #link_directories( ${${LIBNAME}_LIBRARY_DIRS} ) #get rid of this and give absolute paths to libs..
        
        set( INFO_LIBRARY-LIST_${LIBNAME} ${${LIBNAME}_LIBRARIES} CACHE STRING "${LIBNAME} libraries."  FORCE)
        set( INFO_LIBRARY-DIRS_${LIBNAME} ${${LIBNAME}_LIBRARY_DIRS} CACHE STRING "${LIBNAME} library dirs."  FORCE)
        mark_as_advanced(CLEAR INFO_LIBRARY-LIST_${LIBNAME} INFO_LIBRARY-DIRS_${LIBNAME})
        
        #string(TOUPPER ${LIBNAME} upper_libname)
        #set(${upper_libname}_LINKED_LIBS ${INFO_LIBRARY-LIST_${LIBNAME}})	#so as no modification of target link libraries is necessary
        
        list(APPEND info_variable_list INFO_LIBRARY-LIST_${LIBNAME})
        list(APPEND info_variable_list INFO_LIBRARY-DIRS_${LIBNAME})
        
        #go through the libs, extract keyword, prepend path and link it to current target
        foreach(library ${${LIBNAME}_LIBRARIES})
            
            if(${library} STREQUAL "debug" OR ${library} STREQUAL "optimized" OR ${library} STREQUAL "general")
                set(keyword ${library})
                continue()
            endif()
        
            target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "${keyword}" "${${LIBNAME}_LIBRARY_DIRS}/${library}")
        endforeach()
        
        message(STATUS "Linked libraries at ${${LIBNAME}_LIBRARY_DIRS}: ${${LIBNAME}_LIBRARIES}")

    endif()

    
endmacro()



macro(ADD_LIB_GDCM)

    
    ADD_LIB(${GDCM_LIB_NAME} ${ARGV})

    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE TRIDIM_USE_GDCM )

endmacro()


macro(ADD_LIB_OFICIALPYTHON)

    FIND_PACKAGE(PythonLibs)
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
    target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "${PYTHON_LIBRARY}")
    set( PYTHON_DEBUG_LIBRARY "" CACHE STRING "Python debug library." FORCE )
    
endmacro()

macro(ADD_LIB_PYTHONCPP)

    ADD_LIB(${PYTHONCPP_LIB_NAME} ${ARGV}) 
    
endmacro()

macro(ADD_LIB_PYTHON)
    
    ADD_LIB(${PYTHON_LIB_NAME} ${ARGV})
    
endmacro()

macro(ADD_LIB_OPENSSL)
    
    ADD_LIB(${OPENSSL_LIB_NAME} ${ARGV})

    #this define prevents using of md5.h from activation folder and includes openSSL's one instead
    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE HAVE_OPENSSL)

    
endmacro()

macro(ADD_LIB_OPENCTM)
    
    ADD_LIB(${OPENCTM_LIB_NAME} ${ARGV})
    
endmacro()

macro(ADD_LIB_BULLET)
    
    ADD_LIB(${BULLET_LIB_NAME} ${ARGV})
    
endmacro()

# Find Eigen library
macro(ADD_LIB_EIGEN)


    ADD_LIB(${EIGEN_LIB_NAME} ${ARGV})

    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE EIGEN_DONT_PARALLELIZE EIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS)
    

    
    ###Cant this make problems in other projects? - leave it here or put it only in projects that want it?
    
    ##SSE and SSE2 are available in MSVC only for x86 builds. SSE3,4 etc only for Intel compiler..
    #not sure of the utility or reason this is here..
    if(CMAKE_COMPILER_IS_GNUCXX)
    
        option( EIGEN_ENABLE_SSE2 "Enable/Disable SSE2 in the Eigen library" OFF )
    
        if( EIGEN_ENABLE_SSE2 )
            target_compile_options( ${TRIDIM_CURRENT_TARGET} PRIVATE -msse2)
        endif()

        
        option( EIGEN_ENABLE_SSE3 "Enable/Disable SSE3 in the Eigen library" OFF )
    
        if( EIGEN_ENABLE_SSE3 )
            target_compile_options( ${TRIDIM_CURRENT_TARGET} PRIVATE -msse3)
        endif()

    endif()

endmacro()

# Find the VPL library.
macro( ADD_LIB_VPL )
  
    ADD_LIB(${VPL_LIB_NAME} ${ARGV})

    target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE MDS_LIBRARY_STATIC )

    if(DARWIN)
        target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE _MACOSX )
    elseif(UNIX)
        target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE _LINUX )
    elseif(WIN32)
        target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE _WIN32 )
    endif()

endmacro()


# Find the OpenMesh library.
macro( ADD_LIB_OPENMESH )

    ADD_LIB(${OPENMESH_LIB_NAME} ${ARGV})
    
    target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE NOMINMAX )
    target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE _USE_MATH_DEFINES )

endmacro()


# Find LeapMotion
macro( ADD_LIB_LEAPMOTION )

    ADD_LIB(${LEAPMOTION_LIB_NAME} ${ARGV})

endmacro()


# Find OSG library
macro( ADD_LIB_OSG )

    ADD_LIB(${OSG_LIB_NAME} ${ARGV})

endmacro()


##pokud se to nestane, tak smazat
macro( ADD_LIB_GLU )

message(FATAL_ERROR "NECO CHCE PRIDAT GLU... !!!!!!!!!!!!!!!!!!!!!!!!!")

    ADD_LIB(${GLU_LIB_NAME} ${ARGV})

endmacro()


# This module sets the following variables:
#
# ``OPENGL_FOUND``
#  True, if the system has OpenGL.
# ``OPENGL_XMESA_FOUND``
#  True, if the system has XMESA.
# ``OPENGL_GLU_FOUND``
#  True, if the system has GLU.
# ``OPENGL_INCLUDE_DIR``
#  Path to the OpenGL include directory.
# ``OPENGL_LIBRARIES``
#  Paths to the OpenGL and GLU libraries.
#
# If you want to use just GL you can use these values:
#
# ``OPENGL_gl_LIBRARY``
#  Path to the OpenGL library.
# ``OPENGL_glu_LIBRARY``
#  Path to the GLU library.


############################################################
###tady mozna ten find package nechat.. to byva v systemovych slozkach..
macro( ADD_LIB_OPENGL )   
    
    message(STATUS "Adding ${OPENGL_LIB_NAME} to project ${TRIDIM_CURRENT_TARGET}.")

    if(NOT OPENGL_FOUND)
        find_package (opengl REQUIRED)
    endif()

    if( OPENGL_FOUND )

        target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${OPENGL_INCLUDE_DIR} )
        

     # SET( OPENGL_LINKED_LIBS ${OPENGL_LIBRARY} CACHE STRING "OPENGL library." )


        target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "general" "${OPENGL_glu_LIBRARY}")
        target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "general" "${OPENGL_gl_LIBRARY}")


        set(${OPENGL_LIB_NAME}_LIBRARIES ${OPENGL_LIBRARIES})
        set(${OPENGL_LIB_NAME}_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})


        get_filename_component( ${OPENGL_LIB_NAME}_LIBRARY_DIRS ${OPENGL_gl_LIBRARY} PATH )


        set( INFO_INCLUDE-DIRS_${OPENGL_LIB_NAME} ${${OPENGL_LIB_NAME}_INCLUDE_DIRS} CACHE STRING "${OPENGL_LIB_NAME} include dirs." FORCE)

        set( INFO_LIBRARY-LIST_${OPENGL_LIB_NAME} ${${OPENGL_LIB_NAME}_LIBRARIES} CACHE STRING "${OPENGL_LIB_NAME} libraries."  FORCE)
        set( INFO_LIBRARY-DIRS_${OPENGL_LIB_NAME} ${${OPENGL_LIB_NAME}_LIBRARY_DIRS} CACHE STRING "${OPENGL_LIB_NAME} library dirs."  FORCE)

        #string(TOUPPER ${LIBNAME} upper_libname)
        #set(${upper_libname}_LINKED_LIBS ${INFO_LIBRARY-LIST_${LIBNAME}})	#so as no modification of target link libraries is necessary
        list(APPEND info_variable_list INFO_INCLUDE-DIRS_${OPENGL_LIB_NAME})

        list(APPEND info_variable_list INFO_LIBRARY-LIST_${OPENGL_LIB_NAME})
        list(APPEND info_variable_list INFO_LIBRARY-DIRS_${OPENGL_LIB_NAME})

    ELSE()
      MESSAGE( FATAL_ERROR "OPENGL library was not found! Please, set the cache entry OPENGL_ROOT_DIR!"  )
    ENDIF()
  

    #	set(${OPENGL_LIB_NAME}_FOUND TRUE)
    #	set(${OPENGL_LIB_NAME}_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    #	set(${OPENGL_LIB_NAME}_LIBRARY_DIRS ${OPENGL_LIBRARIES})
    #	set(${OPENGL_LIB_NAME}_LIBRARIES ${OPENGL_LIBRARY})



endmacro()


# Find glew 
macro( ADD_LIB_GLEW )

    ADD_LIB(${GLEW_LIB_NAME} ${ARGV})

endmacro()


# Find glfw 
macro( ADD_LIB_GLFW )

message(FATAL_ERROR "NECO CHCE PRIDAT GLFW... !!!!!!!!!!!!!!!!!!!!!!!!!")

    ADD_LIB(${GLFW_LIB_NAME} ${ARGV})

endmacro()


# Find curl 
macro( ADD_LIB_CURL )

    ADD_LIB(${CURL_LIB_NAME} ${ARGV})

endmacro()


# Find zlib
macro( ADD_LIB_ZLIB )

    ADD_LIB(${ZLIB_LIB_NAME} ${ARGV})

endmacro()

# Find png
macro( ADD_LIB_PNG )

    ADD_LIB(${PNG_LIB_NAME} ${ARGV})

endmacro()

# Find iconv
macro( ADD_LIB_ICONV )

    ADD_LIB(${ICONV_LIB_NAME} ${ARGV})

endmacro()


# Find PhysicsFS
macro( ADD_LIB_PHYSFS )

    ADD_LIB(${PHYSFS_LIB_NAME} ${ARGV})

endmacro( ADD_LIB_PHYSFS )

# Find Geos
macro( ADD_LIB_GEOS )

    message(FATAL_ERROR "NECO CHCE PRIDAT GEOS... !!!!!!!!!!!!!!!!!!!!!!!!!")


  if( NOT ADD_LIB_GEOS_INCLUDED )
    set( ADD_LIB_GEOS_INCLUDED TRUE )

    FIND_PACKAGE( OurGEOS REQUIRED )
    IF( GEOS_FOUND )
      INCLUDE_DIRECTORIES(${GEOS_INCLUDE_DIR})
      LINK_DIRECTORIES( ${GEOS_LIB} )
      SET( GEOS_LINKED_LIBS ${GEOS_LIBRARY} CACHE STRING "GEOS library." )

    ELSE( GEOS_FOUND )
      MESSAGE( FATAL_ERROR "GEOS library was not found! Please, set the cache entry GEOS_ROOT_DIR!" )
    ENDIF( GEOS_FOUND )

  endif( NOT ADD_LIB_GEOS_INCLUDED )
endmacro( ADD_LIB_GEOS )



# Find VTK library
macro( ADD_LIB_VTK )

    #should contain only one folder..
    #this is done because there is version in the folder name too..    
    SUBDIRLIST(vtk_config_folder "${${VTK_LIB_NAME}_PATH}/lib/cmake/")
    
    #this enables cmake to find VTKConfig.cmake
    set(VTK_DIR ${${VTK_LIB_NAME}_PATH}/lib/cmake/${vtk_config_folder} CACHE INTERNAL "")
    
    #correct include path..
    #ScanLibs detects only the include folder, but VTK sources and headers count on include path
    #with the subfolder tacked on. find_package sets it right, but another Configure reverts it.. so check for it and restore the path that works.
    
    #cut the last step from path
    get_filename_component(result "${VTK_INCLUDE_DIRS}" DIRECTORY)
    
    #if it is not the same as default include path there is something wrong
    if(NOT "${result}" STREQUAL "${${VTK_LIB_NAME}_PATH}/include")
    
        SUBDIRLIST(vtk_include_folder "${${VTK_LIB_NAME}_PATH}/include/")
        
        #so fix it..
        set(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS}/${vtk_include_folder} CACHE INTERNAL "")
    endif()
       
    mark_as_advanced(VTK_DIR)

    #instead of boolean flag save the path.. this allows finding this package again if the location changes.
    if(NOT "${VTK_path_check}" STREQUAL "${${VTK_LIB_NAME}_PATH}")
    
        set(VTK_path_check "${${VTK_LIB_NAME}_PATH}" CACHE INTERNAL "")        
    
        set(backup ${VTK_LIBRARY_DIRS})
        set(libraries_backup ${VTK_LIBRARIES})
        
        #this clears the VTK_LIBRARY_DIRS variable so save it beforehand..   
        find_package(VTK)

        set(VTK_LIBRARY_DIRS ${backup})
        set(VTK_LIBRARIES ${libraries_backup})

        #file(GLOB vtk_include_folder RELATIVE "${${VTK_LIB_NAME}_PATH}/include/" "${${VTK_LIB_NAME}_PATH}/include/*")

    endif()
        
    #this block is copied and modified from UseVtk.cmake which should replace need to do include(${VTK_USE_FILE})  
    #It is usually placed at X:\msvc14\x64\VTK 8.0\lib\cmake\vtk-8.0 or similar

    # Update CMAKE_MODULE_PATH so includes work.
    list(APPEND CMAKE_MODULE_PATH ${VTK_CMAKE_DIR})   
    #;-list of directories specifying a search path for CMake modules to be loaded by the the include() or find_package()
    #commands before checking the default modules that come with CMake. By default it is empty, it is intended to be set by the project.

       
    #NOTE: these are all empty so it is hard to test if I'm doing it right
    #message("${VTK_REQUIRED_C_FLAGS}")
    #message("${VTK_REQUIRED_CXX_FLAGS}")
    #message("${VTK_REQUIRED_EXE_LINKER_FLAGS}")
    #message("${VTK_REQUIRED_SHARED_LINKER_FLAGS}")
    #message("${VTK_REQUIRED_MODULE_LINKER_FLAGS}")

    # Add compiler flags needed to use VTK.       
    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE "${VTK_REQUIRED_CXX_FLAGS}")
        
    #not sure if it is strictly equivalent so just leave it as it is for now
    #target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "${VTK_REQUIRED_EXE_LINKER_FLAGS}")
    #target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "${VTK_REQUIRED_SHARED_LINKER_FLAGS}")
    #target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE "${VTK_REQUIRED_MODULE_LINKER_FLAGS}")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VTK_REQUIRED_EXE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${VTK_REQUIRED_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${VTK_REQUIRED_MODULE_LINKER_FLAGS}")

    # Add preprocessor definitions needed to use VTK.
    target_compile_definitions( ${TRIDIM_CURRENT_TARGET} PRIVATE ${VTK_DEFINITIONS} )
    
    ADD_LIB(${VTK_LIB_NAME} ${ARGV})

endmacro()

# Find Carve - beware that it is GPL and therefore can't be used in all apps
macro( ADD_LIB_CARVE )

    message(FATAL_ERROR "NECO CHCE PRIDAT CARVE... !!!!!!!!!!!!!!!!!!!!!!!!!")

  if( NOT ADD_LIB_CARVE_INCLUDED )
    set( ADD_LIB_CARVE_INCLUDED TRUE )

    FIND_PACKAGE( OurCarve REQUIRED )
    IF( CARVE_FOUND )
      INCLUDE_DIRECTORIES(${CARVE_INCLUDE_DIR})
      LINK_DIRECTORIES( ${CARVE_LIB} )
      SET( CARVE_LINKED_LIBS ${CARVE_LIBRARY} CACHE STRING "Carve library." )
    ELSE( CARVE_FOUND )
      MESSAGE( FATAL_ERROR "Carve library was not found! Please, set the cache entry CARVE_ROOT_DIR!" )
    ENDIF( CARVE_FOUND )

  endif( NOT ADD_LIB_CARVE_INCLUDED )
endmacro( ADD_LIB_CARVE )

# Find glad
macro( ADD_LIB_GLAD )

    ADD_LIB(${GLAD_LIB_NAME} ${ARGV})

endmacro()

# Find Cares
macro( ADD_LIB_CARES )

    ADD_LIB(${CARES_LIB_NAME} ${ARGV})

endmacro()

# Find Libxml
macro( ADD_LIB_XML )

    ADD_LIB(${XML_LIB_NAME} ${ARGV})

endmacro()

# Find Freeglut
macro( ADD_LIB_FREEGLUT )

    ADD_LIB(${FREEGLUT_LIB_NAME} ${ARGV})

endmacro()

# Find HDF5 library
macro( ADD_LIB_HDF5 )
    message(FATAL_ERROR "NECO CHCE PRIDAT HDF5... !!!!!!!!!!!!!!!!!!!!!!!!!")

    ADD_LIB(${HDF5_LIB_NAME} ${ARGV})

endmacro()

# Find Statismo library
macro( ADD_LIB_STATISMO )
 
    message(FATAL_ERROR "NECO CHCE PRIDAT HDF5... !!!!!!!!!!!!!!!!!!!!!!!!!")

    ADD_LIB(${STATISMO_LIB_NAME} ${ARGV})

endmacro()

# Find the Boost library
macro( ADD_LIB_BOOST )

    message(FATAL_ERROR "NECO CHCE PRIDAT BOOST... !!!!!!!!!!!!!!!!!!!!!!!!!")

    ADD_LIB(${BOOST_LIB_NAME} ${ARGV})

endmacro()

# Find NanoFLANN library
macro( ADD_LIB_NANOFLANN )

    message(FATAL_ERROR "NECO CHCE PRIDAT NANOFLANN... !!!!!!!!!!!!!!!!!!!!!!!!!")

    set(NANOFLANN_LIB_NAME "NanoFlann")

    ADD_LIB(${NANOFLANN_LIB_NAME} ${ARGV})

endmacro()

macro( ADD_LIB_FLANN )

    ADD_LIB(${FLANN_LIB_NAME} ${ARGV})

endmacro()

macro( ADD_LIB_OPENCV )
    
    ADD_LIB(${OPENCV_LIB_NAME} ${ARGV})

    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE OPENCV_AVAILABLE )      

endmacro()

###############################################


#libs are present from scan, just find packages and add their include folders to the new convention variable
macro ( ADD_LIB_QT )      

    if(UNIX)
        set(QT_QMAKE_EXECUTABLE "${LIBRARY_PATHS_Qt_install_path}/bin/qmake")
        set(QT_LIBRARY_DIRS "${LIBRARY_PATHS_Qt_install_path}/lib")
        set(Qt_LIBRARY_DIRS "${LIBRARY_PATHS_Qt_install_path}/lib")

        get_filename_component(tmp ${QT_QMAKE_EXECUTABLE} PATH)
        set(QT_BINARY_DIRS "${tmp}")
        set(Qt_BINARY_DIRS "${tmp}")

    else()
        set(QT_QMAKE_EXECUTABLE "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${QT_LIB_NAME}}/bin/qmake.exe")
    endif()

           
    message(STATUS "Adding ${QT_LIB_NAME}5 to target ${TRIDIM_CURRENT_TARGET}.")
      
      
    set(tmp ${ARGN})
    list(LENGTH tmp list_length)
      
    if("${list_length}" GREATER "0") 
        set(TRIDIM_QT_COMPONENTS "${ARGN}")
    else()
    
        if (QT_BASIC)	#and replace this with parameters to ADD_LIB_QT()
            set(TRIDIM_QT_COMPONENTS Core Gui Network Widgets PrintSupport OpenGL LinguistTools Sql Script WebView)
        else()
            set(TRIDIM_QT_COMPONENTS Core Gui Network Widgets PrintSupport WebEngine WebEngineCore WebEngineWidgets WebView OpenGL LinguistTools Sql Script )
        endif()
        
    endif()
    
    
    ADD_LIB_QT5("${TRIDIM_QT_COMPONENTS}")
            

    #message(${QT_DEFINITIONS})
    add_definitions(${QT_DEFINITIONS})
        
    #MESSAGE(${QT_DEFINITIONS})
    #message(STATUS "Include dirs ${${QT_LIB_NAME}_INCLUDE_DIRS}")		
    #message(STATUS "Library dirs ${${QT_LIB_NAME}_LIBRARY_DIRS}")
    
    string(REPLACE ";" "|" TRIDIM_QT_COMPONENT_EXPRESSION "${TRIDIM_QT_COMPONENTS}")


    #add definitions indicating used libs for testing in precompiled headers
    target_compile_definitions(${TRIDIM_CURRENT_TARGET} PRIVATE USED_LIB_QT)
    SET_PROPERTY(GLOBAL PROPERTY USED_LIB_QT "TRUE")

    #message(${${QT_LIB_NAME}_INCLUDE_DIRS})
    
    list(REMOVE_DUPLICATES ${QT_LIB_NAME}_INCLUDE_DIRS)

    #message(STATUS "Include dirs ${${QT_LIB_NAME}_INCLUDE_DIRS}")
    target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${${QT_LIB_NAME}_INCLUDE_DIRS})

    set( INFO_INCLUDE-DIRS_${QT_LIB_NAME} ${${QT_LIB_NAME}_INCLUDE_DIRS} CACHE STRING "${QT_LIB_NAME} include dirs." FORCE)

    list(APPEND info_variable_list INFO_INCLUDE-DIRS_${QT_LIB_NAME})


    if(NOT ${QT_LIB_NAME}_LIBRARY_DIRS STREQUAL "")
                 
        #info variables
        set( INFO_LIBRARY-LIST_${QT_LIB_NAME} ${${QT_LIB_NAME}_LIBRARIES} CACHE STRING "${QT_LIB_NAME} libraries."  FORCE)
        set( INFO_LIBRARY-DIRS_${QT_LIB_NAME} ${${QT_LIB_NAME}_LIBRARY_DIRS} CACHE STRING "${QT_LIB_NAME} library dirs."  FORCE)
        set( INFO_LIBRARY-LIST_${QT_LIB_NAME}_qmake ${QT_QMAKE_EXECUTABLE} CACHE STRING "Location of qmake.exe."  FORCE)
        mark_as_advanced(CLEAR INFO_LIBRARY-LIST_Qt INFO_LIBRARY-DIRS_Qt INFO_LIBRARY-LIST_${QT_LIB_NAME}_qmake)


        #string(TOUPPER ${QT_LIB_NAME} upper_libname)
        #set(${upper_libname}_LINKED_LIBS ${INFO_LIBRARY-LIST_${QT_LIB_NAME}})
        
        list(APPEND info_variable_list INFO_LIBRARY-LIST_${QT_LIB_NAME})
        list(APPEND info_variable_list INFO_LIBRARY-DIRS_${QT_LIB_NAME})
        list(APPEND info_variable_list INFO_LIBRARY-LIST_${QT_LIB_NAME}_qmake)

        #and finally link them.. + must extract keyword from foreach..
        foreach(library ${${QT_LIB_NAME}_LIBRARIES})
            
            if(${library} STREQUAL "debug" OR ${library} STREQUAL "optimized" OR ${library} STREQUAL "general")
                set(keyword ${library})
                continue()
            endif()

           target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE ${library})
           
        endforeach()
        
        message(STATUS "Linked libraries at ${${QT_LIB_NAME}_LIBRARY_DIRS}: ${${QT_LIB_NAME}_LIBRARIES}")
        
    endif()
    
endmacro ()


# Find QT5 library
macro( ADD_LIB_QT5 components_arg)
    
    if(UNIX)
        set(QT_MODULES_PATH "${LIBRARY_PATHS_Qt_install_path}/lib/cmake/")
    else()
        set(QT_MODULES_PATH "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${QT_LIB_NAME}}/lib/cmake")
        #message(STATUS "${BUILD_PATH_TO_PREBUILT_LIBS}/${LIBRARY_${QT_LIB_NAME}}/lib/cmake was added to CMAKE_MODULE_PATH")
    endif()

    #list(APPEND CMAKE_MODULE_PATH ${QT_MODULES_PATH})
    list(APPEND CMAKE_PREFIX_PATH "${QT_MODULES_PATH}")

    #Qt puts component_DIR variables into CACHE which we must now clear, because switching versions would not work well
    get_filename_component(path "${Qt5Core_DIR}" PATH)

    SUBDIRLIST(folder_list "${QT_MODULES_PATH}")

    if(NOT "${path}" STREQUAL "${QT_MODULES_PATH}")
        message("Cleaning QT cache stuff..")

        foreach(folder ${folder_list})
            unset("${folder}_DIR" CACHE)
        endforeach()
        
    endif()
    
    foreach(folder ${folder_list})
        #add all the modules into cmake search paths
        list(APPEND CMAKE_PREFIX_PATH "${QT_MODULES_PATH}/${folder}")   # CMAKE_PREFIX_PATH is not cached so fill it everytime configure is run..
    endforeach()

            
    set(${QT_LIB_NAME}_INCLUDE_DIRS "")
    set(${QT_LIB_NAME}_LIBRARIES "")
    
    foreach(component ${components_arg})

        #message("Qt5 component ${component}")
        

        if(Qt5${component}_FOUND)

            list(APPEND ${QT_LIB_NAME}_INCLUDE_DIRS ${Qt5${component}_INCLUDE_DIRS})
            list(APPEND ${QT_LIB_NAME}_LIBRARIES ${Qt5${component}_LIBRARIES})

            list(APPEND QT_DEFINITIONS ${Qt5${component}_DEFINITIONS})

        else()

            find_package(Qt5${component} REQUIRED)

            if(NOT Qt5${component}_FOUND)
                message(WARNING "${Qt5${component}} not found")
            endif()

            mark_as_advanced(Qt5${component}_DIR)

            #don't know why, but it puts .//mkspecs/something into component includes
            #Here I fix it to a valid path..
            set(tmp_list "")
            foreach(path ${Qt5${component}_INCLUDE_DIRS})		#maybe take a minute to check which one actually does the work..
                string(REPLACE ".//" "" fixed_path1 ${path})
                string(REPLACE "/./" "/" fixed_path2 ${fixed_path1})
                string(REPLACE "\\.\\" "\\" fixed_path3 ${fixed_path2})
                string(REPLACE ".\\\\" "" fixed_path ${fixed_path3})

                list(APPEND tmp_list ${fixed_path})


                string(FIND ${fixed_path} "mkspecs" result)

                if(${result} GREATER "-1")
                    set(QT_MKSPECS_DIR ${fixed_path})
                endif()

            endforeach()

            set(Qt5${component}_INCLUDE_DIRS ${tmp_list})


            #message("libraries ${Qt5${component}_LIBRARIES}")
            #message("linked libs ${Qt5${component}_LINKED_LIBS}")
            #message("definitions ${Qt5${component}_DEFINITIONS}")


            #foreach(include_dir ${Qt5${_component}_INCLUDE_DIRS})
            #  	message(STATUS "${include_dir}")
            #endforeach()


            list(APPEND ${QT_LIB_NAME}_INCLUDE_DIRS ${Qt5${component}_INCLUDE_DIRS})
            list(APPEND ${QT_LIB_NAME}_LIBRARIES ${Qt5${component}_LIBRARIES})

            list(APPEND QT_DEFINITIONS ${Qt5${component}_DEFINITIONS})

        endif()
        #message("${Qt5${_component}_INCLUDE_DIRS}")

    endforeach()
   
    list(REMOVE_DUPLICATES ${QT_LIB_NAME}_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES ${QT_LIB_NAME}_LIBRARIES)
    list(REMOVE_DUPLICATES QT_DEFINITIONS)

    #hide some additional dependencies..
    mark_as_advanced(Qt5_DIR Qt5WebEngineCore_DIR Qt5WebChannel_DIR Qt5Quick_DIR Qt5Qml_DIR Qt5Positioning_DIR)

endmacro()



# Find DCMTk library
macro( ADD_LIB_DCMTK )

    ADD_LIB(${DCMTK_LIB_NAME} ${ARGV})

    if(WIN32)
        target_link_libraries(${TRIDIM_CURRENT_TARGET} PRIVATE netapi32 wsock32)
    endif()

endmacro()


macro( ADD_LIB_XLAB )

    ADD_LIB(${XLAB_LIB_NAME} ${ARGV})

endmacro()
