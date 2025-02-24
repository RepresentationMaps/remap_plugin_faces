cmake_minimum_required(VERSION 3.8)
project(remap_plugin_faces)

add_compile_options(-fPIC)

# Global flags
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

list(APPEND CMAKE_MODULE_PATH "${FIND_OPENVDB_PATH}")

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Targets
add_library(remap_plugin_faces SHARED
src/remap_plugin_faces/plugin_faces.cpp)

target_link_libraries(${PROJECT_NAME}
  ${PCL_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

pluginlib_export_plugin_description_file(remap_plugin_base plugins.xml)

target_include_directories(remap_plugin_faces PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:include>)

ament_target_dependencies(
  remap_plugin_faces
  hri
  pluginlib
  rclcpp
  remap_map_handler
  remap_plugin_base
  remap_regions_register
  tf2
  tf2_ros
)

ament_export_include_directories(
  include
)

ament_export_libraries(
  remap_plugin_faces
)

ament_export_targets(
  export_${PROJECT_NAME}
)

install(
  TARGETS remap_plugin_faces
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

# Testing
if(BUILD_TESTING)
  ament_auto_find_test_dependencies()
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
