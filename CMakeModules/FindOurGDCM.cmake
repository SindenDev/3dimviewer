# CMake module that tries to find GDCM includes and libraries.
#
# The following variables are set if GDCM is found:
#   GDCM_FOUND         - True when the GDCM include directory is found.
#   GDCM_INCLUDE_DIR   - The directory where GDCM include files are.
#   GDCM_LIBRARIES_DIR - The directory where GDCM libraries are.
#   GDCM_LIBRARIES     - List of all GDCM libraries.
#
# Usage:
#   In your CMakeLists.txt file do something like this:
#   ...
#   FIND_PACKAGE(OurGDCM)
#   ...
#   INCLUDE_DIRECTORIES(${GDCM_INCLUDE_DIR})
#   LINK_DIRECTORIES(${GDCM_LIBRARIES_DIR})
#   ...
#   TARGET_LINK_LIBRARIES( mytarget ${GDCM_LIBRARIES} )

# This macro returns all subdirectories (in first parameter "result") from current directory (second parameter "curdir")
MACRO(SUBDIRLIST result curdir)

  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
   IF(IS_DIRECTORY "${curdir}/${child}")
    #look only for vdcm subfolder		
    STRING(REGEX MATCH "gdcm.*" result ${child})
	if(result)
		LIST(APPEND dirlist ${child})
	endif(result)
      
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})

ENDMACRO()

#This macro returns absolute paths in first parameter (result) to library names specified in second parameter (desired_lib_names)
#3rd argument is optional and it specifies configuration usualy “debug” or “optimised”
MACRO(CREATE_LIB_LIST result desired_lib_names)
  
 SET(dirlist "")
 FOREACH(incLib ${desired_lib_names})
    #try to locate library, GDCM_LIB_SEARCHPATH specified before macro use
    FIND_LIBRARY(out
                NAMES 
                ${incLib}
                PATHS ${GDCM_LIB_SEARCHPATH}
                ) 
    IF(out)

      #IF(${configuration} STREQUAL "")
       IF((${ARGV2}))
        list(APPEND dirlist ${out})
      ELSE()
        list(APPEND dirlist "${ARGV2};${out}")
      ENDIF()

    ENDIF(out)

    UNSET(out CACHE)

  ENDFOREACH(incLib)

  SET(${result} ${dirlist})
ENDMACRO()


# Predefined search directories
if( WIN32 )

 #converts backslashes to forwardslashes
 file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" PGM_FILES) 

  set( GDCM_INC_SEARCHPATH
       "${GDCM_ROOT_DIR}"
       "$ENV{GDCM_ROOT_DIR}"
       "${INCLUDE_INSTALL_DIR}"
       "${PGM_FILES}/gdcm"
       C:/gdcm
       D:/gdcm
       )
  set( GDCM_INC_SEARCHSUFFIXES_FIRST_LEVEL
       include
       )
else( WIN32 )
  set( GDCM_INC_SEARCHPATH
       "${GDCM_ROOT_DIR}"
       "$ENV{GDCM_ROOT_DIR}"
       "${INCLUDE_INSTALL_DIR}"
       /sw/include
       /usr/include
       /usr/local/include
       /opt/include
       /opt/local/include
       )
  set( GDCM_INC_SEARCHSUFFIXES_FIRST_LEVEL
	./
       gdcm
       gdcm/include
       include/gdcm
       )
endif( WIN32 )

#find all subdirectories in predefined include directories
#e.g. predefined include direcotory: "C:/gdcm-2.6.3/install/include" => include subdirectory: "C:/gdcm-2.6.3/install/include/gdcm-2.6" 
SET(GDCM_INC_SEARCHSUFFIXES_SECOND_LEVEL "")

find_path( GDCM_INCLUDE_DIR 
           NAMES
           gdcmDefs.h
           PATHS
           ${GDCM_INC_SEARCHPATH}
           PATH_SUFFIXES
           ${GDCM_INC_SEARCHSUFFIXES_FIRST_LEVEL}
           )

IF( NOT GDCM_INCLUDE_DIR )

FOREACH(incSearchPath ${GDCM_INC_SEARCHPATH})
  
  FOREACH(incSearchSuffixFirst ${GDCM_INC_SEARCHSUFFIXES_FIRST_LEVEL})

      SET(SUBDIRS "")
      SUBDIRLIST(SUBDIRS "${incSearchPath}/${incSearchSuffixFirst}")
      FOREACH(incSearchSuffixSecond ${SUBDIRS})
        LIST(APPEND GDCM_INC_SEARCHSUFFIXES_SECOND_LEVEL "${incSearchSuffixFirst}/${incSearchSuffixSecond}")
      ENDFOREACH(incSearchSuffixSecond)

  ENDFOREACH(incSearchSuffixFirst)

ENDFOREACH(incSearchPath)

#try to locate some header file to verify include path
find_path( GDCM_INCLUDE_DIR 
           NAMES
           gdcmDefs.h
           PATHS
           ${GDCM_INC_SEARCHPATH}
           PATH_SUFFIXES
           ${GDCM_INC_SEARCHSUFFIXES_SECOND_LEVEL}
           )


ENDIF( NOT GDCM_INCLUDE_DIR )
 

#Include dir found
IF(GDCM_INCLUDE_DIR)

####################### Find Libraries #######################

  #paths where we will be looking for libraries
  set( GDCM_LIB_SEARCHPATH
       ${GDCM_INC_SEARCHPATH}/lib
       /usr/lib
       /usr/local/lib
       )

  MARK_AS_ADVANCED(GDCM_LIBRARIES)

#list of all necessary libraries
  list(APPEND GDCM_LIBRARIES_LIST gdcmMSFF gdcmDSED gdcmCommon gdcmMEXD gdcmDICT)

  #Let's check if debug libraries are needed
  LIST (FIND CMAKE_CONFIGURATION_TYPES "Debug" _index)
  
  #if we need debug libraries
  IF (${_index} GREATER -1)
    
    #list of debug libraries
    list(APPEND GDCM_LIBRARIES_LIST_DEBUG gdcmMSFFd gdcmDSEDd gdcmCommond gdcmMEXDd gdcmDICTd)
    #create library list using above specified macro, with debug and optimized specifiers
    CREATE_LIB_LIST(GDCM_LIBRARIES "${GDCM_LIBRARIES_LIST_DEBUG}" "debug")
    CREATE_LIB_LIST(tmp "${GDCM_LIBRARIES_LIST}" "optimized")
    list(APPEND GDCM_LIBRARIES ${tmp})

  ELSE()
    #assume we build only release and  debug or optimized specifiers are not needed
    CREATE_LIB_LIST(GDCM_LIBRARIES "${GDCM_LIBRARIES_LIST}")
  ENDIF() 
  
  IF(GDCM_LIBRARIES)
  	SET( GDCM_FOUND 1 )
  ENDIF(GDCM_LIBRARIES)


ENDIF( GDCM_INCLUDE_DIR )

# display help message
if( NOT GDCM_FOUND )

      set( GDCM_ROOT_DIR "" CACHE PATH "GDCM root dir" )
      message( FATAL_ERROR "GDCM required but some headers or libs not found. Please specify it's location with GDCM_ROOT_DIR variable."  )
      
endif( NOT GDCM_FOUND )
