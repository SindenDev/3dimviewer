#===============================================================================
# $Id: Required3rdPartyLibraries.txt 1283 2011-04-28 11:26:26Z spanel $
#
# 3DimViewer
# Lightweight 3D DICOM viewer.
#
# Copyright 2008-2012 3Dim Laboratory s.r.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#===============================================================================

#-------------------------------------------------------------------------------
# Find additional libraries

# Find WX library
macro( ADD_LIB_WX )
  if( NOT ADD_LIB_WX_INCLUDED )
    set( ADD_LIB_WX_INCLUDED TRUE )
    
    SET( wxWidgets_USE_LIBS base core adv gl html aui )
    FIND_PACKAGE( OurwxWidgets COMPONENTS base core adv gl html aui )
    IF( wxWidgets_FOUND )
        INCLUDE(${wxWidgets_USE_FILE})
        INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDE_DIRS} )
#        LINK_DIRECTORIES( ${wxWidgets_LIBRARIES_DIR} )
    	SET( WX_LINKED_LIBS ${wxWidgets_LIBRARIES} CACHE STRING "wxWidgets libraries." )
        if( WIN32 )
          add_definitions( -D__WXMSW__ )
        endif( WIN32 )
#        if( wxWidgets_USE_UNICODE )
#          add_definitions( -D_UNICODE )
#        endif( wxWidgets_USE_UNICODE )

    ELSE( wxWidgets_FOUND )
    	MESSAGE( FATAL_ERROR "wxWidgets was not found! Please, set the cache entry wxWidgets_ROOT_DIR!" )
    ENDIF( wxWidgets_FOUND )
    
  endif( NOT ADD_LIB_WX_INCLUDED )
endmacro( ADD_LIB_WX )


