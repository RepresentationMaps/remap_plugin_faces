cmake_minimum_required(VERSION 3.8...3.14)
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

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Targets
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/remap_plugin_faces/plugin_faces.cpp
)

target_link_libraries(${PROJECT_NAME})

pluginlib_export_plugin_description_file(remap_plugin_base plugins.xml)

# Testing
if(BUILD_TESTING)
  ament_auto_find_test_dependencies()
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package(USE_SCOPED_HEADER_INSTALL_DIR)
