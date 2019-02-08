# CMake module that tries to find TinyXML includes and libraries.
#
# The following variables are set if TinyXML is found:
#   TINYXML_FOUND         - True when the TinyXML include directory is found.
#   TINYXML_INCLUDE_DIR   - The directory where TinyXML include files are.
#   TINYXML_LIBRARIES_DIR - The directory where TinyXML libraries are.
#   TINYXML_LIBRARIES     - List of all TinyXML libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(TinyXML)
#   ...
#   INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
#   LINK_DIRECTORIES(${TINYXML_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${TINYXML_LIBRARIES} )
#
# Written for BlueSkyPlan by Vojtech Tryhuk
#

# Initialize the search path
SET( TINYXML_DIR_SEARCH "" )

# User defined CMake cache variable
if(TINYXML_ROOT_DIR)
  set( TINYXML_DIR_SEARCH ${TINYXML_DIR_SEARCH} ${TINYXML_ROOT_DIR} ${TINYXML_ROOT_DIR}/include )
endif()

# Others predefined search directories
if( NOT WIN32 )
  set( TINYXML_DIR_SEARCH
       ${TINYXML_DIR_SEARCH}
       /opt/local/include
       /usr/local/include
       /usr/local/include/TinyXML
       /usr/lib
       /usr/local/lib
       /opt/lib
       /opt/local/lib )
endif()

# macro used to find specified TINYXML library
macro( FIND_TINYXML_LIBRARY NAME )
  find_library( TINYXML_${NAME}_LIBRARY
                NAMES ${NAME}
                PATHS ${TINYXML_LIBRARIES_DIR} )
  find_library( TINYXML_${NAME}d_LIBRARY
                NAMES ${NAME}d
                PATHS ${TINYXML_LIBRARIES_DIR} )


  if( TINYXML_${NAME}_LIBRARY AND TINYXML_${NAME}d_LIBRARY )

      get_filename_component(lib_name ${TINYXML_${NAME}_LIBRARY} NAME)
      get_filename_component(lib_named ${TINYXML_${NAME}d_LIBRARY} NAME)

    set( TINYXML_LIBRARIES ${TINYXML_LIBRARIES} optimized ${lib_name} debug ${lib_named} )
  else()
    if( TINYXML_${NAME}_LIBRARY )
        get_filename_component(lib_name ${TINYXML_${NAME}_LIBRARY} NAME)

      SET( TINYXML_LIBRARIES ${TINYXML_LIBRARIES} general ${lib_name} )
    else()
      if( TINYXML_${NAME}d_LIBRARY )
          get_filename_component(lib_named ${TINYXML_${NAME}d_LIBRARY} NAME)

        set( TINYXML_LIBRARIES ${TINYXML_LIBRARIES} ${lib_named} ) #??
      endif()
    endif()
  endif()
endmacro()

# Try to find TINYXML include directory
find_path( TINYXML_LIBRARY_DIRS NAMES "libtinyxml.dylib" PATHS ${TINYXML_DIR_SEARCH} )


# Assume we didn't find it
set( TINYXML_FOUND "FALSE" )
set( TinyXML_FOUND "FALSE" )

set(TINYXML_INCLUDE_DIRS "")
set(TinyXML_INCLUDE_DIRS "")

if(NOT "${TINYXML_LIBRARY_DIRS}" STREQUAL "")
    set(TINYXML_LIBRARY_DIRS "${TINYXML_LIBRARY_DIRS}")
endif()

# Found TINYXML libraries
set( TINYXML_LIBRARIES "" )

# Find all required TINYXML libraries
FIND_TINYXML_LIBRARY(tinyxml)

set(TinyXML_LIBRARIES ${TINYXML_LIBRARIES})
set(TinyXML_LIBRARY_DIRS ${TINYXML_LIBRARY_DIRS})

#find_path( TINYXML_INCLUDE_DIRS 
#	   NAMES tinyxml.h 
#           PATHS ${TINYXML_DIR_SEARCH} )

#message(warning " ${TINYXML_INCLUDE_DIRS}")

if( TINYXML_LIBRARIES )
    set( TINYXML_FOUND "TRUE" )
    set( TinyXML_FOUND "TRUE" )
    set(TINYXML_LIBRARIES "${TINYXML_LIBRARIES}")
endif()


#ask user to help us out..
if(NOT TINYXML_FOUND)
  set(TINYXML_ROOT_DIR "" CACHE PATH "")
endif()

mark_as_advanced( TINYXML_INCLUDE_DIRS
                  TINYXML_LIBRARY_DIRS
                  TINYXML_tinyxml_LIBRARY
                  TINYXML_tinyxmld_LIBRARY
                  )