# Find OpenSSL library
macro( ADD_LIB_OPENSSL )
  if( NOT ADD_LIB_OPENSSL_INCLUDED )
    set( ADD_LIB_OPENSSL_INCLUDED TRUE )
    
    FIND_PACKAGE( OurOpenSSL )
    IF( OPENSSL_FOUND )
      INCLUDE_DIRECTORIES( ${OPENSSL_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${OPENSSL_LIBRARIES_DIR} )
      SET( OPENSSL_LINKED_LIBS ${OPENSSL_LIBRARIES} CACHE STRING "All the OPENSSL libraries." )

    ELSE( OPENSSL_FOUND )
      MESSAGE( FATAL_ERROR "OpenSSL library was not found! Please, set the cache entry OPENSSL_ROOT_DIR!"  )
    ENDIF( OPENSSL_FOUND )

  endif( NOT ADD_LIB_OPENSSL_INCLUDED )
endmacro( ADD_LIB_OPENSSL )

# Find Eigen library
macro( ADD_LIB_EIGEN )
  if( NOT ADD_LIB_EIGEN_INCLUDED )
    set( ADD_LIB_EIGEN_INCLUDED TRUE )
    
    FIND_PACKAGE( Eigen3 )
    IF( EIGEN3_FOUND )
      INCLUDE_DIRECTORIES( ${EIGEN3_INCLUDE_DIR} )
#      LINK_DIRECTORIES( ${OPENSSL_LIBRARIES_DIR} )
#      SET( OPENSSL_LINKED_LIBS ${OPENSSL_LIBRARIES} CACHE STRING "All the OPENSSL libraries." )

    ELSE( EIGEN3_FOUND )
      MESSAGE( FATAL_ERROR "Eigen3 library was not found! Please, set the cache entry EIGEN3_ROOT_DIR!"  )
    ENDIF( EIGEN3_FOUND )

  endif( NOT ADD_LIB_EIGEN_INCLUDED )
endmacro( ADD_LIB_EIGEN )


# Find the VPL library.
macro( ADD_LIB_VPL )
  if( NOT ADD_LIB_VPL_INCLUDED )
    set( ADD_LIB_VPL_INCLUDED TRUE )

#    SET( VPL_ROOT_DIR "" CACHE PATH "VPL root directory." )
    FIND_PACKAGE( VPL )
    IF( VPL_FOUND )
      INCLUDE_DIRECTORIES( ${VPL_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${VPL_LIBRARIES_DIR} )
      SET( VPL_LINKED_LIBS ${VPL_LIBRARIES} CACHE STRING "All the VPL libraries." )

    ELSE( VPL_FOUND )
      MESSAGE( FATAL_ERROR "VPL was not found! Please, set the cache entry VPL_ROOT_DIR!" )
    ENDIF( VPL_FOUND )

  endif( NOT ADD_LIB_VPL_INCLUDED )
endmacro( ADD_LIB_VPL )


# Find the OpenMesh library.
macro( ADD_LIB_OPENMESH )
  if( NOT ADD_LIB_OPENMESH_INCLUDED )
    set( ADD_LIB_OPENMESH_INCLUDED TRUE )

#    SET( OPENMESH_ROOT_DIR "" CACHE PATH "OpenMesh root directory." )
    FIND_PACKAGE( OurOpenMesh )
    IF( OPENMESH_FOUND )
      INCLUDE_DIRECTORIES( ${OPENMESH_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${OPENMESH_LIBRARIES_DIR} )
      SET( OPENMESH_LINKED_LIBS ${OPENMESH_LIBRARIES} CACHE STRING "All the OpenMesh libraries." )

    ELSE( OPENMESH_FOUND )
      MESSAGE( FATAL_ERROR "OpenMesh was not found! Please, set the cache entry OPENMESH_ROOT_DIR!" )
    ENDIF( OPENMESH_FOUND )

  endif( NOT ADD_LIB_OPENMESH_INCLUDED )
endmacro( ADD_LIB_OPENMESH )


# Find the Bullet library.
macro( ADD_LIB_BULLET )
  if( NOT ADD_LIB_BULLET_INCLUDED )
    set( ADD_LIB_BULLET_INCLUDED 1 )

#    SET( BULLET_ROOT_DIR "" CACHE PATH "Bullet root directory." )
    FIND_PACKAGE( OurBullet )
    IF( BULLET_FOUND )
      INCLUDE_DIRECTORIES( ${BULLET_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${BULLET_LIBRARIES_DIR} )
      SET( BULLET_LINKED_LIBS ${BULLET_LIBRARIES} CACHE STRING "All the Bullet libraries." )

    ELSE( BULLET_FOUND )
      MESSAGE( FATAL_ERROR "Bullet was not found! Please, set the cache entry BULLET_ROOT_DIR!" )
    ENDIF( BULLET_FOUND )

  endif( NOT ADD_LIB_BULLET_INCLUDED )
endmacro( ADD_LIB_BULLET )


# Find LeapMotion
macro( ADD_LIB_LEAPMOTION )
  if( NOT ADD_LIB_LEAPMOTION_INCLUDED )
    set( ADD_LIB_LEAPMOTION_INCLUDED TRUE )

    FIND_PACKAGE( OurLeapMotion REQUIRED )
    # Set up used LeapMotion SDK.
    IF( LEAPMOTION_FOUND )
      INCLUDE_DIRECTORIES(${LEAPMOTION_INCLUDE_DIR})
      LINK_DIRECTORIES( ${LEAPMOTION_LIBRARIES_DIR} )
      SET( LEAPMOTION_LINKED_LIBS ${LEAPMOTION_LIBRARIES} CACHE STRING "LeapMotion SDK." )

    ELSE( LEAPMOTION_FOUND )
      MESSAGE( FATAL_ERROR "LeapMotion SDK was not found! Please, set the cache entry LEAPMOTION_ROOT_DIR!" )
    ENDIF( LEAPMOTION_FOUND )

  endif( NOT ADD_LIB_LEAPMOTION_INCLUDED )
endmacro( ADD_LIB_LEAPMOTION )


# Find OSG library
macro( ADD_LIB_OSG )
  if( NOT ADD_LIB_OSG_INCLUDED )
    set( ADD_LIB_OSG_INCLUDED TRUE )

    FIND_PACKAGE( OurOSG )
    IF( OSG_FOUND )
      INCLUDE_DIRECTORIES( ${OSG_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${OSG_LIBRARIES_DIR} )
      SET( OSG_LINKED_LIBS ${OSG_LIBRARIES} CACHE STRING "All the OSG libraries." )

    ELSE( OSG_FOUND )
      MESSAGE( FATAL_ERROR "OSG library was not found! Please, set the cache entry OSG_ROOT_DIR!"  )
    ENDIF( OSG_FOUND )

  endif( NOT ADD_LIB_OSG_INCLUDED )
endmacro( ADD_LIB_OSG )

macro( ADD_LIB_GLU )
  if( NOT ADD_LIB_GLU_INCLUDED )
    set( ADD_LIB_GLU_INCLUDED TRUE)
    
    FIND_PACKAGE (glu REQUIRED)
    IF( OPENGL_GLU_FOUND )
      INCLUDE_DIRECTORIES( ${GLU_INCLUDE_PATH} )
      SET( GLU_LINKED_LIBS ${GLU_LIBRARY} CACHE STRING "GLU library." )
    ELSE( OPENGL_GLU_FOUND )
      MESSAGE( FATAL_ERROR "GLU library was not found! Please, set the cache entry GLFW_ROOT_DIR!"  )
    ENDIF( OPENGL_GLU_FOUND )
  
  endif(NOT ADD_LIB_GLU_INCLUDED)
endmacro( ADD_LIB_GLU )

macro( ADD_LIB_OPENGL )
  if( NOT ADD_LIB_OPENGL_INCLUDED )
    set( ADD_LIB_OPENGL_INCLUDED TRUE)
    
    FIND_PACKAGE (opengl REQUIRED)
    IF( OPENGL_FOUND )
      INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_PATH} )
      SET( OPENGL_LINKED_LIBS ${OPENGL_LIBRARY} CACHE STRING "OPENGL library." )
    ELSE( OPENGL_FOUND )
      MESSAGE( FATAL_ERROR "OPENGL library was not found! Please, set the cache entry OPENGL_ROOT_DIR!"  )
    ENDIF( OPENGL_FOUND )
  
  endif(NOT ADD_LIB_OPENGL_INCLUDED)
endmacro( ADD_LIB_OPENGL )

# Find QT4 library
macro( ADD_LIB_QT4 )
  if( NOT ADD_LIB_QT_INCLUDED )
    set( ADD_LIB_QT_INCLUDED TRUE )

    if (QT_BASIC)
      FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL QtNetwork)
    else (QT_BASIC)
      FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL QtWebKit QtNetwork QtSql QtScript)
    endif (QT_BASIC)
    IF( QT_FOUND )
      INCLUDE(${QT_USE_FILE})
      INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR}  )
      LINK_DIRECTORIES( ${QT_LIBRARIES_DIR} )
      SET( QT_LINKED_LIBS ${QT_LIBRARIES} CACHE STRING "All the QT libraries." )
    ELSE( QT_FOUND )
      MESSAGE( FATAL_ERROR "QT library was not found! Please, set the cache entry QT_ROOT_DIR!"  )
    ENDIF( QT_FOUND )

  endif( NOT ADD_LIB_QT_INCLUDED )
