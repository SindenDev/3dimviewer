# - Find zlib
# Find the native ZLIB includes and library
#
#  ZLIB_INCLUDE_DIRSS   - where to find zlib.h, etc.
#  ZLIB_LIBRARIES      - List of libraries when using zlib.
#  ZLIB_FOUND          - True if zlib found.
#
#  ZLIB_VERSION_STRING - The version of zlib found (x.y.z)
#  ZLIB_MAJOR_VERSION  - the major version of zlib
#  ZLIB_MINOR_VERSION  - The minor version of zlib
#  ZLIB_PATCH_VERSION  - The patch version of zlib

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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
# Modified for 3DimViewer by Michal Spanel
#


if( NOT WIN32 )
  set( ZLIB_DIR_SEARCH
       /usr
       /usr/local
       /opt
       /opt/local
       )
endif()

find_path(ZLIB_INCLUDE_DIRS
    NAMES zlib.h
    PATHS
    ${ZLIB_DIR_SEARCH}
    PATH_SUFFIXES "include"
)

set(ZLIB_NAMES z zlib zdll)

find_library(ZLIB_LIBRARY
    NAMES
        ${ZLIB_NAMES}
    PATHS
        ${ZLIB_DIR_SEARCH}
        PATH_SUFFIXES "lib"
)

get_filename_component( ZLIB_LIBRARY_DIRS ${ZLIB_LIBRARY} PATH )
get_filename_component( ZLIB_LIBRARIES ${ZLIB_LIBRARY} NAME )


mark_as_advanced(ZLIB_LIBRARY ZLIB_INCLUDE_DIRS)



if (ZLIB_INCLUDE_DIRS AND EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
    file(READ "${ZLIB_INCLUDE_DIRS}/zlib.h" ZLIB_H)
    string(REGEX REPLACE ".*#define ZLIB_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*" "\\1.\\2.\\3" ZLIB_VERSION_STRING "${ZLIB_H}")
endif()


if (ZLIB_INCLUDE_DIRS AND ZLIB_LIBRARIES)

    set(ZLIB_FOUND TRUE)
    set(Zlib_FOUND TRUE)

    set(ZLIB_LIBRARIES general ${ZLIB_LIBRARIES})

    set(Zlib_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
    set(Zlib_LIBRARY_DIRS ${ZLIB_LIBRARY_DIRS})
    set(Zlib_LIBRARIES    ${ZLIB_LIBRARIES}  )

endif()

