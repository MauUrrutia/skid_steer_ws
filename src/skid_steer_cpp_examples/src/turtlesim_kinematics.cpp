#include "skid_steer_cpp_examples/turtlesim_kinematics.hpp"

SimpleTurtlesimKinematics::SimpleTurtlesimKinematics(const std::string &name) : Node(name){
    turtle1_pose_subscriber = create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10, 
                                std::bind(&SimpleTurtlesimKinematics::turtle1_pose_callback, this, std::placeholders::_1));
    turtle2_pose_subscriber = create_subscription<turtlesim::msg::Pose>("/turtle2/pose", 10, 
                                std::bind(&SimpleTurtlesimKinematics::turtle2_pose_callback, this, std::placeholders::_1));
}

void SimpleTurtlesimKinematics::turtle1_pose_callback(const turtlesim::msg::Pose &pose){
    last_turtle1_pose = pose;
}

void SimpleTurtlesimKinematics::turtle2_pose_callback(const turtlesim::msg::Pose &pose){
    last_turtle2_pose = pose;
    float Tx = last_turtle2_pose.x - last_turtle1_pose.x;
    float Ty = last_turtle2_pose.y - last_turtle1_pose.y;
    float Theta_rad = last_turtle2_pose.theta - last_turtle1_pose.theta;
    float Theta_deg = 180 * Theta_rad / 3.1416;

    RCLCPP_INFO_STREAM(get_logger(), "\n===========Translation vector turtle1 ---> turtle2 \n" <<
                                       "Tx: " << Tx << "\n" << 
                                       "Ty: " << Ty << 
                                       "Rotation matrix \n" <<
                                       "\nTheta[rad]: " << Theta_rad <<
                                       "\nTheta[deg]: " << Theta_deg <<
                                       "\n[R11    R12]: [" << std::cos(Theta_rad) << "\t" << -std::sin(Theta_rad) << "]\n" <<
                                       "[R21    R22]: [" << std::sin(Theta_rad) << "\t" << std::cos(Theta_rad) << "]\n" << std::endl);
}

SimpleTurtlesimKinematics::~SimpleTurtlesimKinematics(){
    RCLCPP_INFO_STREAM(get_logger(), "===========Translation vector Destructor turtle1 ---> turtle2 =========== \n" << std::endl);
}


int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleTurtlesimKinematics>("simple_turtlesim_kinematics");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}