endmacro( ADD_LIB_QT4 )

# Find QT5 library
macro( ADD_LIB_QT5 )
  if( NOT ADD_LIB_QT_INCLUDED )
    set( ADD_LIB_QT_INCLUDED TRUE )
    
    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    # Instruct CMake to run moc automatically when needed.
    #set(CMAKE_AUTOMOC ON)    

    if (QT_BASIC)
    set(_components
        Core
        Gui
        Network
        Widgets
        PrintSupport
        OpenGL
        LinguistTools
		    Sql
        Script
      )
    else (QT_BASIC)
    set(_components
        Core
        Gui
        Network
        Widgets
        PrintSupport
		    WebEngine
        WebEngineCore
        WebEngineWidgets
        OpenGL
        LinguistTools
		    Sql
        Script
      )    
    endif (QT_BASIC)
    
    foreach(_component ${_components})
      find_package(Qt5${_component})
#      list(APPEND QT_LIBRARIES ${Qt5${_component}_LIBRARIES})
      list(APPEND QT_INCLUDE_DIR ${Qt5${_component}_INCLUDE_DIRS})      
    endforeach()
    #MESSAGE( warning ${QT_INCLUDE_DIR})
    INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR}  )      
   
    IF( Qt5Core_FOUND )
      #MESSAGE( WARNING ${PROJECT_NAME} )
      #foreach(_component ${_components})
      #  qt5_use_modules(${PROJECT_NAME} ${_component}) 
      #endforeach()      
    ELSE( Qt5Core_FOUND )
      MESSAGE( FATAL_ERROR "QT library was not found! Please, set the cache entry QT_ROOT_DIR!"  )
    ENDIF( Qt5Core_FOUND )

  endif( NOT ADD_LIB_QT_INCLUDED )
