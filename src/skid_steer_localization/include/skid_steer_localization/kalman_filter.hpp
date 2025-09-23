#ifndef _KALMAN_FILTER_HPP_
#define _KALMAN_FILTER_HPP_
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/imu.hpp>

class KalmanFilter : public rclcpp::Node
{
private:
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscriber;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher;

    
    double mean;
    double variance;
    double imu_vyaw;
    double last_vyaw;
    double motion;
    nav_msgs::msg::Odometry odom_filtered;
    bool is_first_odom;
    double motion_variance;
    double measurement_variance;
    
    
    void state_prediction();
    void update_measurements();
    void odom_callback(const nav_msgs::msg::Odometry &odom);
    void imu_callback(const sensor_msgs::msg::Imu &imu);
public:
    KalmanFilter(const std::string &name);
    
};


#endif 