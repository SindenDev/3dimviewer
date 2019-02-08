# CMake module that tries to find TinyXML includes and libraries.
#
# The following variables are set if TinyXML is found:
#   TINYXML2_FOUND         - True when the TinyXML include directory is found.
#   TINYXML2_INCLUDE_DIR   - The directory where TinyXML include files are.
#   TINYXML2_LIBRARIES_DIR - The directory where TinyXML libraries are.
#   TINYXML2_LIBRARIES     - List of all TinyXML libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(TinyXML2)
#   ...
#   INCLUDE_DIRECTORIES(${TINYXML2_INCLUDE_DIR})
#   LINK_DIRECTORIES(${TINYXML2_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${TINYXML2_LIBRARIES} )
#
# Written for BlueSkyPlan by Vojtech Tryhuk
#

# Initialize the search path
SET( TINYXML2_DIR_SEARCH "" )

# User defined CMake cache variable
if(TINYXML2_ROOT_DIR)
  set( TINYXML2_DIR_SEARCH ${TINYXML2_DIR_SEARCH} ${TINYXML2_ROOT_DIR} ${TINYXML2_ROOT_DIR}/include )
endif()

# Others predefined search directories
if( NOT WIN32 )
  set( TINYXML2_DIR_SEARCH
       ${TINYXML2_DIR_SEARCH}
       /opt/local/include
       /usr/local/include
       /usr/local/include/TinyXML2
       /usr/lib
       /usr/local/lib
       /opt/lib
       /opt/local/lib )
endif()

# macro used to find specified TINYXML library
macro( FIND_TINYXML2_LIBRARY NAME )
  find_library( TINYXML2_${NAME}_LIBRARY
                NAMES ${NAME}
                PATHS ${TINYXML2_LIBRARIES_DIR} )
  find_library( TINYXML2_${NAME}d_LIBRARY
                NAMES ${NAME}d
                PATHS ${TINYXML2_LIBRARIES_DIR} )


  if( TINYXML2_${NAME}_LIBRARY AND TINYXML2_${NAME}d_LIBRARY )

      get_filename_component(lib_name ${TINYXML2_${NAME}_LIBRARY} NAME)
      get_filename_component(lib_named ${TINYXML2_${NAME}d_LIBRARY} NAME)

    set( TINYXML2_LIBRARIES ${TINYXML2_LIBRARIES} optimized ${lib_name} debug ${lib_named} )
  else()
    if( TINYXML2_${NAME}_LIBRARY )
        get_filename_component(lib_name ${TINYXML2_${NAME}_LIBRARY} NAME)

      SET( TINYXML2_LIBRARIES ${TINYXML2_LIBRARIES} general ${lib_name} )
    else()
      if( TINYXML2_${NAME}d_LIBRARY )
          get_filename_component(lib_named ${TINYXML2_${NAME}d_LIBRARY} NAME)

        set( TINYXML2_LIBRARIES ${TINYXML2_LIBRARIES} ${lib_named} ) #??
      endif()
    endif()
  endif()
endmacro()

# Try to find TINYXML include directory
find_path( TINYXML2_LIBRARY_DIRS NAMES "libtinyxml2.dylib" PATHS ${TINYXML2_DIR_SEARCH} )


# Assume we didn't find it
set( TINYXML2_FOUND "FALSE" )
set( TinyXML2_FOUND "FALSE" )

set(TINYXML2_INCLUDE_DIRS "")
set(TinyXML2_INCLUDE_DIRS "")

if(NOT "${TINYXML2_LIBRARY_DIRS}" STREQUAL "")
    set(TINYXML2_LIBRARY_DIRS "${TINYXML2_LIBRARY_DIRS}")
endif()

# Found TINYXML libraries
set( TINYXML2_LIBRARIES "" )

# Find all required TINYXML libraries
FIND_TINYXML2_LIBRARY(tinyxml2)

set(TinyXML2_LIBRARIES ${TINYXML2_LIBRARIES})
set(TinyXML2_LIBRARY_DIRS ${TINYXML2_LIBRARY_DIRS})

#find_path( TINYXML2_INCLUDE_DIRS 
#	   NAMES tinyxml2.h 
#           PATHS ${TINYXML2_DIR_SEARCH} )

#message(warning " ${TINYXML2_INCLUDE_DIRS}")

if( TINYXML2_LIBRARIES )
    set( TINYXML2_FOUND "TRUE" )
    set( TinyXML2_FOUND "TRUE" )
    set(TINYXML2_LIBRARIES "${TINYXML2_LIBRARIES}")
endif()


#ask user to help us out..
if(NOT TINYXML2_FOUND)
  set(TINYXML2_ROOT_DIR "" CACHE PATH "")
endif()

mark_as_advanced( TINYXML2_INCLUDE_DIRS
                  TINYXML2_LIBRARY_DIRS
                  TINYXML2_tinyxml2_LIBRARY
                  TINYXML2_tinyxml2d_LIBRARY
                  )