endmacro( ADD_LIB_QT5 )

macro ( ADD_LIB_QT )
  if (BUILD_WITH_QT5)
    foreach(f ${ARGN})
      set(MYARGS ${MYARGS} ${f})
    endforeach()  
    ADD_LIB_QT5(${MYARGS})
    set(MYARGS)      
  else (BUILD_WITH_QT5)
    foreach(f ${ARGN})
      set(MYARGS ${MYARGS} ${f})
    endforeach()  
    ADD_LIB_QT4(${MYARGS})
    set(MYARGS)   
  endif (BUILD_WITH_QT5)
endmacro ( ADD_LIB_QT )

macro( USE_LIB_QT5 )
  if (QT_BASIC)
    set(_components
        Core
        Gui
        Network
        Widgets
        PrintSupport
        OpenGL
        LinguistTools
		    Sql
        Script
      )
    else (QT_BASIC)
    set(_components
        Core
        Gui
        Network
        Widgets
        PrintSupport
		    WebEngine
        WebEngineCore
        WebEngineWidgets
        OpenGL
        LinguistTools
		    Sql
        Script
      )    
    endif (QT_BASIC)      
    foreach(_component ${_components})
      qt5_use_modules(${PROJECT_NAME} ${_component}) 
    endforeach()      
endmacro( USE_LIB_QT5 )

# Find DCMTk library
macro( ADD_LIB_DCMTK )
  if( NOT ADD_LIB_DCMTK_INCLUDED )
    set( ADD_LIB_DCMTK_INCLUDED TRUE )

    FIND_PACKAGE( OurDCMTK )
    IF( DCMTK_FOUND )
      INCLUDE_DIRECTORIES( ${DCMTK_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${DCMTK_LIBRARIES_DIR} )
      SET( DCMTK_LINKED_LIBS ${DCMTK_LIBRARIES} CACHE STRING "All the DCMTK libraries." )
    
    ELSE( DCMTK_FOUND )
      MESSAGE( FATAL_ERROR "DCMTK library was not found! Please, set the cache entry DCMTK_ROOT_DIR!"  )
    ENDIF( DCMTK_FOUND )

  endif( NOT ADD_LIB_DCMTK_INCLUDED )
endmacro( ADD_LIB_DCMTK )


# Find glew 
macro( ADD_LIB_GLEW )
  if( NOT ADD_LIB_GLEW_INCLUDED )
    set( ADD_LIB_GLEW_INCLUDED TRUE )

    FIND_PACKAGE( OurGLEW )
    IF( GLEW_FOUND )
      INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${GLEW_LIBRARIES_DIR} )
      SET( GLEW_LINKED_LIBS ${GLEW_LIBRARY} CACHE STRING "All GLEW libraries." )

    ELSE( GLEW_FOUND )
      MESSAGE( FATAL_ERROR "GLEW library was not found! Please, set the cache entry GLEW_ROOT_DIR!"  )
    ENDIF( GLEW_FOUND ) 

  endif( NOT ADD_LIB_GLEW_INCLUDED )
endmacro( ADD_LIB_GLEW )

