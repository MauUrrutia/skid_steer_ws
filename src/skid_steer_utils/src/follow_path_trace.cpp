#include "skid_steer_utils/follow_path_trace.hpp"


FollowPathTrace::FollowPathTrace(const std::string &name): Node(name){
    declare_parameter("odom_topic", "/skid_steer_odom");
    declare_parameter("path_length", 500);
    odom_topic = get_parameter("odom_topic").as_string();
    path_length = get_parameter("path_length").as_int();

    odom_subscriber = create_subscription<nav_msgs::msg::Odometry>(odom_topic, 10, 
                                          std::bind(&FollowPathTrace::odom_callback, this, std::placeholders::_1));
    path_publisher = create_publisher<nav_msgs::msg::Path>("/skid_steer_utils/trajectory", 10);
}

void FollowPathTrace::odom_callback(const nav_msgs::msg::Odometry &msg){
    
    geometry_msgs::msg::PoseStamped pose_stamped;
    path_trajectory.header.frame_id = msg.header.frame_id;
    pose_stamped.header.frame_id = msg.header.frame_id;
    pose_stamped.header.stamp = msg.header.stamp;
    pose_stamped.pose = msg.pose.pose;
    path_trajectory.poses.push_back(pose_stamped);
    
    if (path_trajectory.poses.size() > path_length)
        path_trajectory.poses.erase(path_trajectory.poses.begin());
    
    path_publisher->publish(path_trajectory);
}

int main(int argc ,char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<FollowPathTrace>("path_tracing");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}        