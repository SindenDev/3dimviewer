# CMake module that tries to find MDSTk includes and libraries.
#
# The following variables are set if MDSTk is found:
#   VPL_FOUND         - True when the MDSTk include directory is found.
#   VPL_INCLUDE_DIR   - The directory where MDSTk include files are.
#   VPL_LIBRARIES_DIR - The directory where MDSTk libraries are.
#   VPL_LIBRARIES     - List of all MDSTk libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(VPL)
#   ...
#   INCLUDE_DIRECTORIES(${VPL_INCLUDE_DIR})
#   LINK_DIRECTORIES(${VPL_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${VPL_LIBRARIES} )


# Initialize the search path
if( WIN32 )
  set( VPL_DIR_SEARCH
       "${TRIDIM_3RDPARTY_DIR}"
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/VPL"
       "${VPL_ROOT_DIR}"
       "$ENV{VPL_ROOT_DIR}"
       "$ENV{ProgramFiles}/VPL"
       C:/VPL
       D:/VPL
       )
  set( VPL_SEARCHSUFFIXES
       include
       )
else( WIN32 )
  set( VPL_DIR_SEARCH
       "${TRIDIM_MOOQ_DIR}"
       "${TRIDIM_MOOQ_DIR}/VPL"
       "${VPL_ROOT_DIR}"
       "$ENV{VPL_ROOT_DIR}"
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/VPL/include )
  set( VPL_SEARCHSUFFIXES
       ""
       )
endif( WIN32 )


# Try to find VPL include directory
find_path( VPL_INCLUDE_DIR
           NAMES
           VPL/configure.h
           PATHS
           ${VPL_DIR_SEARCH}
           PATH_SUFFIXES
           ${VPL_SEARCHSUFFIXES}
           )


# Assume we didn't find it
set( VPL_FOUND NO )

# Now try to get the library path
IF( VPL_INCLUDE_DIR )

  # Look for the VPL library path
  SET( VPL_LIBRARIES_DIR ${VPL_INCLUDE_DIR} )

  # Strip off the trailing "/include" in the path
  IF( "${VPL_LIBRARIES_DIR}" MATCHES "/include$" )
    GET_FILENAME_COMPONENT( VPL_LIBRARIES_DIR ${VPL_LIBRARIES_DIR} PATH )
  ENDIF( "${VPL_LIBRARIES_DIR}" MATCHES "/include$" )

  # Check if the 'lib' directory exists
  IF( EXISTS "${VPL_LIBRARIES_DIR}/lib" )
    SET( VPL_LIBRARIES_DIR ${VPL_LIBRARIES_DIR}/lib )
  ENDIF( EXISTS "${VPL_LIBRARIES_DIR}/lib" )

  # Check if the 'debug' and 'release' directory exists
  IF( EXISTS "${VPL_LIBRARIES_DIR}/debug" )
    SET( VPL_LIBRARIES_DIR_DEBUG ${VPL_LIBRARIES_DIR}/debug )
  ENDIF( EXISTS "${VPL_LIBRARIES_DIR}/debug" )
  IF( EXISTS "${VPL_LIBRARIES_DIR}/release" )
    SET( VPL_LIBRARIES_DIR_RELEASE ${VPL_LIBRARIES_DIR}/release )
  ENDIF( EXISTS "${VPL_LIBRARIES_DIR}/release" )

  # Add unversioned image lib name to the testing list
  list(APPEND VPL_IMAGE_LIB_NAMES vplImage)
  list(APPEND VPL_IMAGE_LIB_DEBUG_NAMES vplImaged)
  
  set(_VPL_VERSION_SUFFIXES 1.3)
  
  # If exact or minimal version is needed
  if( VPL_FIND_VERSION MATCHES "^([0-9]+)\\.([0-9]+)$")
      
      set(_VPL_VERSION_SUFFIX_MIN "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}") # minimal suffix used 
      
      if(VPL_FIND_VERSION_EXACT)
        # if exact version - use pre parsed suffix as only searched
        set(_VPL_VERSION_SUFFIXES ${_VPL_VERSION_SUFFIX_MIN}) 
      else()
        # Not exact search - replace insufficient versions by sufficient 
        string(REGEX REPLACE
                  "${_VPL_VERSION_SUFFIX_MIN}.*" "${_VPL_VERSION_SUFFIX_MIN}"
                  _VPL_VERSION_SUFFIXES
                  "${VPL_VERSION_SUFFIXES}"
              )
      endif(VPL_FIND_VERSION_EXACT)
      
      unset(_VPL_VERSION_SUFFIX_MIN)
        
  endif()
  
