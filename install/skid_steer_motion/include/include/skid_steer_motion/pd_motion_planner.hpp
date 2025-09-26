#ifndef _PD_MOTION_PLANNER_HPP_
#define _PD_MOTION_PLANNER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/path.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <algorithm>
namespace skid_steer_motion
{

class PdMotionPlanner : public rclcpp::Node
{
private:
    rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr path_subscriber;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr next_pose_publisher;
    
    std::shared_ptr<tf2_ros::Buffer> tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener;
    
    rclcpp::TimerBase::SharedPtr control_loop;
    rclcpp::Time last_cycle_time;
    
    double kp;
    double kd;
    double step_size;
    double max_linear_vel;
    double max_angular_vel;
    double prev_linear_error;
    double prev_angular_error;
    
    nav_msgs::msg::Path global_plan;

    void ControlLoop();

    void PathCallback(const nav_msgs::msg::Path::SharedPtr path);

    bool TransformPlan(const std::string &frame);

    geometry_msgs::msg::PoseStamped GetNextPose(const geometry_msgs::msg::PoseStamped &robot_pose);


public:
    PdMotionPlanner(const std::string &name);
    ~PdMotionPlanner();
};

}

#endif