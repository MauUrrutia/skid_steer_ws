#include "skid_steer_motion/pid_motion_planner.hpp"

namespace skid_steer_motion
{
PidMotionPlanner::~PidMotionPlanner(){}
PidMotionPlanner::PidMotionPlanner(const std::string &name) : Node(name), 
kp(2.0), 
ki(0.01),
kd(0.1),  
integral_linear_error(0.0),
integral_angular_error(0.0),
max_integral(2.0),
step_size(0.2), 
max_linear_vel(3.0), 
max_angular_vel(3.0),
prev_linear_error(0.0), 
prev_angular_error(0.0)
{
    declare_parameter<double>("kp", kp);
    declare_parameter<double>("ki", ki);
    declare_parameter<double>("kd", kd);
    declare_parameter<double>("step_size", step_size);
    declare_parameter<double>("max_linear_vel", max_linear_vel);
    declare_parameter<double>("max_angular_vel", max_angular_vel);
    declare_parameter<double>("max_integral", max_integral);

    kp = get_parameter("kp").as_double();
    ki = get_parameter("ki").as_double();
    kd = get_parameter("kd").as_double();
    step_size = get_parameter("step_size").as_double();
    max_linear_vel = get_parameter("max_linear_vel").as_double();
    max_angular_vel = get_parameter("max_angular_vel").as_double();
    max_integral = get_parameter("max_integral").as_double();

    path_subscriber = create_subscription<nav_msgs::msg::Path>("/a_star/path", 10, 
                      std::bind(&PidMotionPlanner::PathCallback, this, std::placeholders::_1));

    cmd_vel_publisher = create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    next_pose_publisher = create_publisher<geometry_msgs::msg::PoseStamped>("pid/next_pose", 10);

    tf_buffer = std::make_shared<tf2_ros::Buffer>(get_clock());
    tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

    control_loop = create_wall_timer(std::chrono::milliseconds(100), std::bind(&PidMotionPlanner::ControlLoop, this));
    last_cycle_time = get_clock()->now();
}
    
void PidMotionPlanner::PathCallback(const nav_msgs::msg::Path::SharedPtr path){
    global_plan = *path;
}   

void PidMotionPlanner::ControlLoop(){
    
    if (global_plan.poses.empty())
        return;
    
    geometry_msgs::msg::TransformStamped robot_pose;
    
    try{
        robot_pose = tf_buffer->lookupTransform("odom", "base_footprint", tf2::TimePointZero);
    }
    catch(tf2::TransformException &e){
        RCLCPP_WARN(get_logger(), "Could not transform: %s", e.what());
        return;
    }
    
    if (!TransformPlan(robot_pose.header.frame_id)){
        RCLCPP_ERROR(get_logger(), "Unable to transform plan in robot's frame");
        return;
    }

    geometry_msgs::msg::PoseStamped robot_pose_stamped;
    robot_pose_stamped.header.frame_id = robot_pose.header.frame_id;

    robot_pose_stamped.pose.position.x = robot_pose.transform.translation.x;
    robot_pose_stamped.pose.position.y = robot_pose.transform.translation.y;
    robot_pose_stamped.pose.position.z = robot_pose.transform.translation.z;

    robot_pose_stamped.pose.orientation = robot_pose.transform.rotation;

    auto next_pose = GetNextPose(robot_pose_stamped);

    double dx = next_pose.pose.position.x - robot_pose_stamped.pose.position.x;
    double dy = next_pose.pose.position.y - robot_pose_stamped.pose.position.y;
    double dz = next_pose.pose.position.z - robot_pose_stamped.pose.position.z;
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    if(distance <= 0.2){
        RCLCPP_INFO(get_logger(), "Goal reached");
        global_plan.poses.clear();
        integral_linear_error = 0.0;
        integral_angular_error = 0.0;
        return;
    }

    next_pose_publisher->publish(next_pose);
    
    tf2::Transform robot_tf, next_pose_tf, next_pose_robot_tf;
    tf2::fromMsg(robot_pose_stamped.pose, robot_tf);
    tf2::fromMsg(next_pose.pose, next_pose_tf);
    next_pose_robot_tf = robot_tf.inverse() * next_pose_tf;
    
    double linear_error = next_pose_robot_tf.getOrigin().getX();
    double angular_error = next_pose_robot_tf.getOrigin().getY();
    double dt = (get_clock()->now() - last_cycle_time).seconds();
    double linear_error_derivative = (linear_error - prev_linear_error) / dt;
    double angular_error_derivative = (angular_error - prev_angular_error) / dt;
    integral_linear_error += linear_error * dt;
    integral_angular_error += angular_error * dt;
    integral_linear_error = std::clamp(integral_linear_error, -max_integral, max_integral);
    integral_angular_error = std::clamp(integral_angular_error, -max_integral, max_integral);
    
    geometry_msgs::msg::Twist cmd_vel;
    cmd_vel.linear.x = std::clamp(kp * linear_error + ki * integral_linear_error + kd * linear_error_derivative, -max_linear_vel, max_linear_vel);
    cmd_vel.angular.z = std::clamp(kp * angular_error + ki * integral_angular_error + kd * angular_error_derivative, -max_angular_vel, max_angular_vel);
    
    cmd_vel_publisher->publish(cmd_vel);

    last_cycle_time = get_clock()->now();
    prev_linear_error = linear_error;
    prev_angular_error = angular_error;
}



bool PidMotionPlanner::TransformPlan(const std::string &frame){
    if (global_plan.header.frame_id == frame)
        return true;
    geometry_msgs::msg::TransformStamped transform;
    try{
        transform = tf_buffer->lookupTransform(frame, global_plan.header.frame_id, tf2::TimePointZero);
    }
    catch(tf2::LookupException &e){
        RCLCPP_ERROR_STREAM(get_logger(), 
        "Couldn't transform plan from frame " << global_plan.header.frame_id << " to " << frame);
        return false;
    }
    
    for (auto &pose : global_plan.poses){
        tf2::doTransform(pose, pose, transform);
    }
    
    global_plan.header.frame_id = frame;
    return true;


}

geometry_msgs::msg::PoseStamped PidMotionPlanner::GetNextPose(const geometry_msgs::msg::PoseStamped &robot_pose){
    auto next_pose = global_plan.poses.back();
    for (auto pose = global_plan.poses.rbegin(); pose != global_plan.poses.rend(); pose++){
        double dx = pose->pose.position.x - robot_pose.pose.position.x;
        double dy = pose->pose.position.y - robot_pose.pose.position.y;
        double dz = pose->pose.position.z - robot_pose.pose.position.z;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        if(distance > step_size)
            next_pose = *pose;
        else
            break;
    }
    return next_pose;    
}
}

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<skid_steer_motion::PidMotionPlanner>("pid_motion_planner");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}