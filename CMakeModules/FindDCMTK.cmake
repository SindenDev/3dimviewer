# - find DCMTK libraries and applications
#

#  DCMTK_INCLUDE_DIRS   - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source (default to /usr/include/dcmtk/)

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2009-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
# Copyright 2010 Thomas Sondergaard <ts@medical-insight.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

#
# Written for VXL by Amitha Perera.
# Upgraded for GDCM by Mathieu Malaterre.
# Modified for EasyViz by Thomas Sondergaard.
# Modified for 3DimViewer by Vit Stancl and Michal Spanel
#

#if(NOT DCMTK_FOUND AND NOT DCMTK_DIR)
#  set( DCMTK_DIR
#       "${TRIDIM_3RDPARTY_DIR}"
#       CACHE
#       PATH
#       "Root of DCMTK source tree (optional)."
#       )
#  mark_as_advanced(DCMTK_DIR)
#endif()

if( NOT WIN32 )

  set( DCMTK_INCLUDE_DIR_SEARCH
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       ~/DCMTK/include )

endif()



find_path( DCMTK_INCLUDE_DIR
           NAMES
           dcmtk DCMTK DCMTk
           PATHS
           ${DCMTK_INCLUDE_DIR_SEARCH}
           )

       mark_as_advanced(DCMTK_INCLUDE_DIR)

# Strip off the trailing "/include" in the path
if( "${DCMTK_INCLUDE_DIR}" MATCHES "/include$" )
    get_filename_component( DCMTK_ROOT_DIR ${DCMTK_INCLUDE_DIR} PATH )
endif()

foreach(lib
    dcmdata
    dcmimage
    dcmimgle
    dcmjpeg
    dcmjpls
    dcmnet
    dcmpstat
    dcmrt
    dcmqrdb
    dcmdsig
    dcmsr
    dcmtls
    dcmwlm
    i2d
    ijg12
    ijg16
    ijg8
    oflog
    ofstd)

  # Debug libraries
  find_library(DCMTK_${lib}d_LIBRARY
    ${lib}d
    PATHS
    ${DCMTK_ROOT_DIR}/${lib}/libsrc
    ${DCMTK_ROOT_DIR}/${lib}/libsrc/Release
    ${DCMTK_ROOT_DIR}/${lib}/libsrc/Debug
    ${DCMTK_ROOT_DIR}/${lib}/Release
    ${DCMTK_ROOT_DIR}/${lib}/Debug
    ${DCMTK_ROOT_DIR}/lib
    )

  mark_as_advanced(DCMTK_${lib}d_LIBRARY)

  if(DCMTK_${lib}d_LIBRARY)
    list(APPEND DCMTK_LIBRARIES debug ${DCMTK_${lib}d_LIBRARY})
  endif()

  # Optimized libraries
  find_library(DCMTK_${lib}_LIBRARY
    ${lib}
    PATHS
    ${DCMTK_ROOT_DIR}/${lib}/libsrc
    ${DCMTK_ROOT_DIR}/${lib}/libsrc/Release
    ${DCMTK_ROOT_DIR}/${lib}/libsrc/Debug
    ${DCMTK_ROOT_DIR}/${lib}/Release
    ${DCMTK_ROOT_DIR}/${lib}/Debug
    ${DCMTK_ROOT_DIR}/lib
    )

  mark_as_advanced(DCMTK_${lib}_LIBRARY)

  if(DCMTK_${lib}_LIBRARY)
      get_filename_component(lib_name ${DCMTK_${lib}_LIBRARY} NAME)

      if(NOT DCMTK_LIBRARY_DIRS)
          get_filename_component(DCMTK_LIBRARY_DIRS ${DCMTK_${lib}_LIBRARY} PATH)
      endif()
    if(DCMTK_${lib}d_LIBRARY)
        get_filename_component(lib_named ${DCMTK_${lib}d_LIBRARY} NAME)

      list(APPEND DCMTK_LIBRARIES optimized ${lib_name} debug ${lib_named})
    else()
      list(APPEND DCMTK_LIBRARIES general ${lib_name})
    endif()
  endif()

