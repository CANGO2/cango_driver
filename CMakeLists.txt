cmake_minimum_required(VERSION 3.10)
project(cango_driver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cango_msgs REQUIRED)
find_package(unitree_go REQUIRED)
find_package(unitree_api REQUIRED)

set(TARGET_NAME cango_driver)

add_executable(${TARGET_NAME}
  src/cango_driver.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  /usr/local/include
  /usr/local/include/ddscxx
)

ament_target_dependencies(${TARGET_NAME}
  rclcpp
  std_msgs
  cango_msgs
  unitree_go
  unitree_api
)

target_link_libraries(${TARGET_NAME}
  unitree_sdk2
  ddsc
  ddscxx
)

add_executable(keyboard_control
  src/keyboard_control.cpp
)

ament_target_dependencies(keyboard_control
  rclcpp
  cango_msgs
)

add_executable(motion_check
  src/motion_check.cpp
)

target_include_directories(motion_check PUBLIC
  /usr/local/include
  /usr/local/include/ddscxx
)

ament_target_dependencies(motion_check
  rclcpp
)

target_link_libraries(motion_check
  unitree_sdk2
  ddsc
  ddscxx
)

install(TARGETS
  ${TARGET_NAME}
  keyboard_control
  motion_check
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
  OPTIONAL
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
  OPTIONAL
)

ament_package()