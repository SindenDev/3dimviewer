# - Try to find the OpenSSL encryption library
# Once done this will define
#
#  OPENSSL_ROOT_DIR - set this variable to the root installation of OpenSSL
#
# Read-Only variables:
#  OPENSSL_FOUND - system has the OpenSSL library
#  OPENSSL_INCLUDE_DIR - the OpenSSL include directory
#  OPENSSL_LIBRARIES - The libraries needed to use OpenSSL

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2009-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
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
  set( OPENSSL_DIR_SEARCH
       /usr
       /usr/local
       /opt
       /opt/local
       )
endif()

find_path(OPENSSL_ROOT_DIR NAMES include/openssl/ssl.h PATHS ${OPENSSL_DIR_SEARCH})
mark_as_advanced(OPENSSL_ROOT_DIR)

# Re-use the previous path:
find_path(OPENSSL_INCLUDE_DIRS openssl/ssl.h ${OPENSSL_ROOT_DIR}/include)


find_library(OPENSSL_SSL NAMES ssl PATHS ${OPENSSL_DIR_SEARCH} PATH_SUFFIXES "lib")
find_library(OPENSSL_CRYPTO NAMES crypto PATHS ${OPENSSL_DIR_SEARCH} PATH_SUFFIXES "lib")

mark_as_advanced(OPENSSL_SSL OPENSSL_CRYPTO)

get_filename_component( OPENSSL_LIBRARY_DIRS ${OPENSSL_SSL} PATH )

get_filename_component( OPENSSL_SSL ${OPENSSL_SSL} NAME )
get_filename_component( OPENSSL_CRYPTO ${OPENSSL_CRYPTO} NAME )


set(OPENSSL_LIBRARIES general ${OPENSSL_SSL} general ${OPENSSL_CRYPTO})
set(OpenSSL_LIBRARIES ${OPENSSL_LIBRARIES})


if(OPENSSL_SSL AND OPENSSL_CRYPTO)
    set(OPENSSL_FOUND TRUE)
    set(OpenSSL_FOUND TRUE)

    set(OpenSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIRS})
    set(OpenSSL_LIBRARY_DIRS ${OPENSSL_LIBRARY_DIRS})
    set(OpenSSL_LIBRARIES   ${OPENSSL_LIBRARIES}  )
endif()

mark_as_advanced(OPENSSL_INCLUDE_DIRS OPENSSL_LIBRARIES)