endforeach()


set(DCMTK_config_TEST_HEADER osconfig.h)
set(DCMTK_dcmdata_TEST_HEADER dctypes.h)
set(DCMTK_dcmimage_TEST_HEADER dicoimg.h)
set(DCMTK_dcmimgle_TEST_HEADER dcmimage.h)
set(DCMTK_dcmjpeg_TEST_HEADER djdecode.h)
set(DCMTK_dcmjpls_TEST_HEADER djencode.h)
set(DCMTK_dcmnet_TEST_HEADER assoc.h)
set(DCMTK_dcmpstat_TEST_HEADER dcmpstat.h)
set(DCMTK_dcmqrdb_TEST_HEADER dcmqrdba.h)
set(DCMTK_dcmrt_TEST_HEADER drttypes.h)
set(DCMTK_dcmsign_TEST_HEADER sicert.h)
set(DCMTK_dcmsr_TEST_HEADER dsrtree.h)
set(DCMTK_dcmtls_TEST_HEADER tlslayer.h)
set(DCMTK_dcmwlm_TEST_HEADER wlds.h)
set(DCMTK_oflog_TEST_HEADER logger.h)
set(DCMTK_ofstd_TEST_HEADER ofstdinc.h)

foreach(dir
    config
    dcmdata
    dcmimage
    dcmimgle
    dcmjpeg
    dcmjpls
    dcmnet
    dcmpstat
    dcmqrdb
    dcmrt
    dcmsign
    dcmsr
    dcmtls
    dcmwlm
    oflog
    ofstd)

  find_path(DCMTK_${dir}_INCLUDE_DIR
    ${DCMTK_${dir}_TEST_HEADER}
    PATHS

    ${DCMTK_INCLUDE_DIR}/${dir}/include
    ${DCMTK_INCLUDE_DIR}/include/dcmtk/${dir}
    ${DCMTK_INCLUDE_DIR}/${dir}
    ${DCMTK_INCLUDE_DIR}/dcmtk/${dir}
    ${DCMTK_INCLUDE_DIR}/include/${dir}
    )
  mark_as_advanced(DCMTK_${dir}_INCLUDE_DIR)

  if(DCMTK_${dir}_INCLUDE_DIR)
    list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_${dir}_INCLUDE_DIR})
  endif()
endforeach()

if(WIN32)
  list(APPEND DCMTK_LIBRARIES netapi32 wsock32)
  list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_ROOT_DIR}/include)
endif()

if(DCMTK_ofstd_INCLUDE_DIR)
  get_filename_component(DCMTK_dcmtk_INCLUDE_DIR
    ${DCMTK_ofstd_INCLUDE_DIR}
    PATH
    CACHE)
  list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_dcmtk_INCLUDE_DIR})
  mark_as_advanced(DCMTK_dcmtk_INCLUDE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCMTK DEFAULT_MSG
  DCMTK_config_INCLUDE_DIR
  DCMTK_ofstd_INCLUDE_DIR
  DCMTK_ofstd_LIBRARY
  DCMTK_dcmdata_INCLUDE_DIR
  DCMTK_dcmdata_LIBRARY
  DCMTK_dcmimgle_INCLUDE_DIR
  DCMTK_dcmimgle_LIBRARY)

#message(${DCMTK_LIBRARIES})
# Compatibility: This variable is deprecated
#set(DCMTK_INCLUDE_DIR ${DCMTK_INCLUDE_DIRS})

foreach(executable dcmdump dcmdjpeg dcmdrle)
  string(TOUPPER ${executable} EXECUTABLE)
  find_program(DCMTK_${EXECUTABLE}_EXECUTABLE ${executable} ${DCMTK_LIB_DIR}/bin)
  mark_as_advanced(DCMTK_${EXECUTABLE}_EXECUTABLE)
endforeach()

