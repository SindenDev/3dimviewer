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


# Initialize the search path
SET( OPENMESH_DIR_SEARCH "" )

# User defined environment variable OPENMESH_ROOT_DIR
IF( EXISTS $ENV{OPENMESH_ROOT_DIR} )
  FILE( TO_CMAKE_PATH $ENV{OPENMESH_ROOT_DIR} OPENMESH_DIR_SEARCH)
  SET( OPENMESH_DIR_SEARCH ${OPENMESH_DIR_SEARCH} ${OPENMESH_DIR_SEARCH}/include )
ENDIF( EXISTS $ENV{OPENMESH_ROOT_DIR} )

# User defined CMake cache variable
IF( OPENMESH_ROOT_DIR )
  SET( OPENMESH_DIR_SEARCH ${OPENMESH_DIR_SEARCH} ${OPENMESH_ROOT_DIR} ${OPENMESH_ROOT_DIR}/include )
ENDIF( OPENMESH_ROOT_DIR )

# Others predefined search directories
IF( WIN32 )
  SET( OPENMESH_DIR_SEARCH
       ${OPENMESH_DIR_SEARCH}
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/OpenMesh"
       "${TRIDIM_MOOQ_DIR}/OpenMesh/include"
       "${TRIDIM_3RDPARTY_DIR}"
       "${TRIDIM_3RDPARTY_DIR}/include"
       "C:/Program Files/OpenMesh"
       "C:/Program Files/OpenMesh/include"
       C:/OpenMesh
       C:/OpenMesh/include
       D:/OpenMesh
       D:/OpenMesh/include )
ELSE( WIN32 )
  SET( OPENMESH_DIR_SEARCH
       ${OPENMESH_DIR_SEARCH}
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/OpenMesh"
       "${TRIDIM_MOOQ_DIR}/OpenMesh/include"
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
ENDIF( WIN32 )


# Try to find OpenMesh include directory
FIND_PATH( OPENMESH_INCLUDE_DIR NAMES OpenMesh/Core/System/config.hh PATHS ${OPENMESH_DIR_SEARCH} )


# Assume we didn't find it
SET( OPENMESH_FOUND "NO" )

# Now try to get the library path
IF( OPENMESH_INCLUDE_DIR )

  # Look for the OpenMesh library path
  SET( OPENMESH_LIBRARIES_DIR ${OPENMESH_INCLUDE_DIR} )

  # Strip off the trailing "/include" in the path
  IF( "${OPENMESH_LIBRARIES_DIR}" MATCHES "/include$" )
    GET_FILENAME_COMPONENT( OPENMESH_LIBRARIES_DIR ${OPENMESH_LIBRARIES_DIR} PATH )
  ENDIF( "${OPENMESH_LIBRARIES_DIR}" MATCHES "/include$" )

  # Check if the 'lib' directory exists
  IF( EXISTS "${OPENMESH_LIBRARIES_DIR}/lib" )
    SET( OPENMESH_LIBRARIES_DIR ${OPENMESH_LIBRARIES_DIR}/lib )
  ENDIF( EXISTS "${OPENMESH_LIBRARIES_DIR}/lib" )


  # Macro used to find specified OpenMesh library
  MACRO( FIND_OPENMESH_LIBRARY NAME )
    FIND_LIBRARY( OPENMESH_${NAME}_LIBRARY
                  NAMES ${NAME}
                  PATHS 
                  ${OPENMESH_LIBRARIES_DIR} 
                  ${OPENMESH_LIBRARIES_DIR}/OpenMesh 
                  )
    FIND_LIBRARY( OPENMESH_${NAME}d_LIBRARY
                  NAMES ${NAME}d
                  PATHS 
                  ${OPENMESH_LIBRARIES_DIR} 
                  ${OPENMESH_LIBRARIES_DIR}/OpenMesh 
                  )
    IF( OPENMESH_${NAME}_LIBRARY AND OPENMESH_${NAME}d_LIBRARY )
#      SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} optimized ${NAME} debug ${NAME}d )
      SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} optimized ${OPENMESH_${NAME}_LIBRARY} debug ${OPENMESH_${NAME}d_LIBRARY} )
    ELSE( OPENMESH_${NAME}_LIBRARY AND OPENMESH_${NAME}d_LIBRARY )
      IF( OPENMESH_${NAME}_LIBRARY )
#        SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} ${NAME} )
        SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} ${OPENMESH_${NAME}_LIBRARY} )
      ELSE( OPENMESH_${NAME}_LIBRARY )
        IF( OPENMESH_${NAME}d_LIBRARY )
#          SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} ${NAME}d )
          SET( OPENMESH_LIBRARIES ${OPENMESH_LIBRARIES} ${OPENMESH_${NAME}d_LIBRARY} )
        ENDIF( OPENMESH_${NAME}d_LIBRARY )
      ENDIF( OPENMESH_${NAME}_LIBRARY )
    ENDIF( OPENMESH_${NAME}_LIBRARY AND OPENMESH_${NAME}d_LIBRARY )
  ENDMACRO( FIND_OPENMESH_LIBRARY )


  # Found OpenMesh libraries
  SET( OPENMESH_LIBRARIES "" )

  # Find all required OpenMesh libraries
  FIND_OPENMESH_LIBRARY(OpenMeshCore)
  FIND_OPENMESH_LIBRARY(OpenMeshTools)

  IF( OPENMESH_LIBRARIES )
    SET( OPENMESH_FOUND "YES" )
  ENDIF( OPENMESH_LIBRARIES )

ENDIF( OPENMESH_INCLUDE_DIR )


IF( NOT OPENMESH_FOUND )
  SET( OPENMESH_ROOT_DIR "" CACHE PATH "" )
ENDIF( NOT OPENMESH_FOUND )

MARK_AS_ADVANCED( OPENMESH_INCLUDE_DIR
                  OPENMESH_OpenMeshCore_LIBRARY
                  OPENMESH_OpenMeshTools_LIBRARY )