# Find glfw 
macro( ADD_LIB_GLFW )
  if( NOT ADD_LIB_GLFW_INCLUDED )
    set( ADD_LIB_GLFW_INCLUDED TRUE )

    FIND_PACKAGE( OurGLFW )
    IF( GLFW_FOUND )
      INCLUDE_DIRECTORIES( ${GLFW_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${GLFW_LIBRARIES_DIR} )
      SET( GLFW_LINKED_LIBS ${GLFW_LIBRARY} CACHE STRING "All GLFW libraries." )

    ELSE( GLFW_FOUND )
      MESSAGE( FATAL_ERROR "GLFW library was not found! Please, set the cache entry GLFW_ROOT_DIR!"  )
    ENDIF( GLFW_FOUND ) 

  endif( NOT ADD_LIB_GLFW_INCLUDED )
endmacro( ADD_LIB_GLFW )


# Find curl 
macro( ADD_LIB_CURL )
  if( NOT ADD_LIB_CURL_INCLUDED )
    set( ADD_LIB_CURL_INCLUDED TRUE )

    FIND_PACKAGE( OurCURL )
    IF( CURL_FOUND )
      INCLUDE_DIRECTORIES( ${CURL_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${CURL_LIBRARIES_DIR} )
      SET( CURL_LINKED_LIBS ${CURL_LIBRARY} CACHE STRING "All CURL libraries." )

    ELSE( CURL_FOUND )
      MESSAGE( FATAL_ERROR "CURL library was not found! Please, set the cache entry CURL_ROOT_DIR!"  )
    ENDIF( CURL_FOUND ) 

  endif( NOT ADD_LIB_CURL_INCLUDED )
endmacro( ADD_LIB_CURL )

# Find the OpenCTM library.
macro( ADD_LIB_OPENCTM )
  if( NOT ADD_LIB_OPENCTM_INCLUDED )
    set( ADD_LIB_OPENCTM_INCLUDED 1 )

    FIND_PACKAGE( OurOpenCTM )
    IF( OPENCTM_FOUND )
      INCLUDE_DIRECTORIES( ${OPENCTM_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${OPENCTM_LIBRARIES_DIR} )
      SET( OPENCTM_LINKED_LIBS ${OPENCTM_LIBRARIES} CACHE STRING "All the OpenCTM libraries." )

    ELSE( OPENCTM_FOUND )
      MESSAGE( FATAL_ERROR "OpenCTM was not found!" )
    ENDIF( OPENCTM_FOUND )

  endif( NOT ADD_LIB_OPENCTM_INCLUDED )
endmacro( ADD_LIB_OPENCTM )

# Find zlib
macro( ADD_LIB_ZLIB )
  if( NOT ADD_LIB_ZLIB_INCLUDED )
    set( ADD_LIB_ZLIB_INCLUDED TRUE )

    FIND_PACKAGE( OurZLIB REQUIRED )
    # Set up used zlib library.
    IF( ZLIB_FOUND )
      INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
      LINK_DIRECTORIES( ${ZLIB_LIB} )
      SET( ZLIB_LINKED_LIBS ${ZLIB_LIBRARY} CACHE STRING "Zlib library." )

    ELSE( ZLIB_FOUND )
      MESSAGE( FATAL_ERROR "Zlib library was not found! Please, set the cache entry ZLIB_LIBRARY_DIR!" )
    ENDIF( ZLIB_FOUND )

  endif( NOT ADD_LIB_ZLIB_INCLUDED )
endmacro( ADD_LIB_ZLIB )

# Find png
macro( ADD_LIB_PNG )
  if( NOT ADD_LIB_PNG_INCLUDED )
    set( ADD_LIB_PNG_INCLUDED TRUE )

    FIND_PACKAGE( OurPNG REQUIRED )
    # Set up used zlib library.
    IF( PNG_FOUND )
      INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
      LINK_DIRECTORIES( ${PNG_LIB} )
      SET( PNG_LINKED_LIBS ${PNG_LIBRARY} CACHE STRING "PNG library." )

    ELSE( PNG_FOUND )
      MESSAGE( FATAL_ERROR "PNG library was not found! Please, set the cache entry PNG_LIBRARY_DIR!" )
    ENDIF( PNG_FOUND )

  endif( NOT ADD_LIB_PNG_INCLUDED )
endmacro( ADD_LIB_PNG )

# Find iconv
macro( ADD_LIB_ICONV )
  if( NOT ADD_LIB_ICONV_INCLUDED )
    set( ADD_LIB_ICONV_INCLUDED TRUE )

    FIND_PACKAGE( Iconv )
    IF( ICONV_FOUND )
      INCLUDE_DIRECTORIES( ${ICONV_INCLUDE_DIR} )
      SET( ICONV_LINKED_LIBS ${ICONV_LIBRARIES} CACHE STRING "All iconv libraries." )

    ELSE( ICONV_FOUND )
      MESSAGE( FATAL_ERROR "iconv library was not found!"  )
    ENDIF( ICONV_FOUND ) 

  endif( NOT ADD_LIB_ICONV_INCLUDED )
endmacro( ADD_LIB_ICONV )

# Find PhysicsFS
macro( ADD_LIB_PHYSFS )
  if( NOT ADD_LIB_PHYSFS_INCLUDED )
    set( ADD_LIB_PHYSFS_INCLUDED TRUE )

    FIND_PACKAGE( OurPhysFS REQUIRED )
    # Set up used PhysicsFS library.
    IF( PHYSFS_FOUND )
      INCLUDE_DIRECTORIES(${PHYSFS_INCLUDE_DIR})
      LINK_DIRECTORIES( ${PHYSFS_LIB} )
      SET( PHYSFS_LINKED_LIBS ${PHYSFS_LIBRARY} CACHE STRING "PhysicsFS library." )

    ELSE( PHYSFS_FOUND )
      MESSAGE( FATAL_ERROR "PhysicsFS library was not found! Please, set the cache entry PHYSFS_LIBRARY_DIR!" )
    ENDIF( PHYSFS_FOUND )

  endif( NOT ADD_LIB_PHYSFS_INCLUDED )
endmacro( ADD_LIB_PHYSFS )

# Find Geos
macro( ADD_LIB_GEOS )
  if( NOT ADD_LIB_GEOS_INCLUDED )
    set( ADD_LIB_GEOS_INCLUDED TRUE )

    FIND_PACKAGE( OurGEOS REQUIRED )
    IF( GEOS_FOUND )
      INCLUDE_DIRECTORIES(${GEOS_INCLUDE_DIR})
      LINK_DIRECTORIES( ${GEOS_LIB} )
      SET( GEOS_LINKED_LIBS ${GEOS_LIBRARY} CACHE STRING "GEOS library." )

    ELSE( GEOS_FOUND )
      MESSAGE( FATAL_ERROR "GEOS library was not found! Please, set the cache entry GEOS_ROOT_DIR!" )
    ENDIF( GEOS_FOUND )

  endif( NOT ADD_LIB_GEOS_INCLUDED )
endmacro( ADD_LIB_GEOS )

# Find VTK library
macro( ADD_LIB_VTK )
  if( NOT ADD_LIB_VTK_INCLUDED )
    set( ADD_LIB_VTK_INCLUDED TRUE )
    
    FIND_PACKAGE( VTK )
    IF( VTK_FOUND )
      #INCLUDE(${VTK_USE_FILE})
      INCLUDE_DIRECTORIES( ${VTK_INCLUDE_DIRS} )
      LINK_DIRECTORIES( ${VTK_RUNTIME_LIBRARY_DIRS} )
      SET( VTK_LINKED_LIBS ${VTK_LIBRARIES} CACHE STRING "All the VTK libraries." )
      SET( VTK_LINKED_LIBS ${VTK_LIBRARIES})
      #MESSAGE( warning "${VTK_LINKED_LIBS}"  )

    ELSE( VTK_FOUND )
      MESSAGE( FATAL_ERROR "VTK library was not found! Please, set the cache entry VTK_DIR!"  )
    ENDIF( VTK_FOUND )

  endif( NOT ADD_LIB_VTK_INCLUDED )
endmacro( ADD_LIB_VTK )

# Find Carve - beware that it is GPL and therefore can't be used in all apps
macro( ADD_LIB_CARVE )
  if( NOT ADD_LIB_CARVE_INCLUDED )
    set( ADD_LIB_CARVE_INCLUDED TRUE )

    FIND_PACKAGE( OurCarve REQUIRED )
    IF( CARVE_FOUND )
      INCLUDE_DIRECTORIES(${CARVE_INCLUDE_DIR})
      LINK_DIRECTORIES( ${CARVE_LIB} )
      SET( CARVE_LINKED_LIBS ${CARVE_LIBRARY} CACHE STRING "Carve library." )
    ELSE( CARVE_FOUND )
      MESSAGE( FATAL_ERROR "Carve library was not found! Please, set the cache entry CARVE_ROOT_DIR!" )
    ENDIF( CARVE_FOUND )

  endif( NOT ADD_LIB_CARVE_INCLUDED )
endmacro( ADD_LIB_CARVE )
