# - Find zlib
# Find the native PNG includes and library
#
#  PNG_INCLUDE_DIRS   - where to find PNG.h, etc.
#  PNG_LIBRARIES      - List of libraries when using PNG.
#  PNG_FOUND          - True if PNG found.
#
#  PNG_VERSION_STRING - The version of PNG found (x.y.z)
#  PNG_MAJOR_VERSION  - the major version of PNG
#  PNG_MINOR_VERSION  - The minor version of PNG
#  PNG_PATCH_VERSION  - The patch version of PNG

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

FIND_PATH(PNG_INCLUDE_DIR
    NAMES png.h
    PATHS
    "${TRIDIM_3RDPARTY_DIR}/include"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\PNG;InstallPath]/include"
)

SET(PNG_NAMES png libpng)
FIND_LIBRARY(PNG_LIBRARY
    NAMES
        ${PNG_NAMES}
    PATHS
        "${TRIDIM_3RDPARTY_DIR}/lib"
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\PNG;InstallPath]/lib"
)
MARK_AS_ADVANCED(PNG_LIBRARY PNG_INCLUDE_DIR)

IF (PNG_INCLUDE_DIR AND EXISTS "${PNG_INCLUDE_DIR}/png.h")
    FILE(READ "${PNG_INCLUDE_DIR}/PNG.h" PNG_H)
    STRING(REGEX REPLACE ".*#define PNG_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*" "\\1.\\2.\\3" PNG_VERSION_STRING "${PNG_H}")
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PNG DEFAULT_MSG PNG_INCLUDE_DIR PNG_LIBRARY)

IF (PNG_FOUND)
    SET(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
    SET(PNG_LIBRARIES    ${PNG_LIBRARY})
    SET(PNG_LINKED_LIBS ${PNG_LIBRARY})
ENDIF()

