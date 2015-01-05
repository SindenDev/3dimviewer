# CMake module that tries to find MDSTk includes and libraries.
#
# The following variables are set if MDSTk is found:
#   MDSTk_FOUND         - True when the MDSTk include directory is found.
#   MDSTk_INCLUDE_DIR   - The directory where MDSTk include files are.
#   MDSTk_LIBRARIES_DIR - The directory where MDSTk libraries are.
#   MDSTk_LIBRARIES     - List of all MDSTk libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(MDSTk)
#   ...
#   INCLUDE_DIRECTORIES(${MDSTk_INCLUDE_DIR})
#   LINK_DIRECTORIES(${MDSTk_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${MDSTk_LIBRARIES} )


# Initialize the search path
if( WIN32 )
  set( MDSTk_DIR_SEARCH
       "${TRIDIM_3RDPARTY_DIR}"
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/MDSTk"
       "${MDSTk_ROOT_DIR}"
       "$ENV{MDSTk_ROOT_DIR}"
       "$ENV{ProgramFiles}/MDSTk"
       C:/MDSTk
       D:/MDSTk
       )
  set( MDSTk_SEARCHSUFFIXES
       include
       )
else( WIN32 )
  set( MDSTk_DIR_SEARCH
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/MDSTk"
       "${MDSTk_ROOT_DIR}"
       "$ENV{MDSTk_ROOT_DIR}"
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/MDSTk/include )
  set( MDSTk_SEARCHSUFFIXES
       ""
       )
endif( WIN32 )


# Try to find MDSTk include directory
find_path( MDSTk_INCLUDE_DIR
           NAMES
           MDSTk/configure.h
           PATHS
           ${MDSTk_DIR_SEARCH}
           PATH_SUFFIXES
           ${MDSTk_SEARCHSUFFIXES}
           )


# Assume we didn't find it
set( MDSTk_FOUND NO )

