#include "skid_steer_controller/noisy_controller.hpp"
#include <Eigen/Geometry>

using std::placeholders::_1;


NoisyController::NoisyController(const std::string & name)
    : Node(name), left_wheels_array_previous_pos(Eigen::RowVector3d::Zero()), right_wheels_array_previous_pos(Eigen::RowVector3d::Zero()),
      x{}, y{}, theta{}{
        declare_parameter("wheel_radius", 0.059);
        declare_parameter("wheel_separation", 0.55645);

        wheel_radius = get_parameter("wheel_radius").as_double();
        wheel_separation = get_parameter("wheel_separation").as_double();

        RCLCPP_INFO_STREAM(get_logger(), "Using wheel_radius " << wheel_radius);
        RCLCPP_INFO_STREAM(get_logger(), "Using wheel_separation " << wheel_separation);
        previous_time = get_clock()->now();

        joint_subscriber = create_subscription<sensor_msgs::msg::JointState>("/joint_states", 10, 
                        std::bind(&NoisyController::joint_callback, this, std::placeholders::_1));

        
        odom_publisher = create_publisher<nav_msgs::msg::Odometry>("/skid_steer_controller/odom_noisy", 10);
        odometry_msg.header.frame_id = "odom";
        odometry_msg.child_frame_id = "base_footprint_ekf";
        odometry_msg.pose.pose.orientation.x = 0;
        odometry_msg.pose.pose.orientation.y = 0;
        odometry_msg.pose.pose.orientation.z = 0;
        odometry_msg.pose.pose.orientation.w = 1;

        transform_broadcaster = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
        transform_stamped.header.frame_id = "odom";
        transform_stamped.child_frame_id = "base_footprint_noisy";



    };


void NoisyController::joint_callback(const sensor_msgs::msg::JointState &msg){

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine noise_generator{seed};
    std::normal_distribution<double> left_encoders_noise(0.0, 0.0005);
    std::normal_distribution<double> right_encoders_noise(0.0, 0.0005);
    Eigen::RowVector3d left_wheel_encoder_vec = Eigen::RowVector3d(msg.position.at(1) + left_encoders_noise(noise_generator), msg.position.at(3) + left_encoders_noise(noise_generator), msg.position.at(5) + left_encoders_noise(noise_generator));
    Eigen::RowVector3d right_wheel_encoder_vec = Eigen::RowVector3d(msg.position.at(0) + right_encoders_noise(noise_generator), msg.position.at(2) + right_encoders_noise(noise_generator), msg.position.at(4) + right_encoders_noise(noise_generator));


    Eigen::RowVector3d dp_left_vec = left_wheel_encoder_vec - left_wheels_array_previous_pos;
    Eigen::RowVector3d dp_right_vec = right_wheel_encoder_vec - right_wheels_array_previous_pos;
    rclcpp::Time msg_time = msg.header.stamp;
    rclcpp::Duration dt = msg_time - previous_time;
    left_wheels_array_previous_pos = Eigen::RowVector3d(msg.position.at(1), msg.position.at(3), msg.position.at(5));
    right_wheels_array_previous_pos = Eigen::RowVector3d(msg.position.at(0), msg.position.at(2), msg.position.at(4));
    previous_time = msg_time;

    Eigen::RowVector3d fi_left_array = dp_left_vec / dt.seconds();
    Eigen::RowVector3d fi_right_array = dp_right_vec / dt.seconds();

    double fi_left_avg = fi_left_array.mean() ;
    double fi_right_avg = fi_right_array.mean() ;

    double linear = wheel_radius * (fi_right_avg + fi_left_avg) / 2.0;
    double angular = wheel_radius * (fi_right_avg - fi_left_avg) / wheel_separation;
    
    
    Eigen::RowVector3d d_pos_vec = (wheel_radius * dp_right_vec + wheel_radius * dp_left_vec) / 2.0;
    Eigen::RowVector3d d_theta_vec = (wheel_radius * dp_right_vec - wheel_radius * dp_left_vec) / wheel_separation;
    double d_pos = d_pos_vec.mean() * d_pos_vec.size();
    double d_theta = d_theta_vec.mean() * d_theta_vec   .size();
    theta += d_theta; 
    x += d_pos * cos(theta);
    y += d_pos * sin(theta);
    
    tf2::Quaternion q;
    q.setRPY(0, 0, theta);
    odometry_msg.header.stamp = get_clock()->now();

    odometry_msg.pose.pose.orientation.x = q.getX();
    odometry_msg.pose.pose.orientation.y = q.getY();
    odometry_msg.pose.pose.orientation.z = q.getZ();
    odometry_msg.pose.pose.orientation.w = q.getW();
    
    odometry_msg.pose.pose.position.set__x(x);
    odometry_msg.pose.pose.position.set__y(y);
    odometry_msg.pose.pose.position.set__z(0.0);
    odometry_msg.twist.twist.linear.x = linear;
    odometry_msg.twist.twist.angular.z = angular;

    
    transform_stamped.header.stamp = get_clock()->now();

    transform_stamped.transform.translation.set__x(x);
    transform_stamped.transform.translation.set__y(y);
    transform_stamped.transform.translation.set__x(x);
    transform_stamped.transform.translation.set__z(0.0);

    transform_stamped.transform.rotation.set__x(q.getX());
    transform_stamped.transform.rotation.set__y(q.getY());
    transform_stamped.transform.rotation.set__z(q.getZ());
    transform_stamped.transform.rotation.set__w(q.getW());
    

    odom_publisher->publish(odometry_msg);
    transform_broadcaster->sendTransform(transform_stamped);
};

int main(int argc ,char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NoisyController>("noisy_controller");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}                       



