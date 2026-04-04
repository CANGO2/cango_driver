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

add_executable(read_go2_state_node src/read_go2_state.cpp)
target_include_directories(read_go2_state_node PUBLIC include)
ament_target_dependencies(read_go2_state_node rclcpp std_msgs cango_msgs unitree_go unitree_api)

add_executable(go2_mode_ctrl_node 
  src/go2_mode_ctrl.cpp 
  src/common/ros2_sport_client.cpp
)
target_include_directories(go2_mode_ctrl_node PUBLIC include)
ament_target_dependencies(go2_mode_ctrl_node rclcpp std_msgs cango_msgs unitree_go unitree_api)

add_executable(cango_driver_node 
  src/cango_driver.cpp
  src/common/ros2_sport_client.cpp
)
target_include_directories(cango_driver_node PUBLIC include)
ament_target_dependencies(cango_driver_node rclcpp std_msgs cango_msgs unitree_go unitree_api)


install(TARGETS 
  read_go2_state_node 
  go2_mode_ctrl_node 
  cango_driver_node
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