# Now try to get the library path
IF( MDSTk_INCLUDE_DIR )

  # Look for the MDSTk library path
  SET( MDSTk_LIBRARIES_DIR ${MDSTk_INCLUDE_DIR} )

  # Strip off the trailing "/include" in the path
  IF( "${MDSTk_LIBRARIES_DIR}" MATCHES "/include$" )
    GET_FILENAME_COMPONENT( MDSTk_LIBRARIES_DIR ${MDSTk_LIBRARIES_DIR} PATH )
  ENDIF( "${MDSTk_LIBRARIES_DIR}" MATCHES "/include$" )

  # Check if the 'lib' directory exists
  IF( EXISTS "${MDSTk_LIBRARIES_DIR}/lib" )
    SET( MDSTk_LIBRARIES_DIR ${MDSTk_LIBRARIES_DIR}/lib )
  ENDIF( EXISTS "${MDSTk_LIBRARIES_DIR}/lib" )

  # Check if the 'debug' and 'release' directory exists
  IF( EXISTS "${MDSTk_LIBRARIES_DIR}/debug" )
    SET( MDSTk_LIBRARIES_DIR_DEBUG ${MDSTk_LIBRARIES_DIR}/debug )
  ENDIF( EXISTS "${MDSTk_LIBRARIES_DIR}/debug" )
  IF( EXISTS "${MDSTk_LIBRARIES_DIR}/release" )
    SET( MDSTk_LIBRARIES_DIR_RELEASE ${MDSTk_LIBRARIES_DIR}/release )
  ENDIF( EXISTS "${MDSTk_LIBRARIES_DIR}/release" )


  # Find release libraries
  FIND_LIBRARY( MDSTk_Image_LIBRARY
                NAMES mdsImage
                PATHS ${MDSTk_LIBRARIES_DIR} ${MDSTk_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )

  # Try to find debug libraries
  FIND_LIBRARY( MDSTk_Imaged_LIBRARY
                NAMES mdsImaged
                PATHS ${MDSTk_LIBRARIES_DIR} ${MDSTk_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
  # Try to find VE libraries
  FIND_LIBRARY( VE_LIBRARY
                NAMES mdsVectorEntity
                PATHS ${MDSTk_LIBRARIES_DIR} ${MDSTk_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
  FIND_LIBRARY( VE_LIBRARY_DEBUG
                NAMES mdsVectorEntityd
                PATHS ${MDSTk_LIBRARIES_DIR} ${MDSTk_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
                
  # Check what libraries was found
  IF( MDSTk_Image_LIBRARY AND MDSTk_Imaged_LIBRARY )
    SET( MDSTk_LIBRARIES
         optimized mdsImageIO
         optimized mdsImage
         optimized mdsTinyXML
         optimized mdsModule
         optimized mdsSystem
         optimized mdsMath
         optimized mdsBase
         debug mdsImageIOd
         debug mdsImaged
         debug mdsTinyXMLd
         debug mdsModuled
         debug mdsSystemd
         debug mdsMathd
         debug mdsBased )
    SET( MDSTk_FOUND "YES" )
  ELSE( MDSTk_Image_LIBRARY AND MDSTk_Imaged_LIBRARY )
    IF( MDSTk_Image_LIBRARY )
      SET( MDSTk_LIBRARIES mdsImageIO mdsImage mdsModule mdsSystem mdsMath mdsBase mdsTinyXML )
      SET( MDSTk_FOUND "YES" )
    ELSE( MDSTk_Image_LIBRARY )
      IF( MDSTk_Imaged_LIBRARY )
        SET( MDSTk_LIBRARIES mdsImageIOd mdsImaged mdsModuled mdsSystemd mdsMathd mdsBased mdsTinyXMLd )
        SET( MDSTk_FOUND "YES" )
      ENDIF( MDSTk_Imaged_LIBRARY )
    ENDIF( MDSTk_Image_LIBRARY )
  ENDIF( MDSTk_Image_LIBRARY AND MDSTk_Imaged_LIBRARY )

  if( VE_LIBRARY AND VE_LIBRARY_DEBUG )
    SET( MDSTk_LIBRARIES ${MDSTk_LIBRARIES}
         optimized ${VE_LIBRARY}
         debug ${VE_LIBRARY_DEBUG} )
  else( VE_LIBRARY AND VE_LIBRARY_DEBUG )
    if( VE_LIBRARY )
      SET( MDSTk_LIBRARIES ${MDSTk_LIBRARIES} ${VE_LIBRARY} )
    else( VE_LIBRARY )
      if( VE_LIBRARY_DEBUG )
        SET( MDSTk_LIBRARIES ${MDSTk_LIBRARIES} ${VE_LIBRARY_DEBUG} )
      endif( VE_LIBRARY_DEBUG )
    endif( VE_LIBRARY ) 
  endif( VE_LIBRARY AND VE_LIBRARY_DEBUG )

ENDIF( MDSTk_INCLUDE_DIR )

# Using shared or static library?
IF( MDSTk_FOUND )
#    OPTION( MDSTk_USE_SHARED_LIBRARY "Should shared library be used?" OFF )
#    IF( MDSTk_USE_SHARED_LIBRARY )
#      ADD_DEFINITIONS( -DMDS_LIBRARY_SHARED )
#    ELSE( MDSTk_USE_SHARED_LIBRARY )
      ADD_DEFINITIONS( -DMDS_LIBRARY_STATIC )
#    ENDIF( MDSTk_USE_SHARED_LIBRARY )
    if( APPLE )
        add_definitions( -D_MACOSX )
    else()
        if( UNIX )
            add_definitions( -D_LINUX )
        endif()
    endif()
ENDIF( MDSTk_FOUND )

MARK_AS_ADVANCED( MDSTk_INCLUDE_DIR
                  MDSTk_Image_LIBRARY
                  MDSTk_Imaged_LIBRARY )


# display help message
IF( NOT MDSTk_FOUND )
  # make FIND_PACKAGE friendly
  IF( NOT MDSTk_FIND_QUIETLY )
    IF( MDSTk_FIND_REQUIRED )
      MESSAGE( FATAL_ERROR "MDSTk required but some headers or libs not found. Please specify it's location with MDSTk_ROOT_DIR variable.")
      SET( MDSTk_ROOT_DIR "" CACHE PATH "MDSTk root dir" )
    ELSE( MDSTk_FIND_REQUIRED )
      MESSAGE(STATUS "ERROR: MDSTk was not found.")
    ENDIF( MDSTk_FIND_REQUIRED )
  ENDIF( NOT MDSTk_FIND_QUIETLY )
ENDIF( NOT MDSTk_FOUND )

