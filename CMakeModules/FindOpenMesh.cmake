# CMake module that tries to find OpenMesh includes and libraries.
#
# The following variables are set if OpenMesh is found:
#   OPENMESH_FOUND         - True when the OpenMesh include directory is found.
#   OPENMESH_INCLUDE_DIR   - The directory where OpenMesh include files are.
#   OPENMESH_LIBRARIES_DIR - The directory where OpenMesh libraries are.
#   OPENMESH_LIBRARIES     - List of all OpenMesh libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(OPENMESH)
#   ...
#   INCLUDE_DIRECTORIES(${OPENMESH_INCLUDE_DIR})
#   LINK_DIRECTORIES(${OPENMESH_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${OPENMESH_LIBRARIES} )
#
# Written for BlueSkyPlan by Martin Wilczak
#

# Macro used to find specified OpenMesh library
macro( FIND_OPENMESH_LIBRARY NAME )
  FIND_LIBRARY( OPENMESH_${NAME}_LIBRARY
                NAMES ${NAME}
                PATHS
                ${OPENMESH_LIBRARY_DIRS}
                ${OPENMESH_LIBRARY_DIRS}/OpenMesh
                )
  FIND_LIBRARY( OPENMESH_${NAME}d_LIBRARY
                NAMES ${NAME}d
                PATHS
                ${OPENMESH_LIBRARY_DIRS}
                ${OPENMESH_LIBRARY_DIRS}/OpenMesh
                )

  if( OPENMESH_${NAME}_LIBRARY AND OPENMESH_${NAME}d_LIBRARY )
      get_filename_component(lib_name ${OPENMESH_${NAME}_LIBRARY} NAME)
      get_filename_component(lib_named ${OPENMESH_${NAME}d_LIBRARY} NAME)

      get_filename_component(OPENMESH_LIBRARY_DIRS ${OPENMESH_${NAME}_LIBRARY} PATH)

      set( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} optimized ${lib_name} debug ${lib_named} )
  else()
    if( OPENMESH_${NAME}_LIBRARY )
        get_filename_component(lib_name ${OPENMESH_${NAME}_LIBRARY} NAME)
        get_filename_component(OPENMESH_LIBRARY_DIRS ${OPENMESH_${NAME}_LIBRARY} PATH)

      set( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} general ${lib_name} )
    else()
      if( OPENMESH_${NAME}d_LIBRARY )
          get_filename_component(lib_named ${OPENMESH_${NAME}d_LIBRARY} NAME)
          get_filename_component(OPENMESH_LIBRARY_DIRS ${OPENMESH_${NAME}d_LIBRARY} PATH)

        set( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} general ${lib_named} )
      endif()
    endif()
  endif()

endmacro()

# Initialize the search path
set( OPENMESH_DIR_SEARCH "" )

# User defined CMake cache variable
if( OPENMESH_ROOT_DIR )
  set( OPENMESH_DIR_SEARCH ${OPENMESH_DIR_SEARCH} ${OPENMESH_ROOT_DIR} ${OPENMESH_ROOT_DIR}/include )
endif()

# Others predefined search directories
if( NOT WIN32 )
  set( OPENMESH_DIR_SEARCH
       ${OPENMESH_DIR_SEARCH}
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/OpenMesh/include
       ~/Library/Frameworks
       /Library/Frameworks
       /sw/include
       /opt/csw/include
       /usr/freeware/include )
endif()


# Try to find OpenMesh include directory
find_path( OPENMESH_INCLUDE_DIRS NAMES OpenMesh/Core/System/config.hh PATHS ${OPENMESH_DIR_SEARCH} )

# Assume we didn't find it
set( OPENMESH_FOUND "FALSE" )

# Now try to get the library path
if( OPENMESH_INCLUDE_DIRS )

  # Look for the OpenMesh library path
  set( OPENMESH_LIBRARY_DIRS ${OPENMESH_INCLUDE_DIRS} )

  # Strip off the trailing "/include" in the path
  if( "${OPENMESH_LIBRARY_DIRS}" MATCHES "/include$" )
    get_filename_component( OPENMESH_LIBRARY_DIRS ${OPENMESH_LIBRARY_DIRS} PATH )
  endif()

  # Check if the 'lib' directory exists
  if( EXISTS "${OPENMESH_LIBRARY_DIRS}/lib" )
    set( OPENMESH_LIBRARY_DIRS ${OPENMESH_LIBRARY_DIRS}/lib )
  endif()

  # Found OpenMesh libraries
  set( OPENMESH_LIBRARIES "" )

  # Find all required OpenMesh libraries
  FIND_OPENMESH_LIBRARY(OpenMeshCore)
  FIND_OPENMESH_LIBRARY(OpenMeshTools)

  if( OPENMESH_LIBRARIES )
    set( OPENMESH_FOUND "TRUE" )
    set( OpenMesh_FOUND "TRUE" )

    set(OpenMesh_INCLUDE_DIRS ${OPENMESH_INCLUDE_DIRS})
    set(OpenMesh_LIBRARY_DIRS ${OPENMESH_LIBRARY_DIRS})
    set(OpenMesh_LIBRARIES    ${OPENMESH_LIBRARIES}  )
  endif()

endif()


if( NOT OPENMESH_FOUND )
  set( OPENMESH_ROOT_DIR "" CACHE PATH "" )
endif()

MARK_AS_ADVANCED( OPENMESH_INCLUDE_DIRS
                  OPENMESH_OpenMeshCore_LIBRARY
                  OPENMESH_OpenMeshTools_LIBRARY
                  OPENMESH_OpenMeshCored_LIBRARY
                  OPENMESH_OpenMeshToolsd_LIBRARY )

