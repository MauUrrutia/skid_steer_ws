#ifndef SIMPLE_CONTROLLER_HPP
#define SIMPLE_CONTROLLER_HPP
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <Eigen/Core>
#include <vector>

class SimpleController : public rclcpp::Node{
public:
    SimpleController(const std::string & name);

private:
    
    void velCallback(const geometry_msgs::msg::TwistStamped & msg);
    void joint_callback(const sensor_msgs::msg::JointState &msg);
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr vel_subscriber;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_cmd_publisher;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_subscriber; 
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher;
    double wheel_radius;
    double wheel_separation;
    Eigen::Matrix2d speed_conversion;
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