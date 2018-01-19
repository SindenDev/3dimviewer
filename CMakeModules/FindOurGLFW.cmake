# - Try to find GLEW
# Once done this will define
#  
#  GLFW_FOUND        - system has GLEW
#  GLFW_INCLUDE_DIR  - the GLEW include directory
#  GLFW_LIBRARY_DIR  - where the libraries are
#  GLFW_LIBRARY      - Link these to use GLEW
#
# Modified for 3DimViewer by Vit Stancl
#

IF (GLFW_INCLUDE_DIR)
  # Already in cache, be silent
  SET(GLFW_FIND_QUIETLY TRUE)
ENDIF (GLFW_INCLUDE_DIR)

if( WIN32 )
   if( MSVC80 )
       set( COMPILER_PATH "C:/Program\ Files/Microsoft\ Visual\ Studio\ 8/VC" )
   endif( MSVC80 )
   if( MSVC71 )
       set( COMPILER_PATH "C:/Program\ Files/Microsoft\ Visual\ Studio\ .NET\ 2003/Vc7" )
   endif( MSVC71 )
   FIND_PATH( GLFW_INCLUDE_DIR gl/GLFW.h 
              PATHS
              "${TRIDIM_3RDPARTY_DIR}/include"
              ${COMPILER_PATH}/PlatformSDK/Include
              )
   SET( GLFW_DEBUG_NAME GLFWD)
   SET( GLFW_OPTIMIZED_NAME GLFW)
   
   FIND_LIBRARY( GLFW_LIBRARY_DEBUG
                 NAMES ${GLFW_DEBUG_NAME} 
                 PATHS
                 "${TRIDIM_3RDPARTY_DIR}/lib"
                 ${COMPILER_PATH}/PlatformSDK/Lib
                 )
                 
    
   FIND_LIBRARY( GLFW_LIBRARY_OPTIMIZED
                 NAMES ${GLFW_OPTIMIZED_NAME} 
                 PATHS
                 "${TRIDIM_3RDPARTY_DIR}/lib"
                 ${COMPILER_PATH}/PlatformSDK/Lib
                 )             
                 
                 
else( WIN32 )
   FIND_PATH( GLFW_INCLUDE_DIR GLFW.h GLFW.h
              PATHS
              "${TRIDIM_3RDPARTY_DIR}/include"
              /usr/local/include
              /usr/include
              /opt/local/include
              /opt/include
              PATH_SUFFIXES gl/ GL/
              )
 
   FIND_LIBRARY( GLFW_LIBRARY_DEBUG
                 NAMES ${GLFW_DEBUG_NAME} 
                 PATHS
                 "${TRIDIM_3RDPARTY_DIR}/lib"
                 /usr/lib
                 /usr/local/lib
                 /opt/lib
                 /opt/local/lib
                 )
                 
    FIND_LIBRARY( GLFW_LIBRARY_OPTIMIZED
                 NAMES ${GLFW_OPTIMIZED_NAME}
                 PATHS
                 "${TRIDIM_3RDPARTY_DIR}/lib"
                 /usr/lib
                 /usr/local/lib
                 /opt/lib
                 /opt/local/lib
                 )
                 
endif( WIN32 )

GET_FILENAME_COMPONENT( GLFW_LIBRARY_DIR ${GLFW_LIBRARY_DEBUG} PATH )

IF (GLFW_INCLUDE_DIR AND GLFW_LIBRARY_DEBUG AND GLFW_LIBRARY_OPTIMIZED)
   SET(GLFW_FOUND TRUE)
   SET( GLFW_LINKED_LIBS debug ${GLFW_LIBRARY_DEBUG} optimized ${GLFW_LIBRARY_OPTIMIZED}  )
   SET( GLFW_LIBRARY debug ${GLFW_LIBRARY_DEBUG} optimized ${GLFW_LIBRARY_OPTIMIZED})
    SET( GLFW_LIBRARY_DIR ${GLFW_LIBRARY_DEBUG} )
ELSE ()
   SET( GLFW_FOUND FALSE )
   SET( GLFW_LIBRARY_DIR )
ENDIF ()

MARK_AS_ADVANCED(
  GLFW_LIBRARY
  GLFW_INCLUDE_DIR
)  


