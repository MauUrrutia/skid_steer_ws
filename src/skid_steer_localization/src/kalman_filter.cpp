#include "skid_steer_localization/kalman_filter.hpp"

KalmanFilter::KalmanFilter(const std::string &name): Node(name), mean{0.0}, variance{1000}, imu_vyaw{0.0},
                                                     last_vyaw{0.0}, motion{0.0}, is_first_odom{true},
                                                     motion_variance{4.0}, measurement_variance{0.5}
{
    odom_subscriber = create_subscription<nav_msgs::msg::Odometry>("skid_steer_controller/odom_noisy", 10, 
                      std::bind(&KalmanFilter::odom_callback, this, std::placeholders::_1));
    imu_subscriber = create_subscription<sensor_msgs::msg::Imu>("imu/out", 100, 
                      std::bind(&KalmanFilter::imu_callback, this, std::placeholders::_1));
    odom_publisher = create_publisher<nav_msgs::msg::Odometry>("skid_steer_controller/odom_filtered", 10);                                   
}

void KalmanFilter::imu_callback(const sensor_msgs::msg::Imu &imu){
    imu_vyaw = imu.angular_velocity.z;

}

void KalmanFilter::odom_callback(const nav_msgs::msg::Odometry &odom){
    odom_filtered = odom;
    if(is_first_odom){
        mean = odom.twist.twist.angular.z;
        is_first_odom = false;
        return;
    }

    motion = odom.twist.twist.angular.z - last_vyaw;


    state_prediction();
    update_measurements();
    odom_filtered.twist.twist.angular.z = mean;
    odom_publisher->publish(odom_filtered);
    
}

void KalmanFilter::update_measurements(){
    mean = (measurement_variance * mean + variance * imu_vyaw) / (variance + measurement_variance);
    variance = (variance * measurement_variance) / (variance + measurement_variance);
}

void KalmanFilter::state_prediction(){
    mean = mean + motion;
    variance = variance + motion_variance;
}


int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KalmanFilter>("kalman_filter");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}