#ifndef _NOISY_CONTROLLER_HPP_
#define _NOISY_CONTROLLER_HPP_
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <Eigen/Core>
#include <random>
#include <vector>

class NoisyController : public rclcpp::Node{
public:
    NoisyController(const std::string & name);

private:
    void joint_callback(const sensor_msgs::msg::JointState &msg);
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_subscriber; 
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher;
    double wheel_radius;
    double wheel_separation;

    Eigen::RowVector3d left_wheels_array_previous_pos; 
    Eigen::RowVector3d right_wheels_array_previous_pos;
    rclcpp::Time previous_time;
    double x;
    double y;
    double theta;
    nav_msgs::msg::Odometry odometry_msg;

    std::unique_ptr<tf2_ros::TransformBroadcaster> transform_broadcaster;
    geometry_msgs::msg::TransformStamped transform_stamped;
};

#endif