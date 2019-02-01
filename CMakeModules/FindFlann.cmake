# - Try to find lib
#
# Once done this will define
#
#  FLANN_FOUND - system has lib
#  FLANN_INCLUDE_DIR - the include directory


# User defined CMake cache variable
if( FLANN_ROOT_DIR )
  set( FLANN_DIR_SEARCH ${FLANN_DIR_SEARCH} ${FLANN_ROOT_DIR} ${FLANN_ROOT_DIR}/include)
endif()

# Predefined search directories
set( FLANN_INC_SEARCHPATH
       "${FLANN_DIR_SEARCH}"
       /sw/include
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       )
set( FLANN_INC_SEARCHSUFFIXES
       FLANN
       include 
       include/flann
       flann/include
       )
find_path(FLANN_INCLUDE_DIRS NAMES flann.hpp PATH_SUFFIXES ${FLANN_INC_SEARCHSUFFIXES}
            PATHS 
            ${FLANN_INC_SEARCHPATH}
            )
    

if (FLANN_INCLUDE_DIRS)

    # Strip off the trailing "/flann" in the path
    string(TOLOWER "${FLANN_INCLUDE_DIRS}" FLANN_INCLUDE_LOWER) 
    if( "${FLANN_INCLUDE_LOWER}" MATCHES "/flann$" )
        get_filename_component( FLANN_INCLUDE_DIRS ${FLANN_INCLUDE_DIRS} PATH )
    endif()

    # Look for the FLANN library path
    set( FLANN_LIBRARY_DIRS ${FLANN_INCLUDE_DIRS} )

    # Strip off the trailing "/include" in the path
    if( "${FLANN_LIBRARY_DIRS}" MATCHES "/include$" )
      
        get_filename_component( FLANN_LIBRARY_DIRS ${FLANN_LIBRARY_DIRS} PATH )
    
        # Check if the 'lib' directory exists
        if( EXISTS "${FLANN_LIBRARY_DIRS}/lib" )
            set( FLANN_LIBRARY_DIRS ${FLANN_LIBRARY_DIRS}/lib )
        endif()
    endif()


    find_library(FLANN_libflann_cpp_LIBRARY
                    NAMES flann_cpp
                    PATHS 
                    ${FLANN_LIBRARY_DIRS}
                    )

  set(FLANN_LIBRARIES "")

  if (FLANN_INCLUDE_DIRS AND FLANN_libflann_cpp_LIBRARY)
      set(FLANN_FOUND TRUE)
      set(Flann_FOUND TRUE)
      
      set(Flann_INCLUDE_DIRS ${FLANN_INCLUDE_DIRS})
      set(Flann_LIBRARY_DIRS ${FLANN_LIBRARY_DIRS})

      get_filename_component(lib_name ${FLANN_libflann_cpp_LIBRARY} NAME)
      
      set(FLANN_LIBRARIES ${FLANN_LIBRARIES} general ${lib_name})
      set(Flann_LIBRARIES ${FLANN_LIBRARIES})

  else()
    set(FLANN_FOUND FALSE)
    set(Flann_FOUND FALSE)
  endif()

  mark_as_advanced(FLANN_INCLUDE_DIRS)

endif()

if( NOT FLANN_FOUND )
  set( FLANN_ROOT_DIR "" CACHE PATH "" )
endif()
