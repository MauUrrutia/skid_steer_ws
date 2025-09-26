#include "skid_steer_cpp_examples/simple_tf_kinematics.hpp"


SimpleTfKinematics::SimpleTfKinematics(const std::string &name) : Node(name), x_increment{0.05}, last_x{0.0}, rotation_counter{}{
    static_tf_broadcaster = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);
    dynamic_tf_bracaster = std::make_unique<tf2_ros::TransformBroadcaster>(this);
    tf_buffer = std::make_unique<tf2_ros::Buffer>(get_clock());
    tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);
    static_transform_stamped.header.stamp = get_clock()->now();
    static_transform_stamped.header.frame_id = "skid_steer_base";
    static_transform_stamped.child_frame_id = "skid_steer_top";
    static_transform_stamped.transform.translation.x = 0;
    static_transform_stamped.transform.translation.y = 0;
    static_transform_stamped.transform.translation.z = 0.3;
    static_transform_stamped.transform.rotation.x = 0;
    static_transform_stamped.transform.rotation.y = 0;
    static_transform_stamped.transform.rotation.z = 0;
    static_transform_stamped.transform.rotation.w = 1;

    static_tf_broadcaster->sendTransform(static_transform_stamped);
    last_orientation.setRPY(0 , 0, 0);
    orientation_increment.setRPY(0.05, 0, 0.05);

    RCLCPP_INFO_STREAM(get_logger(), "Publishing static transform between " << static_transform_stamped.header.frame_id 
                                                                            << " and " << static_transform_stamped.child_frame_id);
    timer = create_wall_timer(std::chrono::milliseconds(100), std::bind(&SimpleTfKinematics::timer_callback,  this));
    get_transform_service = create_service<skid_steer_msgs::srv::GetTransform>("get_transform", 
                                                                                std::bind(&SimpleTfKinematics::get_transform_callback, 
                                                                                this, std::placeholders::_1, std::placeholders::_2));
}

void SimpleTfKinematics::timer_callback(){
    dynamic_transform_stamped.header.stamp = get_clock()->now();
    dynamic_transform_stamped.header.frame_id = "odom";
    dynamic_transform_stamped.child_frame_id = "skid_steer_base";
    dynamic_transform_stamped.transform.translation.x = last_x + x_increment;
    dynamic_transform_stamped.transform.translation.y = 0.0;
    dynamic_transform_stamped.transform.translation.z = 0.0;
    tf2::Quaternion q;
    q = last_orientation * orientation_increment;
    q.normalize();
    dynamic_transform_stamped.transform.rotation.x = q.x();
    dynamic_transform_stamped.transform.rotation.y = q.y();
    dynamic_transform_stamped.transform.rotation.z = q.z();
    dynamic_transform_stamped.transform.rotation.w = q.w();

    dynamic_tf_bracaster->sendTransform(dynamic_transform_stamped);
    last_x = dynamic_transform_stamped.transform.translation.x;  
    rotation_counter ++;
    last_orientation = q;
    if (rotation_counter >= 100){
        orientation_increment = orientation_increment.inverse();
        rotation_counter = 0;
    }
}

bool SimpleTfKinematics::get_transform_callback(const std::shared_ptr<skid_steer_msgs::srv::GetTransform::Request> request, 
                                const std::shared_ptr<skid_steer_msgs::srv::GetTransform::Response> response)
{
    RCLCPP_INFO_STREAM(get_logger(), "Rquested transform between " << request->frame_id << " and " << request->child_frame_id);
    geometry_msgs::msg::TransformStamped requested_transform;
    try{
        requested_transform = tf_buffer->lookupTransform(request->frame_id, request->child_frame_id, tf2::TimePointZero);
    }
    catch(tf2::TransformException &e)
    {
        RCLCPP_ERROR_STREAM(get_logger(), "An error ocurre while transforming from: " << request->frame_id << " and " << request->child_frame_id << ": " << e.what());
        response->success = false;
        return true;
    }
    response->transform = requested_transform;
    response->success = true;
    return true;
}

int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleTfKinematics>("simple_tf_kinematics");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

