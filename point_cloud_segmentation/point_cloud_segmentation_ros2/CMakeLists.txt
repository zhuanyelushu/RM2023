cmake_minimum_required(VERSION 3.8)
project(point_cloud_segmentation_ros2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-std=c++17 -O3)
include_directories(usr/include/vtk-9.1)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()
ament_auto_add_executable(
  point_cloud_segmentation
  src/point_cloud_segmentation_node.cc
)

ament_auto_package(INSTALL_TO_SHARE launch rviz)

FILE(GLOB_RECURSE LibFiles "include/*")
add_custom_target(headers SOURCES ${LibFiles})

