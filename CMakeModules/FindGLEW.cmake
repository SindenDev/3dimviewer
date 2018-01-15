# - Try to find GLEW
# Once done this will define
#  
#  GLEW_FOUND        - system has GLEW
#  GLEW_INCLUDE_DIR  - the GLEW include directory
#  GLEW_LIBRARY_DIR  - where the libraries are
#  GLEW_LIBRARY      - Link these to use GLEW
#
# Modified for 3DimViewer by Michal Spanel
#

if (GLEW_INCLUDE_DIRS)
  # Already in cache, be silent
  set(GLEW_FIND_QUIETLY TRUE)
endif ()

if( NOT WIN32 )

   find_path( GLEW_INCLUDE_DIRS glew.h wglew.h
              PATHS
              /usr/local/include
              /usr/include
              /opt/local/include
              /opt/include
              PATH_SUFFIXES gl/ GL/
              )

   set( GLEW_NAMES glew GLEW )

   find_library( GLEW_LIBRARIES
                 NAMES ${GLEW_NAMES}
                 PATHS
                 /usr/lib
                 /usr/local/lib
                 /opt/lib
                 /opt/local/lib
                 )
endif()

get_filename_component( GLEW_LIBRARY_DIRS ${GLEW_LIBRARIES} PATH )
get_filename_component( GLEW_LIBRARIES ${GLEW_LIBRARIES} NAME )

if (GLEW_INCLUDE_DIRS AND GLEW_LIBRARIES)
    set(GLEW_LIBRARIES general ${GLEW_LIBRARIES})
   set(GLEW_FOUND TRUE)
else ()
   set( GLEW_FOUND FALSE )
   set( GLEW_LIBRARY_DIRS "")
endif ()

MARK_AS_ADVANCED(
  GLEW_LIBRARIES
  GLEW_INCLUDE_DIRS
)
