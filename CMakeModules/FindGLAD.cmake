# CMake module that tries to find GLAD includes and libraries.
#
# The following variables are set if GLAD is found:
#   GLAD_FOUND         - True when the GLAD include directory is found.
#   GLAD_INCLUDE_DIRS   - The directory where GLAD include files are.
#   GLAD_LIBRARY_DIRS - The directory where GLAD libraries are.
#   GLAD_LIBRARIES     - List of all GLAD libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(GLAD)
#   ...
#   INCLUDE_DIRECTORIES(${GLAD_INCLUDE_DIRS})
#   LINK_DIRECTORIES(${GLAD_LIBRARY_DIRS})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${GLAD_LIBRARIES} )
#
# Written for 3DIM Laboratory by Martin Wilczak
#


# Initialize the search path
set( GLAD_DIR_SEARCH "" )

# Others predefined search directories
  set( GLAD_DIR_SEARCH
       ${GLAD_DIR_SEARCH}
       ${GLAD_ROOT_DIR}
       /usr/include/glad
       /usr/local/include/glad )


# Try to find GLAD include directory
FIND_PATH( GLAD_INCLUDE_DIRS NAMES glad.h PATHS ${GLAD_DIR_SEARCH} )


# Assume we didn't find it
set( GLAD_FOUND FALSE )
set( glad_FOUND FALSE )

# Now try to get the library path
if( GLAD_INCLUDE_DIRS )

  # Look for the GLAD library path
  set( GLAD_LIBRARY_DIRS ${GLAD_INCLUDE_DIRS} )

  # Strip off the trailing "/include/glad" in the path
  if( "${GLAD_LIBRARY_DIRS}" MATCHES "/include/glad$" )
  
    #yes, twice
    get_filename_component( GLAD_LIBRARY_DIRS ${GLAD_LIBRARY_DIRS} PATH )
    
    #just the part with include
    set( GLAD_INCLUDE_DIRS ${GLAD_LIBRARY_DIRS} )
    
    get_filename_component( GLAD_LIBRARY_DIRS ${GLAD_LIBRARY_DIRS} PATH )
  endif()

  # Check if the 'lib' directory exists
  if( EXISTS "${GLAD_LIBRARY_DIRS}/lib" )
    set( GLAD_LIBRARY_DIRS ${GLAD_LIBRARY_DIRS}/lib )
  endif()


  # Macro used to find specified GLAD library
  macro( FIND_GLAD_LIBRARY NAME )
    find_library( GLAD_${NAME}_LIBRARY
                  NAMES ${NAME}
                  PATHS 
                  ${GLAD_LIBRARY_DIRS}
                  )

      if( GLAD_${NAME}_LIBRARY )
        get_filename_component( file ${GLAD_${NAME}_LIBRARY} NAME )

        set( GLAD_LIBRARIES ${GLAD_LIBRARIES} general ${file} )
      endif()

  endmacro()


  # Found GLAD libraries
  set( GLAD_LIBRARIES "" )

  # Find all required GLAD libraries
  FIND_GLAD_LIBRARY(glad)

  if( GLAD_LIBRARIES )
    set( GLAD_FOUND TRUE )
    set( glad_FOUND TRUE )

    set(glad_INCLUDE_DIRS   ${GLAD_INCLUDE_DIRS})
    set(glad_LIBRARY_DIRS   ${GLAD_LIBRARY_DIRS})
    set(glad_LIBRARIES      ${GLAD_LIBRARIES})
  endif()

endif()


if( NOT GLAD_FOUND )
  set( GLAD_ROOT_DIR "" CACHE PATH "" )
endif()

MARK_AS_ADVANCED( GLAD_INCLUDE_DIRS
                  GLAD_glad_LIBRARY )