#  message(${_VPL_VERSION_SUFFIXES})
  
  # Generate lists of possible names
  foreach(v IN LISTS _VPL_VERSION_SUFFIXES)
    if( v MATCHES "^([0-9]+)\\.([0-9]+)$")
        set(LIB_SUFFIX "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
        list(APPEND VPL_IMAGE_LIB_NAMES vplImage${LIB_SUFFIX})
        list(APPEND VPL_IMAGE_LIB_DEBUG_NAMES vplImage${LIB_SUFFIX}d)
        unset(LIB_SUFFIX)
    endif()
  endforeach()
  
#  message(${VPL_IMAGE_LIB_NAMES})
#  message(${VPL_IMAGE_LIB_DEBUG_NAMES})
  
  unset(_VPL_VERSION_SUFFIXES)  
  

  # Find release libraries
  FIND_LIBRARY( VPL_Image_LIBRARY
                NAMES ${VPL_IMAGE_LIB_NAMES}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )

  # Try to find debug libraries
  FIND_LIBRARY( VPL_Imaged_LIBRARY
                NAMES ${VPL_IMAGE_LIB_DEBUG_NAMES}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
  
  unset(VPL_IMAGE_LIB_NAMES)
  unset(VPL_IMAGE_LIB_NAMES_DEBUG)
  
#  message(${VPL_Image_LIBRARY} ";" ${VPL_Imaged_LIBRARY})              

  # Parse used suffix
  if(VPL_Image_LIBRARY  MATCHES "([0-9]+)\\.[a-z]*$")
    set(VPL_LIBRARY_SUFFIX "${CMAKE_MATCH_1}")
#    message("Suffix: " ${VPL_LIBRARY_SUFFIX})
  else()
      if(VPL_Imaged_LIBRARY  MATCHES "([0-9]+)d\\.[a-z]*$")
          set(VPL_LIBRARY_SUFFIX "${CMAKE_MATCH_1}")
#          message("Suffix: " ${VPL_LIBRARY_SUFFIX})
      endif()          
  endif()  
  
  # Try to find VE libraries
  FIND_LIBRARY( VE_LIBRARY
                NAMES vplVectorEntity${VPL_LIBRARY_SUFFIX}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
  FIND_LIBRARY( VE_LIBRARY_DEBUG
                NAMES vplVectorEntity${VPL_LIBRARY_SUFFIX}d
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
                
  # Check what libraries was found
  IF( VPL_Image_LIBRARY AND VPL_Imaged_LIBRARY )
    SET( VPL_LIBRARIES
         optimized vplImageIO${VPL_LIBRARY_SUFFIX}
         optimized vplImage${VPL_LIBRARY_SUFFIX}
         optimized vplTinyXML${VPL_LIBRARY_SUFFIX}
         optimized vplModule${VPL_LIBRARY_SUFFIX}
         optimized vplSystem${VPL_LIBRARY_SUFFIX}
         optimized vplMath${VPL_LIBRARY_SUFFIX}
         optimized vplBase${VPL_LIBRARY_SUFFIX}
         debug vplImageIO${VPL_LIBRARY_SUFFIX}d
         debug vplImage${VPL_LIBRARY_SUFFIX}d
         debug vplTinyXML${VPL_LIBRARY_SUFFIX}d
         debug vplModule${VPL_LIBRARY_SUFFIX}d
         debug vplSystem${VPL_LIBRARY_SUFFIX}d
         debug vplMath${VPL_LIBRARY_SUFFIX}d
         debug vplBase${VPL_LIBRARY_SUFFIX}d )
    SET( VPL_FOUND "YES" )
  ELSE( VPL_Image_LIBRARY AND VPL_Imaged_LIBRARY )
    IF( VPL_Image_LIBRARY )
      SET( VPL_LIBRARIES vplImageIO${VPL_LIBRARY_SUFFIX} vplImage${VPL_LIBRARY_SUFFIX} vplModule${VPL_LIBRARY_SUFFIX} vplSystem${VPL_LIBRARY_SUFFIX} vplMath${VPL_LIBRARY_SUFFIX} vplBase${VPL_LIBRARY_SUFFIX} vplTinyXML${VPL_LIBRARY_SUFFIX} )
      SET( VPL_FOUND "YES" )
    ELSE( VPL_Image_LIBRARY )
      IF( VPL_Imaged_LIBRARY )
        SET( VPL_LIBRARIES vplImageIO${VPL_LIBRARY_SUFFIX}d vplImage${VPL_LIBRARY_SUFFIX}d vplModule${VPL_LIBRARY_SUFFIX}d vplSystemd vplMath${VPL_LIBRARY_SUFFIX}d vplBase${VPL_LIBRARY_SUFFIX}d vplTinyXML${VPL_LIBRARY_SUFFIX}d )
        SET( VPL_FOUND "YES" )
      ENDIF( VPL_Imaged_LIBRARY )
    ENDIF( VPL_Image_LIBRARY )
  ENDIF( VPL_Image_LIBRARY AND VPL_Imaged_LIBRARY )

  if( VE_LIBRARY AND VE_LIBRARY_DEBUG )
    SET( VPL_LIBRARIES ${VPL_LIBRARIES}
         optimized ${VE_LIBRARY}
         debug ${VE_LIBRARY_DEBUG} )
  else( VE_LIBRARY AND VE_LIBRARY_DEBUG )
    if( VE_LIBRARY )
      SET( VPL_LIBRARIES ${VPL_LIBRARIES} ${VE_LIBRARY} )
    else( VE_LIBRARY )
      if( VE_LIBRARY_DEBUG )
        SET( VPL_LIBRARIES ${VPL_LIBRARIES} ${VE_LIBRARY_DEBUG} )
      endif( VE_LIBRARY_DEBUG )
    endif( VE_LIBRARY ) 
  endif( VE_LIBRARY AND VE_LIBRARY_DEBUG )

ENDIF( VPL_INCLUDE_DIR )

# Using shared or static library?
IF( VPL_FOUND )
#    OPTION( VPL_USE_SHARED_LIBRARY "Should shared library be used?" OFF )
#    IF( VPL_USE_SHARED_LIBRARY )
#      ADD_DEFINITIONS( -DMDS_LIBRARY_SHARED )
#    ELSE( VPL_USE_SHARED_LIBRARY )
      ADD_DEFINITIONS( -DMDS_LIBRARY_STATIC )
#    ENDIF( VPL_USE_SHARED_LIBRARY )
    if( APPLE )
        add_definitions( -D_MACOSX )
    else()
        if( UNIX )
            add_definitions( -D_LINUX )
        endif()
    endif()
ENDIF( VPL_FOUND )

MARK_AS_ADVANCED( VPL_INCLUDE_DIR
                  VPL_Image_LIBRARY
                  VPL_Imaged_LIBRARY )


# display help message
IF( NOT VPL_FOUND )
  # make FIND_PACKAGE friendly
  IF( NOT VPL_FIND_QUIETLY )
    IF( VPL_FIND_REQUIRED )
      MESSAGE( FATAL_ERROR "VPL required but some headers or libs not found. Please specify it's location with VPL_ROOT_DIR variable.")
      SET( VPL_ROOT_DIR "" CACHE PATH "VPL root dir" )
    ELSE( VPL_FIND_REQUIRED )
      MESSAGE(STATUS "ERROR: VPL was not found.")
    ENDIF( VPL_FIND_REQUIRED )
  ENDIF( NOT VPL_FIND_QUIETLY )
ENDIF( NOT VPL_FOUND )

