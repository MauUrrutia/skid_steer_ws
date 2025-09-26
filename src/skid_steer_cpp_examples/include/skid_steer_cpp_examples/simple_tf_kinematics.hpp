#ifndef _SIMPLE_TF_KINEMATICS_HPP_
#define _SIMPLE_TF_KINEMATICS_HPP_
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/static_transform_broadcaster.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <skid_steer_msgs/srv/get_transform.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <memory>
class SimpleTfKinematics: public rclcpp::Node{
private:
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_broadcaster;
    geometry_msgs::msg::TransformStamped static_transform_stamped;
    std::unique_ptr<tf2_ros::TransformBroadcaster> dynamic_tf_bracaster;
    geometry_msgs::msg::TransformStamped dynamic_transform_stamped;
    rclcpp::TimerBase::SharedPtr timer; 
    double x_increment;
    double last_x;
    int rotation_counter;
    tf2::Quaternion last_orientation;
    tf2::Quaternion orientation_increment;
    
    rclcpp::Service<skid_steer_msgs::srv::GetTransform>::SharedPtr get_transform_service;
    std::unique_ptr<tf2_ros::Buffer> tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener;
    void timer_callback();
    bool get_transform_callback(const std::shared_ptr<skid_steer_msgs::srv::GetTransform::Request> request, 
                                const std::shared_ptr<skid_steer_msgs::srv::GetTransform::Response> response);
public:
    SimpleTfKinematics(const std::string &name);
    
};

#endif