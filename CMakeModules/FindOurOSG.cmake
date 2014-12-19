# CMake module that tries to find OSG includes and libraries.
#
# The following variables are set if OSG is found:
#   OSG_FOUND         - True when the OSG include directory is found.
#   OSG_INCLUDE_DIR   - The directory where OSG include files are.
#   OSG_LIBRARIES_DIR - The directory where OSG libraries are.
#   OSG_LIBRARIES     - List of all OSG libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(OSG)
#   ...
#   INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
#   LINK_DIRECTORIES(${OSG_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${OSG_LIBRARIES} )
#
# Modified for 3DimViewer by Vit Stancl and Michal Spanel
#


# Initialize the search path
SET( OSG_DIR_SEARCH "" )

# User defined environment variable OSG_ROOT_DIR
IF( EXISTS $ENV{OSG_ROOT_DIR} )
  FILE( TO_CMAKE_PATH $ENV{OSG_ROOT_DIR} OSG_DIR_SEARCH)
  SET( OSG_DIR_SEARCH ${OSG_DIR_SEARCH} ${OSG_DIR_SEARCH}/include )
ENDIF( EXISTS $ENV{OSG_ROOT_DIR} )

# User defined CMake cache variable
IF( OSG_ROOT_DIR )
  SET( OSG_DIR_SEARCH ${OSG_DIR_SEARCH} ${OSG_ROOT_DIR} ${OSG_ROOT_DIR}/include )
ENDIF( OSG_ROOT_DIR )

# Others predefined search directories
IF( WIN32 )
  SET( OSG_DIR_SEARCH
       ${OSG_DIR_SEARCH}
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/OSG"
       "${TRIDIM_MOOQ_DIR}/OSG/include"
       "${TRIDIM_3RDPARTY_DIR}"
       "${TRIDIM_3RDPARTY_DIR}/include"
       "C:/Program Files/OSG"
       "C:/Program Files/OSG/include"
       "C:/Program Files/OpenSceneGraph"
       "C:/Program Files/OpenSceneGraph/include"
       C:/OSG
       C:/OSG/include
       D:/OSG
       D:/OSG/include
       [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include )
ELSE( WIN32 )
  SET( OSG_DIR_SEARCH
       ${OSG_DIR_SEARCH}
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/OSG"
       "${TRIDIM_MOOQ_DIR}/OSG/include"
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/OSG/include
       ~/Library/Frameworks
       /Library/Frameworks
       /sw/include # Fink
       /opt/csw/include # Blastwave
       /usr/freeware/include )
ENDIF( WIN32 )


# Try to find OSG include directory
FIND_PATH( OSG_INCLUDE_DIR NAMES osg/Node PATHS ${OSG_DIR_SEARCH} )


# Assume we didn't find it
SET( OSG_FOUND "NO" )

# Now try to get the library path
IF( OSG_INCLUDE_DIR )

  # Look for the OSG library path
  SET( OSG_LIBRARIES_DIR ${OSG_INCLUDE_DIR} )

  # Strip off the trailing "/include" in the path
  IF( "${OSG_LIBRARIES_DIR}" MATCHES "/include$" )
    GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARIES_DIR} PATH )
  ENDIF( "${OSG_LIBRARIES_DIR}" MATCHES "/include$" )

  # Check if the 'lib' directory exists
  IF( EXISTS "${OSG_LIBRARIES_DIR}/lib" )
    SET( OSG_LIBRARIES_DIR ${OSG_LIBRARIES_DIR}/lib )
  ENDIF( EXISTS "${OSG_LIBRARIES_DIR}/lib" )


  # Macro used to find specified OSG library
  MACRO( FIND_OSG_LIBRARY NAME )
    FIND_LIBRARY( OSG_${NAME}_LIBRARY
                  NAMES ${NAME}
                  PATHS ${OSG_LIBRARIES_DIR} )
    FIND_LIBRARY( OSG_${NAME}d_LIBRARY
                  NAMES ${NAME}d
                  PATHS ${OSG_LIBRARIES_DIR} )
    IF( OSG_${NAME}_LIBRARY AND OSG_${NAME}d_LIBRARY )
      SET( OSG_LIBRARIES ${OSG_LIBRARIES} optimized ${NAME} debug ${NAME}d )
    ELSE( OSG_${NAME}_LIBRARY AND OSG_${NAME}d_LIBRARY )
      IF( OSG_${NAME}_LIBRARY )
        SET( OSG_LIBRARIES ${OSG_LIBRARIES} ${NAME} )
      ELSE( OSG_${NAME}_LIBRARY )
        IF( OSG_${NAME}d_LIBRARY )
          SET( OSG_LIBRARIES ${OSG_LIBRARIES} ${NAME}d )
        ENDIF( OSG_${NAME}d_LIBRARY )
      ENDIF( OSG_${NAME}_LIBRARY )
    ENDIF( OSG_${NAME}_LIBRARY AND OSG_${NAME}d_LIBRARY )
  ENDMACRO( FIND_OSG_LIBRARY )


  # Found OSG libraries
  SET( OSG_LIBRARIES "" )

  # Find all required OSG libraries
  FIND_OSG_LIBRARY(OpenThreads)
  FIND_OSG_LIBRARY(osg)
  FIND_OSG_LIBRARY(osgUtil)
  FIND_OSG_LIBRARY(osgDB)
  FIND_OSG_LIBRARY(osgText)
  FIND_OSG_LIBRARY(osgTerrain)
  FIND_OSG_LIBRARY(osgFX)
  FIND_OSG_LIBRARY(osgViewer)
  FIND_OSG_LIBRARY(osgGA)
  FIND_OSG_LIBRARY(osgManipulator)
  FIND_OSG_LIBRARY(osgWidget)
  
  IF( OSG_LIBRARIES )
    SET( OSG_FOUND "YES" )
  ENDIF( OSG_LIBRARIES )

ENDIF( OSG_INCLUDE_DIR )


IF( NOT OSG_FOUND )
  SET( OSG_ROOT_DIR "" CACHE PATH "" )
ENDIF( NOT OSG_FOUND )

MARK_AS_ADVANCED( OSG_INCLUDE_DIR
                  OSG_osg_LIBRARY
                  OSG_osgUtil_LIBRARY
                  OSG_osgDB_LIBRARY
                  OSG_osgText_LIBRARY
                  OSG_osgTerrain_LIBRARY
                  OSG_osgFX_LIBRARY
                  OSG_osgViewer_LIBRARY
                  OSG_osgGA_LIBRARY
                  OSG_osgManipulator_LIBRARY
                  OSG_osgWidget_LIBRARY
                  OSG_osgd_LIBRARY
                  OSG_osgUtild_LIBRARY
                  OSG_osgDBd_LIBRARY
                  OSG_osgTextd_LIBRARY
                  OSG_osgTerraind_LIBRARY
                  OSG_osgFXd_LIBRARY
                  OSG_osgViewerd_LIBRARY
                  OSG_osgManipulatord_LIBRARY
                  OSG_osgGAd_LIBRARY
                  OSG_osgWidgetd_LIBRARY )

