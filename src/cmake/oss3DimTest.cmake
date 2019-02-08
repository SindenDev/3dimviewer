#///////////////////////////////////////////////////////////////////////////////
#
# 3DimViewer
# Lightweight 3D DICOM viewer.
#
# Copyright 2018 TESCAN 3DIM, s.r.o.
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
#///////////////////////////////////////////////////////////////////////////////


# Create library
ADD_TRIDIM_LIBRARY( 3DimTest )

# Options
set( TRIDIM_TEST_LIB_INCLUDE include )
set( TRIDIM_TEST_LIB_SRC src/test )

INCLUDE_BASIC_HEADERS()

target_include_directories(${TRIDIM_CURRENT_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/${TRIDIM_TEST_LIB_INCLUDE} )


ADD_LIB_VPL()
ADD_LIB_EIGEN()
ADD_LIB_OPENMESH()
ADD_LIB_OSG()


#-------------------------------------------------------------------------------
# Add Headers and Sources

ADD_HEADER_DIRECTORY( ${CMAKE_SOURCE_DIR}/${TRIDIM_TEST_LIB_INCLUDE} )
ADD_SOURCE_DIRECTORY( ${CMAKE_SOURCE_DIR}/${TRIDIM_TEST_LIB_SRC} )



#-------------------------------------------------------------------------------
# Finalize library

target_sources(${TRIDIM_CURRENT_TARGET} PRIVATE "${${TRIDIM_CURRENT_TARGET}_HEADERS}" "${${TRIDIM_CURRENT_TARGET}_SOURCES}")

#-------------------------------------------------------------------------------
# Create source groups

ADD_SOURCE_GROUPS( ${TRIDIM_TEST_LIB_INCLUDE}
                   ${TRIDIM_TEST_LIB_SRC}
                   )

set_target_properties( ${TRIDIM_CURRENT_TARGET} PROPERTIES
                        PROJECT_LABEL lib${TRIDIM_CURRENT_TARGET}
                        DEBUG_POSTFIX d
                        LINK_FLAGS "${TRIDIM_LINK_FLAGS}"
                        )

pop_target_stack()
