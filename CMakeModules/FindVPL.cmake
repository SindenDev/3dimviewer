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
if( NOT WIN32 )

  set( VPL_DIR_SEARCH
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/VPL/include )
endif()


# Try to find VPL include directory
find_path( VPL_INCLUDE_DIR
           NAMES
           VPL
           PATHS
           ${VPL_DIR_SEARCH}     
           )


# Assume we didn't find it
set( VPL_FOUND FALSE )

# Now try to get the library path
if( VPL_INCLUDE_DIR )

  # Look for the VPL library path
  set( VPL_LIBRARIES_DIR ${VPL_INCLUDE_DIR} )

  # Strip off the trailing "/include" in the path
  if( "${VPL_LIBRARIES_DIR}" MATCHES "/include$" )
    get_filename_component( VPL_LIBRARIES_DIR ${VPL_LIBRARIES_DIR} PATH )
  endif()

  # Check if the 'lib' directory exists
  if( EXISTS "${VPL_LIBRARIES_DIR}/lib" )
    set( VPL_LIBRARIES_DIR ${VPL_LIBRARIES_DIR}/lib )
  endif()

  # Check if the 'debug' and 'release' directory exists
  if( EXISTS "${VPL_LIBRARIES_DIR}/debug" )
    set( VPL_LIBRARIES_DIR_DEBUG ${VPL_LIBRARIES_DIR}/debug )
  endif()

  if( EXISTS "${VPL_LIBRARIES_DIR}/release" )
    set( VPL_LIBRARIES_DIR_RELEASE ${VPL_LIBRARIES_DIR}/release )
  endif()

  # Add unversioned image lib name to the testing list
  list(APPEND VPL_IMAGE_LIB_NAMES vplImage)
  list(APPEND VPL_IMAGE_LIB_DEBUG_NAMES vplImaged)
  
  set(_VPL_VERSION_SUFFIXES 1.3)
  
  # if exact or minimal version is needed
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
      endif()
      
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
  find_library( VPL_Image_LIBRARY
                NAMES ${VPL_IMAGE_LIB_NAMES}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )

  # Try to find debug libraries
  find_library( VPL_Imaged_LIBRARY
                NAMES ${VPL_IMAGE_LIB_DEBUG_NAMES}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
  
  unset(VPL_IMAGE_LIB_NAMES)
  unset(VPL_IMAGE_LIB_NAMES_DEBUG)
  
  #message(${VPL_Image_LIBRARY} ";" ${VPL_Imaged_LIBRARY})

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
  find_library( VE_LIBRARY
                NAMES vplVectorEntity${VPL_LIBRARY_SUFFIX}
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_RELEASE}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
  find_library( VE_LIBRARY_DEBUG
                NAMES vplVectorEntity${VPL_LIBRARY_SUFFIX}d
                PATHS ${VPL_LIBRARIES_DIR} ${VPL_LIBRARIES_DIR_DEBUG}
                NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH
                NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH
                NO_CMAKE_SYSTEM_PATH )
                
                #do it like OSG.. this is not ideal..
  # Check what libraries was found
  if( VPL_Image_LIBRARY AND VPL_Imaged_LIBRARY )
    set( VPL_LIBRARIES
         optimized libvplImageIO${VPL_LIBRARY_SUFFIX}.a
         optimized libvplImage${VPL_LIBRARY_SUFFIX}.a
         optimized libvplModule${VPL_LIBRARY_SUFFIX}.a
         optimized libvplSystem${VPL_LIBRARY_SUFFIX}.a
         optimized libvplMath${VPL_LIBRARY_SUFFIX}.a
         optimized libvplBase${VPL_LIBRARY_SUFFIX}.a
         debug libvplImageIO${VPL_LIBRARY_SUFFIX}d.a
         debug libvplImage${VPL_LIBRARY_SUFFIX}d.a
         debug libvplModule${VPL_LIBRARY_SUFFIX}d.a
         debug libvplSystem${VPL_LIBRARY_SUFFIX}d.a
         debug libvplMath${VPL_LIBRARY_SUFFIX}d.a
         debug libvplBase${VPL_LIBRARY_SUFFIX}d.a )
    set( VPL_FOUND "TRUE" )
  else()
    if( VPL_Image_LIBRARY )
      set( VPL_LIBRARIES vplImageIO${VPL_LIBRARY_SUFFIX} vplImage${VPL_LIBRARY_SUFFIX} vplModule${VPL_LIBRARY_SUFFIX} vplSystem${VPL_LIBRARY_SUFFIX} vplMath${VPL_LIBRARY_SUFFIX} vplBase${VPL_LIBRARY_SUFFIX} )
      set( VPL_FOUND "TRUE" )
    else()
      if( VPL_Imaged_LIBRARY )
        set( VPL_LIBRARIES vplImageIO${VPL_LIBRARY_SUFFIX}d vplImage${VPL_LIBRARY_SUFFIX}d vplModule${VPL_LIBRARY_SUFFIX}d vplSystemd vplMath${VPL_LIBRARY_SUFFIX}d vplBase${VPL_LIBRARY_SUFFIX}d )
        set( VPL_FOUND "TRUE" )
      endif()
    endif()
  endif()

  if( VE_LIBRARY AND VE_LIBRARY_DEBUG )
    set( VPL_LIBRARIES ${VPL_LIBRARIES}
         optimized ${VE_LIBRARY}
         debug ${VE_LIBRARY_DEBUG} )
  else()

    if( VE_LIBRARY )
      set( VPL_LIBRARIES ${VPL_LIBRARIES} ${VE_LIBRARY} )
    else()
      if( VE_LIBRARY_DEBUG )
        set( VPL_LIBRARIES ${VPL_LIBRARIES} ${VE_LIBRARY_DEBUG} )
      endif()
    endif()

  endif()

endif()

# Using shared or static library?
if( VPL_FOUND )

  #  set(VPL_DEFINITIONS "-DMDS_LIBRARY_STATIC")
  #
  #  if( APPLE )
  #      list(APPEND VPL_DEFINITIONS "-D_MACOSX")
  #  else()
  #      if( UNIX )
  #          list(APPEND VPL_DEFINITIONS "-D_LINUX")
  #      endif()
  #  endif()

    set(VPL_INCLUDE_DIRS ${VPL_INCLUDE_DIR})
    set(VPL_LIBRARY_DIRS ${VPL_LIBRARIES_DIR})


endif()

mark_as_advanced( VPL_INCLUDE_DIR
                  VPL_Image_LIBRARY
                  VPL_Imaged_LIBRARY )



