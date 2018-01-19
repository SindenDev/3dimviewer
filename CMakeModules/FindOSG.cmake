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

# User defined CMake cache variable
if(OSG_ROOT_DIR)
  set( OSG_DIR_SEARCH ${OSG_DIR_SEARCH} ${OSG_ROOT_DIR} ${OSG_ROOT_DIR}/include )
endif()

# Others predefined search directories
if( NOT WIN32 )
  set( OSG_DIR_SEARCH
       ${OSG_DIR_SEARCH}
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
endif()

# macro used to find specified OSG library
macro( FIND_OSG_LIBRARY NAME )
  find_library( OSG_${NAME}_LIBRARY
                NAMES ${NAME}
                PATHS ${OSG_LIBRARIES_DIR} )
  find_library( OSG_${NAME}d_LIBRARY
                NAMES ${NAME}d
                PATHS ${OSG_LIBRARIES_DIR} )


  if( OSG_${NAME}_LIBRARY AND OSG_${NAME}d_LIBRARY )

      get_filename_component(lib_name ${OSG_${NAME}_LIBRARY} NAME)
      get_filename_component(lib_named ${OSG_${NAME}d_LIBRARY} NAME)

    set( OSG_LIBRARIES ${OSG_LIBRARIES} optimized ${lib_name} debug ${lib_named}d )
  else()
    if( OSG_${NAME}_LIBRARY )
        get_filename_component(lib_name ${OSG_${NAME}_LIBRARY} NAME)

      SET( OSG_LIBRARIES ${OSG_LIBRARIES} general ${lib_name} )
    else()
      if( OSG_${NAME}d_LIBRARY )
          get_filename_component(lib_named ${OSG_${NAME}d_LIBRARY} NAME)

        set( OSG_LIBRARIES ${OSG_LIBRARIES} ${lib_named}d ) #??
      endif()
    endif()
  endif()
endmacro()

# Try to find OSG include directory
find_path( OSG_INCLUDE_DIRS NAMES osg/Node PATHS ${OSG_DIR_SEARCH} )


# Assume we didn't find it
SET( OSG_FOUND "FALSE" )

# Now try to get the library path
if( OSG_INCLUDE_DIRS )

  # Look for the OSG library path
  SET( OSG_LIBRARY_DIRS ${OSG_INCLUDE_DIRS} )

  # Strip off the trailing "/include" in the path
  if( "${OSG_LIBRARY_DIRS}" MATCHES "/include$" )
    get_filename_component( OSG_LIBRARY_DIRS ${OSG_LIBRARY_DIRS} PATH )
  endif()

  # Check if the 'lib' directory exists
  if( EXISTS "${OSG_LIBRARY_DIRS}/lib" )
    set( OSG_LIBRARY_DIRS ${OSG_LIBRARY_DIRS}/lib )
  endif()


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
  
  if( OSG_LIBRARIES )
    set( OSG_FOUND "TRUE" )
  endif()

endif()

#ask user to help us out..
if(NOT OSG_FOUND)
  set(OSG_ROOT_DIR "" CACHE PATH "")
endif()

mark_as_advanced( OSG_INCLUDE_DIRS
                  OSG_OpenThreads_LIBRARY
                  OSG_OpenThreadsd_LIBRARY
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

