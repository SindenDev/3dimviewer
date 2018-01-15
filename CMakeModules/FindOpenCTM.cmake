# CMake module that tries to find OpenCTM includes and libraries.
#
# The following variables are set if OpenCTM is found:
#   OPENCTM_FOUND         - True when the OpenCTM include directory is found.
#   OPENCTM_INCLUDE_DIRS   - The directory where OpenCTM include files are.
#   OPENCTM_LIBRARY_DIRS - The directory where OpenCTM libraries are.
#   OPENCTM_LIBRARIES     - List of all OpenCTM libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(OPENCTM)
#   ...
#   INCLUDE_DIRECTORIES(${OPENCTM_INCLUDE_DIRS})
#   LINK_DIRECTORIES(${OPENCTM_LIBRARY_DIRS})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${OPENCTM_LIBRARIES} )
#
# Written for 3DIM Laboratory by Martin Wilczak
#


# Initialize the search path
set( OPENCTM_DIR_SEARCH "" )

# Others predefined search directories
  set( OPENCTM_DIR_SEARCH
       ${OPENCTM_DIR_SEARCH}
       ${OPENCTM_ROOT_DIR}
       /usr/include
       /usr/local/include )


# Try to find OpenCTM include directory
FIND_PATH( OPENCTM_INCLUDE_DIRS NAMES openctm.h PATHS ${OPENCTM_DIR_SEARCH} )


# Assume we didn't find it
set( OPENCTM_FOUND FALSE )
set( OpenCTM_FOUND FALSE )

# Now try to get the library path
if( OPENCTM_INCLUDE_DIRS )

  # Look for the OpenCTM library path
  set( OPENCTM_LIBRARY_DIRS ${OPENCTM_INCLUDE_DIRS} )

  # Strip off the trailing "/include" in the path
  if( "${OPENCTM_LIBRARY_DIRS}" MATCHES "/include$" )
    get_filename_component( OPENCTM_LIBRARY_DIRS ${OPENCTM_LIBRARY_DIRS} PATH )
  endif( "${OPENCTM_LIBRARY_DIRS}" MATCHES "/include$" )

  # Check if the 'lib' directory exists
  if( EXISTS "${OPENCTM_LIBRARY_DIRS}/lib" )
    set( OPENCTM_LIBRARY_DIRS ${OPENCTM_LIBRARY_DIRS}/lib )
  endif( EXISTS "${OPENCTM_LIBRARY_DIRS}/lib" )


  # Macro used to find specified OpenCTM library
  MACRO( FIND_OPENCTM_LIBRARY NAME )
    find_library( OPENCTM_${NAME}_LIBRARY
                  NAMES ${NAME}
                  PATHS 
                  ${OPENCTM_LIBRARY_DIRS}
                  ${OPENCTM_LIBRARY_DIRS}/OpenCTM
                  )

      if( OPENCTM_${NAME}_LIBRARY )
        get_filename_component( file ${OPENCTM_${NAME}_LIBRARY} NAME )

        set( OPENCTM_LIBRARIES ${OPENCTM_LIBRARIES} general ${file} )
      endif()

  endmacro()


  # Found OpenCTM libraries
  set( OPENCTM_LIBRARIES "" )

  # Find all required OpenCTM libraries
  FIND_OPENCTM_LIBRARY(openctm)

  if( OPENCTM_LIBRARIES )
    set( OPENCTM_FOUND TRUE )
    set( OpenCTM_FOUND TRUE )

    set(OpenCTM_INCLUDE_DIRS ${OPENCTM_INCLUDE_DIRS})
    set(OpenCTM_LIBRARY_DIRS ${OPENCTM_LIBRARY_DIRS})
    set(OpenCTM_LIBRARIES ${OPENCTM_LIBRARIES})
  endif()

endif()


if( NOT OPENCTM_FOUND )
  set( OPENCTM_ROOT_DIR "" CACHE PATH "" )
endif()

MARK_AS_ADVANCED( OPENCTM_INCLUDE_DIRS
                  OPENCTM_openctm_LIBRARY )

