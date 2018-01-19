# CMake module that tries to find Bullet includes and libraries.
#
# The following variables are set if Bullet is found:
#   BULLET_FOUND         - True when the Bullet include directory is found.
#   BULLET_INCLUDE_DIRS   - The directory where BULLET include files are.
#   BULLET_LIBRARY_DIRS - The directory where BULLET libraries are.
#   BULLET_LIBRARIES     - List of all BULLET libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(BULLET)
#   ...
#   INCLUDE_DIRECTORIES(${BULLET_INCLUDE_DIRS})
#   LINK_DIRECTORIES(${BULLET_LIBRARY_DIRS})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${BULLET_LIBRARIES} )
#
# Written for BlueSkyPlan by Martin Wilczak
#

# Initialize the search path
set( BULLET_DIR_SEARCH "" )

# User defined CMake cache variable
if( BULLET_ROOT_DIR )
  set( BULLET_DIR_SEARCH ${BULLET_DIR_SEARCH} ${BULLET_ROOT_DIR} ${BULLET_ROOT_DIR}/include )
endif()

# Others predefined search directories

  set( BULLET_DIR_SEARCH
       ${BULLET_DIR_SEARCH}
       /usr/include
       /usr/local/include
       /usr/local/include/bullet
       /usr/local/include/bullet3
       /opt/include
       /opt/local/include
       ~/Bullet/include
       ~/Library/Frameworks
       /Library/Frameworks
       /sw/include
       /opt/csw/include
       /usr/freeware/include )

# Try to find Bullet include directory
find_path( BULLET_INCLUDE_DIRS NAMES BulletCollision/btBulletCollisionCommon.h PATHS ${BULLET_DIR_SEARCH} )

# Assume we didn't find it
set( BULLET_FOUND FALSE )

# Now try to get the library path
if( BULLET_INCLUDE_DIRS )

  # Look for the Bullet library path
  set( BULLET_LIBRARY_DIRS ${BULLET_INCLUDE_DIRS} )

  # Strip off the trailing "/include" in the path    

  if( "${BULLET_LIBRARY_DIRS}" MATCHES "/include/bullet$" )
    get_filename_component( BULLET_LIBRARY_DIRS ${BULLET_LIBRARY_DIRS} PATH )
  endif()

  if( "${BULLET_LIBRARY_DIRS}" MATCHES "/include$" )
    get_filename_component( BULLET_LIBRARY_DIRS ${BULLET_LIBRARY_DIRS} PATH )
  endif()

  # Check if the 'lib' directory exists
  if( EXISTS "${BULLET_LIBRARY_DIRS}/lib" )
    set( BULLET_LIBRARY_DIRS ${BULLET_LIBRARY_DIRS}/lib )
  endif()


  # Macro used to find specified Bullet library
  macro( FIND_BULLET_LIBRARY NAME )
    find_library( BULLET_${NAME}_LIBRARY
                  NAMES ${NAME}
                  PATHS ${BULLET_LIBRARY_DIRS} )

    find_library( BULLET_${NAME}_Debug_LIBRARY
                  NAMES ${NAME}d
                  PATHS ${BULLET_LIBRARY_DIRS} )

    mark_as_advanced(BULLET_${NAME}_LIBRARY BULLET_${NAME}_Debug_LIBRARY)

    if( BULLET_${NAME}_LIBRARY AND BULLET_${NAME}_Debug_LIBRARY )
        get_filename_component( bullet_lib_name ${BULLET_${NAME}_LIBRARY} NAME )
        get_filename_component( bullet_debug_lib_name ${BULLET_${NAME}_Debug_LIBRARY} NAME )


      set( BULLET_LIBRARIES ${BULLET_LIBRARIES} optimized ${bullet_lib_name} debug ${bullet_debug_lib_name} )
    else()
      if( BULLET_${NAME}_LIBRARY )
          get_filename_component( bullet_lib_name ${BULLET_${NAME}_LIBRARY} NAME )

        set( BULLET_LIBRARIES ${BULLET_LIBRARIES} general ${bullet_lib_name} )
      else()
        if( BULLET_${NAME}_Debug_LIBRARY )
            get_filename_component( bullet_debug_lib_name ${BULLET_${NAME}_Debug_LIBRARY} NAME )

          set( BULLET_LIBRARIES ${BULLET_LIBRARIES} general ${bullet_debug_lib_name} )
        endif()
      endif()
    endif()
  endmacro()


  # Found Bullet libraries
  set( BULLET_LIBRARIES "" )

  # Find all required Bullet libraries
  FIND_BULLET_LIBRARY(BulletCollision)
  FIND_BULLET_LIBRARY(BulletDynamics)
  FIND_BULLET_LIBRARY(LinearMath)
  FIND_BULLET_LIBRARY(ConvexDecomposition)
  FIND_BULLET_LIBRARY(HACD)

  if( BULLET_LIBRARIES )
    set( BULLET_FOUND TRUE )
    set( Bullet_FOUND TRUE )

    set( Bullet_LIBRARIES ${BULLET_LIBRARIES} )
    set( Bullet_INCLUDE_DIRS ${BULLET_INCLUDE_DIRS} )
    set( Bullet_LIBRARY_DIRS ${BULLET_LIBRARY_DIRS} )

  endif()

endif()


if( NOT BULLET_FOUND )
  set( BULLET_ROOT_DIR "" CACHE PATH "" )
endif()

mark_as_advanced( BULLET_INCLUDE_DIRS )
