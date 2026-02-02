cmake_minimum_required(VERSION 3.8)
project(map_tools)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(interactive_markers REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(yaml-cpp REQUIRED)

find_package(rviz_common REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(rviz_ogre_vendor REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(pluginlib REQUIRED)

find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

include_directories(include)
include_directories(${QT_INCLUDE_DIRS})

ament_export_dependencies(
  "interactive_markers"
  "rclcpp"
  "rclcpp_components"
  "tf2"
  "tf2_geometry_msgs"
  "tf2_ros"
  "visualization_msgs"
  "geometry_msgs"
  "std_msgs"
  "yaml-cpp")

find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/WayPoint.msg"
  "msg/WayPointArray.msg"
  "srv/AddPoint.srv"
  "srv/DeletePoint.srv"
  DEPENDENCIES
  "geometry_msgs"
)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")



set(map_tools_plugins_headers_to_moc
  include/map_tools/rviz_tools.hpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 ${rviz_QT_VERSION} REQUIRED Core Widgets)
set(QT_LIBRARIES Qt5::Widgets)
add_definitions(-DQT_NO_KEYWORDS)

set(library_name map_tools_plugins)

add_library(${library_name} SHARED
  include/map_tools/rviz_tools.hpp
  src/rviz_tools.cpp
  ${map_tools_plugins_headers_to_moc}
)

set(dependencies
  geometry_msgs
  pluginlib
  Qt5
  rclcpp
  rviz_common
  rviz_default_plugins
  rviz_ogre_vendor
  rviz_rendering
  tf2_geometry_msgs
  visualization_msgs
)

ament_target_dependencies(${library_name}
  ${dependencies}
)

target_include_directories(${library_name} PUBLIC
  ${Qt5Widgets_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}
)

target_link_libraries(${library_name}
  rviz_common::rviz_common
  ${QT_LIBRARIES}
  "${cpp_typesupport_target}"
)

target_compile_definitions(${library_name} PRIVATE "RVIZ_DEFAULT_PLUGINS_BUILDING_LIBRARY")
ament_export_targets(${library_name} HAS_LIBRARY_TARGET)

target_compile_definitions(${library_name} PUBLIC "PLUGINLIB__DISABLE_BOOST_FUNCTIONS")

pluginlib_export_plugin_description_file(rviz_common map_tools_plugin_description.xml)

install(
  FILES 
  map_tools_plugin_description.xml
  DESTINATION share/${PROJECT_NAME}
)

install(
  TARGETS ${library_name}
  EXPORT ${library_name}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

add_executable(rviz_tools_server src/rviz_tools_server.cpp)
ament_target_dependencies(rviz_tools_server
  "rclcpp"
  "rclcpp_components"
  "tf2_ros"
  "visualization_msgs"
  "geometry_msgs"
  ${dependencies}
)

target_link_libraries(rviz_tools_server
  "${cpp_typesupport_target}"
)

install(
  DIRECTORY include/
  DESTINATION include/
)

install(
  TARGETS
    rviz_tools_server
  DESTINATION
    lib/${PROJECT_NAME}
)


install(DIRECTORY launch rviz msg srv meshes map
  DESTINATION share/${PROJECT_NAME})


ament_package()
