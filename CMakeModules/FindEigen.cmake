# - Try to find Eigen3 lib
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(Eigen3 3.1.2)
# to require version 3.1.2 or newer of Eigen3.
#
# Once done this will define
#
#  EIGEN_FOUND - system has eigen lib with correct version
#  EIGEN3_INCLUDE_DIR - the eigen include directory
#  EIGEN3_VERSION - eigen version
#
# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Copyright (c) 2008, 2009 Gael Guennebaud, <g.gael@free.fr>
# Copyright (c) 2009 Benoit Jacob <jacob.benoit.1@gmail.com>
# Redistribution and use is allowed according to the terms of the 2-clause BSD license.
#
# 2014/08 modified by Michal Spanel for VPL 

# Predefined search directories
if( NOT WIN32 )

  set( EIGEN_INC_SEARCHPATH
       "${EIGEN_ROOT_DIR}"
       "$ENV{EIGEN_ROOT_DIR}"
       "${INCLUDE_INSTALL_DIR}"
       "${VPL_3RDPARTY_DIR}/include"
       "${CMAKE_INSTALL_PREFIX}/include"
       "${KDE4_INCLUDE_DIR}"
       /sw/include
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       )
  set( EIGEN_INC_SEARCHSUFFIXES
       eigen3
       eigen
       )
endif()

if(NOT Eigen3_FIND_VERSION)

  if(NOT Eigen3_FIND_VERSION_MAJOR)
    set(Eigen3_FIND_VERSION_MAJOR 2)
  endif()

  if(NOT Eigen3_FIND_VERSION_MINOR)
    set(Eigen3_FIND_VERSION_MINOR 91)
  endif()

  if(NOT Eigen3_FIND_VERSION_PATCH)
    set(Eigen3_FIND_VERSION_PATCH 0)
  endif()

  set(Eigen3_FIND_VERSION "${Eigen3_FIND_VERSION_MAJOR}.${Eigen3_FIND_VERSION_MINOR}.${Eigen3_FIND_VERSION_PATCH}")
endif()

macro(_eigen3_check_version)
  file(READ "${EIGEN_INCLUDE_DIRS}/Eigen/src/Core/util/Macros.h" _eigen3_version_header)

  string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen3_world_version_match "${_eigen3_version_header}")
  set(EIGEN3_WORLD_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen3_major_version_match "${_eigen3_version_header}")
  set(EIGEN3_MAJOR_VERSION "${CMAKE_MATCH_1}")
  string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen3_minor_version_match "${_eigen3_version_header}")
  set(EIGEN3_MINOR_VERSION "${CMAKE_MATCH_1}")

  set(EIGEN3_VERSION ${EIGEN3_WORLD_VERSION}.${EIGEN3_MAJOR_VERSION}.${EIGEN3_MINOR_VERSION})

  if(${EIGEN3_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})
    set(EIGEN3_VERSION_OK FALSE)
  else()
    set(EIGEN3_VERSION_OK TRUE)
  endif()

  if(NOT EIGEN3_VERSION_OK)
    message(STATUS "Eigen3 version ${EIGEN3_VERSION} found in ${EIGEN_INCLUDE_DIRS}, "
                   "but at least version ${Eigen3_FIND_VERSION} is required")
  endif()

endmacro()


if (EIGEN_INCLUDE_DIRS)

  # in cache already
  _eigen3_check_version()

  if(${EIGEN3_VERSION_OK})
    set(EIGEN_FOUND TRUE)
    set(Eigen_FOUND TRUE)

    set(Eigen_INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} CACHE PATH "")
  endif()

else ()

  find_path(EIGEN_INCLUDE_DIRS NAMES Eigen
            PATHS 
            ${EIGEN_INC_SEARCHPATH}
            PATH_SUFFIXES 
            ${EIGEN_INC_SEARCHSUFFIXES}
            )

  if (EIGEN_INCLUDE_DIRS)
    _eigen3_check_version()
    if(${EIGEN3_VERSION_OK})
      set(EIGEN_FOUND TRUE)
      set(Eigen_FOUND TRUE)

      set(Eigen_INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS} CACHE PATH "")
    endif()
  endif()

  #include(FindPackageHandleStandardArgs)
  #find_package_handle_standard_args(Eigen3 DEFAULT_MSG EIGEN_INCLUDE_DIRS EIGEN3_VERSION_OK)

endif()

mark_as_advanced(EIGEN_INCLUDE_DIRS Eigen_INCLUDE_DIRS)
