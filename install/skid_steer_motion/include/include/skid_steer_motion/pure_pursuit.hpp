// #ifndef _PURE_PURSUIT_HPP_
// #define _PURE_PURSUIT_HPP_

// #include <rclcpp/rclcpp.hpp>
// #include <nav_msgs/msg/path.hpp>
// #include <geometry_msgs/msg/twist.hpp>
// #include <geometry_msgs/msg/pose_stamped.hpp>
// #include <tf2_ros/buffer.hpp>
// #include <tf2_ros/transform_listener.hpp>
// #include <geometry_msgs/msg/transform_stamped.hpp>
// #include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
// #include <algorithm>
// namespace skid_steer_motion
// {

// class PurePursuit : public rclcpp::Node
// {
// private:
//     rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr path_subscriber;
//     rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher;
//     rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr check_point_pose_publisher;
    
//     std::shared_ptr<tf2_ros::Buffer> tf_buffer;
//     std::shared_ptr<tf2_ros::TransformListener> tf_listener;
    
//     rclcpp::TimerBase::SharedPtr control_loop;


//     double look_ahead_distance;
//     double max_linear_vel;
//     double max_angular_vel;

    
//     nav_msgs::msg::Path global_plan;

//     void ControlLoop();

//     void PathCallback(const nav_msgs::msg::Path::SharedPtr path);

//     bool TransformPlan(const std::string &frame);

//     geometry_msgs::msg::PoseStamped GetCheckpointPose(const geometry_msgs::msg::PoseStamped &robot_pose);

//     double GetCurvature(const geometry_msgs::msg::Pose &checkpoint_pose);

// public:
//     PurePursuit(const std::string &name);
//     ~PurePursuit();
// };

// }

// #endif

#ifndef _PURE_PURSUIT_HPP_
#define _PURE_PURSUIT_HPP_

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/path.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <algorithm>
#include <cmath>

namespace skid_steer_motion
{

class PurePursuit : public rclcpp::Node
{
private:
    rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr path_subscriber;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr check_point_pose_publisher;
    
    std::shared_ptr<tf2_ros::Buffer> tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener;
    
    rclcpp::TimerBase::SharedPtr control_loop;

    double look_ahead_distance;
    double max_linear_vel;
    double max_angular_vel;
    double goal_tolerance;
    double min_linear_vel;
    
    nav_msgs::msg::Path global_plan;

    void ControlLoop();
    void PathCallback(const nav_msgs::msg::Path::SharedPtr path);
    bool TransformPlan(const std::string &frame);
    geometry_msgs::msg::Point GetLookaheadPoint(const geometry_msgs::msg::PoseStamped &robot_pose, bool &goal_reached);
    double CalculateCurvature(const geometry_msgs::msg::PoseStamped &robot_pose, const geometry_msgs::msg::Point &lookahead_point);
    geometry_msgs::msg::Twist CalculateVelocities(double curvature, const geometry_msgs::msg::PoseStamped &robot_pose);

public:
    PurePursuit();
    ~PurePursuit();
};

}

#endif