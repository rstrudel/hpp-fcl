#
# Software License Agreement (BSD License)
#
#  Copyright (c) 2014-2019 CNRS-LAAS, INRIA
#  Author: Florent Lamiraux, Joseph Mirabel
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of CNRS-LAAS. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)
set(CXX_DISABLE_WERROR TRUE)

set(PROJECT_NAME hpp-fcl)
set(PROJECT_DESCRIPTION
  "HPP fork of FCL -- The Flexible Collision Library"
  )
SET(PROJECT_USE_CMAKE_EXPORT TRUE)

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_CXX_STANDARD 98)

# Do not support CMake older than 2.8.12
CMAKE_POLICY(SET CMP0022 NEW)
SET(PROJECT_USE_KEYWORD_LINK_LIBRARIES TRUE)

# Check if the submodule cmake have been initialized
IF(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/base.cmake")
  MESSAGE(FATAL_ERROR "\nPlease run the following command first:\ngit submodule update --init\n")
ENDIF()

include(cmake/eigen.cmake)
include(cmake/boost.cmake)
include(cmake/python.cmake)
include(cmake/hpp.cmake)
include(cmake/apple.cmake)
include(cmake/ide.cmake)

# If needed, fix CMake policy for APPLE systems
APPLY_DEFAULT_APPLE_CONFIGURATION()

OPTION(BUILD_PYTHON_INTERFACE "Build the python bindings" OFF)

COMPUTE_PROJECT_ARGS(PROJECT_ARGS LANGUAGES CXX)
PROJECT(${PROJECT_NAME} ${PROJECT_ARGS})

add_required_dependency("eigen3 >= 3.0.0")

# Add a cache variable to allow not compiling and running tests
set (RUN_TESTS TRUE CACHE BOOL "compile and run unit tests")

# Required dependencies
set(BOOST_COMPONENTS thread date_time system)
if (RUN_TESTS)
  set(BOOST_COMPONENTS ${BOOST_COMPONENTS} filesystem unit_test_framework chrono)
endif ()
if (BUILD_PYTHON_INTERFACE)
  FINDPYTHON()
  set(BOOST_COMPONENTS ${BOOST_COMPONENTS} python)
endif ()

search_for_boost()
# Optional dependencies
add_optional_dependency("octomap >= 1.6")
if (OCTOMAP_FOUND)
  SET(HPP_FCL_HAVE_OCTOMAP TRUE)
  string(REPLACE "." ";" VERSION_LIST ${OCTOMAP_VERSION})
  list(GET VERSION_LIST 0 OCTOMAP_MAJOR_VERSION)
  list(GET VERSION_LIST 1 OCTOMAP_MINOR_VERSION)
  list(GET VERSION_LIST 2 OCTOMAP_PATCH_VERSION)
  message(STATUS "FCL uses Octomap")
else()
  SET(HPP_FCL_HAVE_OCTOMAP FALSE)
  message(STATUS "FCL does not use Octomap")
endif()

ADD_REQUIRED_DEPENDENCY("assimp >= 2.0")

SET(${PROJECT_NAME}_HEADERS
  include/hpp/fcl/collision_data.h
  include/hpp/fcl/profile.h
  include/hpp/fcl/BV/kIOS.h
  include/hpp/fcl/BV/BV.h
  include/hpp/fcl/BV/RSS.h
  include/hpp/fcl/BV/OBBRSS.h
  include/hpp/fcl/BV/BV_node.h
  include/hpp/fcl/BV/AABB.h
  include/hpp/fcl/BV/OBB.h
  include/hpp/fcl/BV/kDOP.h
  include/hpp/fcl/narrowphase/narrowphase.h
  include/hpp/fcl/narrowphase/gjk.h
  include/hpp/fcl/shape/geometric_shape_to_BVH_model.h
  include/hpp/fcl/shape/geometric_shapes.h
  include/hpp/fcl/distance_func_matrix.h
  include/hpp/fcl/collision.h
  include/hpp/fcl/collision_func_matrix.h
  include/hpp/fcl/distance.h
  include/hpp/fcl/math/matrix_3f.h
  include/hpp/fcl/math/vec_3f.h
  include/hpp/fcl/math/types.h
  include/hpp/fcl/math/transform.h
  include/hpp/fcl/data_types.h
  include/hpp/fcl/BVH/BVH_internal.h
  include/hpp/fcl/BVH/BVH_model.h
  include/hpp/fcl/BVH/BVH_front.h
  include/hpp/fcl/BVH/BVH_utility.h
  include/hpp/fcl/collision_object.h
  include/hpp/fcl/collision_utility.h
  include/hpp/fcl/octree.h
  include/hpp/fcl/fwd.hh
  include/hpp/fcl/mesh_loader/assimp.h
  include/hpp/fcl/mesh_loader/loader.h
  include/hpp/fcl/internal/BV_fitter.h
  include/hpp/fcl/internal/BV_splitter.h
  include/hpp/fcl/internal/intersect.h
  include/hpp/fcl/internal/tools.h
  include/hpp/fcl/internal/traversal_node_base.h
  include/hpp/fcl/internal/traversal_node_bvh_shape.h
  include/hpp/fcl/internal/traversal_node_bvhs.h
  include/hpp/fcl/internal/traversal_node_octree.h
  include/hpp/fcl/internal/traversal_node_setup.h
  include/hpp/fcl/internal/traversal_node_shapes.h
  include/hpp/fcl/internal/traversal_recurse.h
  include/hpp/fcl/internal/traversal.h
  )

add_subdirectory(src)
if (BUILD_PYTHON_INTERFACE)
  add_subdirectory(python)
endif ()
if (RUN_TESTS)
  add_subdirectory(test)
endif ()

pkg_config_append_libs("hpp-fcl")
PKG_CONFIG_APPEND_BOOST_LIBS(thread date_time filesystem system)
IF(HPP_FCL_HAVE_OCTOMAP)
  # FCL_HAVE_OCTOMAP kept for backward compatibility reasons.
  PKG_CONFIG_APPEND_CFLAGS(
    "-DHPP_FCL_HAVE_OCTOMAP -DFCL_HAVE_OCTOMAP -DOCTOMAP_MAJOR_VERSION=${OCTOMAP_MAJOR_VERSION} -DOCTOMAP_MINOR_VERSION=${OCTOMAP_MINOR_VERSION} -DOCTOMAP_PATCH_VERSION=${OCTOMAP_PATCH_VERSION}")
ENDIF(HPP_FCL_HAVE_OCTOMAP)

# Install catkin package.xml
INSTALL(FILES package.xml DESTINATION share/${PROJECT_NAME})

