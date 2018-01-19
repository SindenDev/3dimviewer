# CMake module that tries to find XLAB includes and libraries.
#
# The following variables are set if XLAB is found:
#   XLAB_FOUND         - True when the XLAB include directory is found.
#   XLAB_INCLUDE_DIR   - The directory where XLAB include files are.
#   XLAB_LIBRARIES_DIR - The directory where XLAB libraries are.
#   XLAB_LIBRARIES     - List of all XLAB libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(XLAB)
#   ...
#   INCLUDE_DIRECTORIES(${XLAB_INCLUDE_DIR})
#   LINK_DIRECTORIES(${XLAB_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${XLAB_LIBRARIES} )
#
# Modified for 3DimViewer by Vit Stancl and Michal Spanel
#


# Initialize the search path
SET( XLAB_DIR_SEARCH "" )

# User defined CMake cache variable
if(XLAB_ROOT_DIR)
  set( XLAB_DIR_SEARCH ${XLAB_DIR_SEARCH} ${XLAB_ROOT_DIR} ${XLAB_ROOT_DIR}/include )
endif()

# Others predefined search directories
if( NOT WIN32 )
  set( XLAB_DIR_SEARCH
       ${XLAB_DIR_SEARCH}
       /usr/lib
       /usr/local/lib
       /opt/lib
       /opt/local/lib )
endif()

# macro used to find specified XLAB library
macro( FIND_XLAB_LIBRARY NAME )
  find_library( XLAB_${NAME}_LIBRARY
                NAMES ${NAME}
                PATHS ${XLAB_LIBRARIES_DIR} )
  find_library( XLAB_${NAME}d_LIBRARY
                NAMES ${NAME}d
                PATHS ${XLAB_LIBRARIES_DIR} )


  if( XLAB_${NAME}_LIBRARY AND XLAB_${NAME}d_LIBRARY )

      get_filename_component(lib_name ${XLAB_${NAME}_LIBRARY} NAME)
      get_filename_component(lib_named ${XLAB_${NAME}d_LIBRARY} NAME)

    set( XLAB_LIBRARIES ${XLAB_LIBRARIES} optimized ${lib_name} debug ${lib_named}d )
  else()
    if( XLAB_${NAME}_LIBRARY )
        get_filename_component(lib_name ${XLAB_${NAME}_LIBRARY} NAME)

      SET( XLAB_LIBRARIES ${XLAB_LIBRARIES} general ${lib_name} )
    else()
      if( XLAB_${NAME}d_LIBRARY )
          get_filename_component(lib_named ${XLAB_${NAME}d_LIBRARY} NAME)

        set( XLAB_LIBRARIES ${XLAB_LIBRARIES} ${lib_named}d ) #??
      endif()
    endif()
  endif()
endmacro()

# Try to find XLAB include directory
find_path( XLAB_LIBRARY_DIRS NAMES "libBSP_CAD.dylib" PATHS ${XLAB_DIR_SEARCH} )

# Assume we didn't find it
set( XLAB_FOUND "FALSE" )
set( Xlab_FOUND "FALSE" )

set(XLAB_INCLUDE_DIRS "")
set(Xlab_INCLUDE_DIRS "")

if(NOT "${XLAB_LIBRARY_DIRS}" STREQUAL "")
    set(Xlab_LIBRARY_DIRS "${XLAB_LIBRARY_DIRS}")
endif()

# Found XLAB libraries
set( XLAB_LIBRARIES "" )

# Find all required XLAB libraries
FIND_XLAB_LIBRARY(BSP_CAD)

if( XLAB_LIBRARIES )
    set( XLAB_FOUND "TRUE" )
    set( Xlab_FOUND "TRUE" )
    set(Xlab_LIBRARIES "${XLAB_LIBRARIES}")
endif()


#ask user to help us out..
if(NOT XLAB_FOUND)
  set(XLAB_ROOT_DIR "" CACHE PATH "")
endif()

mark_as_advanced( XLAB_INCLUDE_DIRS
                  XLAB_LIBRARY_DIRS
                  XLAB_BSP_CAD_LIBRARY
                  XLAB_BSP_CADd_LIBRARY
                  )

