#ifndef _TURTLESIM_KINEMATICS_HPP_
#define _TURTLESIM_KINEMATICS_HPP_
#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
class SimpleTurtlesimKinematics : public rclcpp::Node{
private:
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr turtle1_pose_subscriber;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr turtle2_pose_subscriber;
    turtlesim::msg::Pose last_turtle1_pose;
    turtlesim::msg::Pose last_turtle2_pose;
    void turtle1_pose_callback(const turtlesim::msg::Pose &pose);
    void turtle2_pose_callback(const turtlesim::msg::Pose &pose);
public: 
    SimpleTurtlesimKinematics(const std::string &name);
    ~SimpleTurtlesimKinematics();
};


#endif