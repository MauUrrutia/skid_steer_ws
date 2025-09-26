cmake_minimum_required(VERSION 3.8)
project(skid_steer_utils)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)

include_directories(include)

add_executable(follow_path_trace src/follow_path_trace.cpp)
ament_Target_dependencies(follow_path_trace rclcpp geometry_msgs nav_msgs)



install(
  DIRECTORY include
  DESTINATION include
)
install(TARGETS
  follow_path_trace
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)

  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
