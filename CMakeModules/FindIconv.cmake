# - Try to find Iconv 
# Once done this will define 
# 
#  ICONV_FOUND - system has Iconv 
#  ICONV_INCLUDE_DIRS - the Iconv include directory
#  ICONV_LIBRARIES - Link these to use Iconv 
#  ICONV_SECOND_ARGUMENT_IS_CONST - the second argument for iconv() is const
# 
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

if (ICONV_INCLUDE_DIRS AND ICONV_LIBRARIES)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif (ICONV_INCLUDE_DIRS AND ICONV_LIBRARIES)

if(APPLE)
    find_path(ICONV_INCLUDE_DIRS iconv.h
             PATHS
             /opt/local/include/
             NO_CMAKE_SYSTEM_PATH
    )

    find_library(ICONV_LIBRARIES NAMES iconv libiconv c
             PATHS
             /opt/local/lib/
             NO_CMAKE_SYSTEM_PATH
    )
endif()

find_path(ICONV_INCLUDE_DIRS iconv.h PATHS /opt/local/include /sw/include)

string(REGEX REPLACE "(.*)/include/?" "\\1" ICONV_INCLUDE_BASE_DIR "${ICONV_INCLUDE_DIRS}")

find_library(ICONV_LIBRARIES NAMES iconv libiconv c HINTS "${ICONV_INCLUDE_BASE_DIR}/lib" PATHS /opt/local/lib)
 
if(ICONV_INCLUDE_DIRS AND ICONV_LIBRARIES)
   set(ICONV_FOUND TRUE)
   set(Iconv_FOUND TRUE)
endif()

set(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})
if(ICONV_FOUND)
  check_c_compiler_flag("-Werror" ICONV_HAVE_WERROR)
  set (CMAKE_C_FLAGS_BACKUP "${CMAKE_C_FLAGS}")
  if(ICONV_HAVE_WERROR)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  endif(ICONV_HAVE_WERROR)
  check_c_source_compiles("
  #include <iconv.h>
  int main(){
    iconv_t conv = 0;
    const char* in = 0;
    size_t ilen = 0;
    char* out = 0;
    size_t olen = 0;
    iconv(conv, &in, &ilen, &out, &olen);
    return 0;
  }
" ICONV_SECOND_ARGUMENT_IS_CONST )
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
endif()

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)

if(ICONV_FOUND)
  if(Iconv_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Iconv")
  endif()
endif()

mark_as_advanced(
  ICONV_INCLUDE_DIRS
  ICONV_LIBRARIES
  ICONV_SECOND_ARGUMENT_IS_CONST
)
