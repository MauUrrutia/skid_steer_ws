cmake_minimum_required(VERSION 3.8)
project(skid_steer_cpp_examples)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(turtlesim REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(skid_steer_msgs REQUIRED)

include_directories(include)

add_executable(simple_publisher src/simple_publisher.cpp)
ament_target_dependencies(simple_publisher rclcpp std_msgs)
add_executable(simple_subscriber src/simple_subscriber.cpp)
ament_target_dependencies(simple_subscriber rclcpp std_msgs)
add_executable(pcl_test src/pcl_test.cpp)
ament_target_dependencies(pcl_test rclcpp sensor_msgs  pcl_conversions)
add_executable(simple_lifecycle_node src/simple_lifecycle_node.cpp)
ament_target_dependencies(simple_lifecycle_node rclcpp rclcpp_lifecycle std_msgs)
add_executable(turtlesim_kinematics src/turtlesim_kinematics.cpp)
ament_target_dependencies(turtlesim_kinematics rclcpp turtlesim)
add_executable(simple_tf_kinematics src/simple_tf_kinematics.cpp)
ament_target_dependencies(simple_tf_kinematics rclcpp tf2_ros geometry_msgs skid_steer_msgs tf2)
add_executable(simple_service_server src/simple_service_server.cpp)
ament_target_dependencies(simple_service_server rclcpp skid_steer_msgs)
add_executable(simple_service_client src/simple_service_client.cpp)
ament_target_dependencies(simple_service_client rclcpp skid_steer_msgs)
add_executable(simple_qos_publisher src/simple_qos_publisher.cpp)
ament_target_dependencies(simple_qos_publisher rclcpp std_msgs)
add_executable(simple_qos_subscriber src/simple_qos_subscriber.cpp)
ament_target_dependencies(simple_qos_subscriber rclcpp std_msgs)


install(
  DIRECTORY include
  DESTINATION include
)
install(TARGETS
  simple_publisher
  simple_subscriber
  pcl_test
  simple_lifecycle_node
  turtlesim_kinematics
  simple_tf_kinematics
  simple_service_server
  simple_service_client
  simple_qos_publisher
  simple_qos_subscriber
  DESTINATION lib/${PROJECT_NAME}
)
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
