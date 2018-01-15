#==============================================================================
# This file is part of
#
# VPLswig - SWIG-based VPLswig bindings for Python
# Changes are Copyright 2015 3Dim Laboratory s.r.o.
# All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#==============================================================================
#
# - Find 3DimPython
# Find the 3DimPython includes and library
#
#  3DIMPYTHON_INCLUDE_DIRS   - where to find runtime.h, etc.
#  3DIMPYTHON_LIBRARIES      - List of libraries when using 3DimPython.
#  3DIMPYTHON_FOUND          - True if 3DimPython found.
#
#  3DIMPYTHON_VERSION_STRING - The version of 3DimPython found (x.y.z)
#  3DIMPYTHON_MAJOR_VERSION  - the major version of 3DimPython
#  3DIMPYTHON_MINOR_VERSION  - The minor version of 3DimPython
#  3DIMPYTHON_PATCH_VERSION  - The patch version of 3DimPython



if( NOT WIN32 )
  set( PYTHONCPP_DIR_SEARCH
       /usr
       /usr/local
       /opt
       /opt/local
       )
endif()

find_path(PYTHONCPP_INCLUDE_DIRS
    NAMES runtime.h
    PATHS
    ${PYTHONCPP_DIR_SEARCH}
    PATH_SUFFIXES "include"
)

set(PYTHONCPP_NAMES  libPythonCpp libPythonCppd)

find_library(PYTHONCPP_LIBRARY
    NAMES
        ${PYTHONCPP_NAMES}
    PATHS
        ${PYTHONCPP_DIR_SEARCH}
        PATH_SUFFIXES "lib"
)

get_filename_component( PYTHONCPP_LIBRARY_DIRS ${PYTHONCPP_LIBRARY} PATH )
get_filename_component( PYTHONCPP_LIBRARIES ${PYTHONCPP_LIBRARY} NAME )


mark_as_advanced(PYTHONCPP_LIBRARY PYTHONCPP_INCLUDE_DIRS)


if (PYTHONCPP_INCLUDE_DIRS AND EXISTS "${PYTHONCPP_INCLUDE_DIRS}/Base.h")
    file(READ "${PYTHONCPP_INCLUDE_DIRS}/Base.h" PYTHONCPP_H)
    string(REGEX REPLACE ".*#define PYTHONCPP_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*" "\\1.\\2.\\3" PYTHONCPP_VERSION_STRING "${PYTHONCPP_H}")
endif()


if (PYTHONCPP_INCLUDE_DIRS AND PYTHONCPP_LIBRARIES)

    set(PYTHONCPP_FOUND TRUE)
    set(3DimPython_FOUND TRUE)

    set(PYTHONCPP_LIBRARIES general ${PYTHONCPP_LIBRARIES})

    set(3DimPython_INCLUDE_DIRS ${PYTHONCPP_INCLUDE_DIRS})
    set(3DimPython_LIBRARY_DIRS ${PYTHONCPP_LIBRARY_DIRS})
    set(3DimPython_LIBRARIES    ${PYTHONCPP_LIBRARIES}  )

endif()

