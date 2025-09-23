#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_publisher;

void imu_callback(const sensor_msgs::msg::Imu &imu){
    sensor_msgs::msg::Imu new_imu;
    new_imu = imu;
    new_imu.header.frame_id = "base_footprint_ekf";
    imu_publisher->publish(new_imu);
}

int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("imu_republisher_node");
    rclcpp::sleep_for(std::chrono::seconds(1));
    imu_publisher = node->create_publisher<sensor_msgs::msg::Imu>("imu_ekf", 10);
    auto imu_subscriber = node->create_subscription<sensor_msgs::msg::Imu>("imu/out", 10, imu_callback);

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}