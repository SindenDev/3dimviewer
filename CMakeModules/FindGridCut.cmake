# CMake module that tries to find GRIDCUT includes and libraries.
#
# The following variables are set if GRIDCUT is found:
#   GRIDCUT_FOUND         - True when the GRIDCUT include directory is found.
#   GRIDCUT_INCLUDE_DIRS   - The directory where GRIDCUT include files are.
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(GRIDCUT)
#   ...
#   INCLUDE_DIRECTORIES(${GRIDCUT_INCLUDE_DIRS})
#   ...
#
# Written for Tescan 3Dim by Vojtech Tryhuk
#


# Initialize the search path
set( GRIDCUT_DIR_SEARCH "" )

# Others predefined search directories
  set( GRIDCUT_DIR_SEARCH
       ${GRIDCUT_DIR_SEARCH}
       ${GRIDCUT_ROOT_DIR}
       /usr/include/GridCut
       /usr/local/include/GridCut )


# Try to find GRIDCUT include directory
FIND_PATH( GRIDCUT_INCLUDE_DIRS NAMES GridGraph_2D_4C.h PATHS ${GRIDCUT_DIR_SEARCH} )

#message(${GRIDCUT_INCLUDE_DIRS})

# Assume we didn't find it
if (GRIDCUT_INCLUDE_DIRS)
  set( GRIDCUT_FOUND TRUE )
  set( GridCut_FOUND TRUE )
else()
  set( GRIDCUT_FOUND FALSE )
  set( GridCut_FOUND FALSE )
endif()


MARK_AS_ADVANCED( GRIDCUT_INCLUDE_DIRS )

