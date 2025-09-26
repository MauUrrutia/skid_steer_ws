#ifndef _FOLLOW_PATH_TRACE_HPP_
#define _FOLLOW_PATH_TRACE_HPP_
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <nav_msgs/msg/path.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <deque>
#include <string>

class FollowPathTrace : public rclcpp::Node{
private:
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_publisher;

    void odom_callback(const nav_msgs::msg::Odometry &msg);
    nav_msgs::msg::Path path_trajectory;
    long unsigned int path_length;
    std::string odom_topic;

public:
    FollowPathTrace(const std::string &name);

};

#endif