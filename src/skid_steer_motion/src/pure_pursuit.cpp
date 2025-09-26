#include "skid_steer_motion/pure_pursuit.hpp"
#include <tf2/utils.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

namespace skid_steer_motion
{

PurePursuit::~PurePursuit(){}

PurePursuit::PurePursuit() : Node("pure_pursuit"),
    look_ahead_distance(0.3),
    max_linear_vel(0.3),
    max_angular_vel(1.0),
    goal_tolerance(0.1),
    min_linear_vel(0.05)
{
    // Declarar parámetros
    declare_parameter<double>("look_ahead_distance", look_ahead_distance);
    declare_parameter<double>("max_linear_vel", max_linear_vel);
    declare_parameter<double>("max_angular_vel", max_angular_vel);
    declare_parameter<double>("goal_tolerance", goal_tolerance);
    declare_parameter<double>("min_linear_vel", min_linear_vel);

    // Obtener parámetros
    look_ahead_distance = get_parameter("look_ahead_distance").as_double();
    max_linear_vel = get_parameter("max_linear_vel").as_double();
    max_angular_vel = get_parameter("max_angular_vel").as_double();
    goal_tolerance = get_parameter("goal_tolerance").as_double();
    min_linear_vel = get_parameter("min_linear_vel").as_double();

    // Suscriptores y publicadores
    path_subscriber = create_subscription<nav_msgs::msg::Path>(
        "/a_star/path", 10, 
        std::bind(&PurePursuit::PathCallback, this, std::placeholders::_1)
    );

    cmd_vel_publisher = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    check_point_pose_publisher = create_publisher<geometry_msgs::msg::PoseStamped>("/pure_pursuit/check_point", 10);

    // TF buffer y listener
    tf_buffer = std::make_shared<tf2_ros::Buffer>(get_clock());
    tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

    // Timer de control
    control_loop = create_wall_timer(
        std::chrono::milliseconds(100),
        std::bind(&PurePursuit::ControlLoop, this)
    );

    RCLCPP_INFO(get_logger(), "Pure Pursuit initialized");
}
    
void PurePursuit::PathCallback(const nav_msgs::msg::Path::SharedPtr path){
    global_plan = *path;
    RCLCPP_INFO(get_logger(), "New path received with %zu points", global_plan.poses.size());
}   

void PurePursuit::ControlLoop(){
    if (global_plan.poses.empty()) {
        return;
    }
    
   
    geometry_msgs::msg::TransformStamped robot_transform;
    try {
        robot_transform = tf_buffer->lookupTransform(
            "odom", "base_footprint", rclcpp::Time(0)
        );
    } catch (tf2::TransformException &e) {
        RCLCPP_WARN(get_logger(), "Could not transform: %s", e.what());
        return;
    }
    

    geometry_msgs::msg::PoseStamped robot_pose;
    robot_pose.header = robot_transform.header;
    robot_pose.pose.position.x = robot_transform.transform.translation.x;
    robot_pose.pose.position.y = robot_transform.transform.translation.y;
    robot_pose.pose.position.z = robot_transform.transform.translation.z;
    robot_pose.pose.orientation = robot_transform.transform.rotation;


    if (!TransformPlan(robot_pose.header.frame_id)) {
        RCLCPP_ERROR(get_logger(), "Unable to transform plan to robot frame");
        return;
    }


    bool goal_reached = false;
    auto lookahead_point = GetLookaheadPoint(robot_pose, goal_reached);
    
    if (goal_reached) {
        RCLCPP_INFO(get_logger(), "Goal reached!");
        geometry_msgs::msg::Twist stop_cmd;
        cmd_vel_publisher->publish(stop_cmd);
        global_plan.poses.clear();
        return;
    }

    geometry_msgs::msg::PoseStamped lookahead_pose;
    lookahead_pose.header = robot_pose.header;
    lookahead_pose.pose.position = lookahead_point;
    check_point_pose_publisher->publish(lookahead_pose);

    double curvature = CalculateCurvature(robot_pose, lookahead_point);
    auto cmd_vel = CalculateVelocities(curvature, robot_pose);
    
    cmd_vel_publisher->publish(cmd_vel);
}

bool PurePursuit::TransformPlan(const std::string &frame){
    if (global_plan.header.frame_id == frame) {
        return true;
    }
    
    try {
        auto transform = tf_buffer->lookupTransform(
            frame, global_plan.header.frame_id, rclcpp::Time(0)
        );
        
        for (auto &pose : global_plan.poses) {
            tf2::doTransform(pose, pose, transform);
        }
        global_plan.header.frame_id = frame;
        return true;
        
    } catch (tf2::TransformException &e) {
        RCLCPP_ERROR(get_logger(), "Transform error: %s", e.what());
        return false;
    }
}

geometry_msgs::msg::Point PurePursuit::GetLookaheadPoint(
    const geometry_msgs::msg::PoseStamped &robot_pose, bool &goal_reached)
{
    goal_reached = false;
    
    // Verificar si alcanzamos el goal
    auto goal_pose = global_plan.poses.back();
    double goal_dx = goal_pose.pose.position.x - robot_pose.pose.position.x;
    double goal_dy = goal_pose.pose.position.y - robot_pose.pose.position.y;
    double goal_distance = std::sqrt(goal_dx*goal_dx + goal_dy*goal_dy);
    
    if (goal_distance < goal_tolerance) {
        goal_reached = true;
        return geometry_msgs::msg::Point();
    }

    // Encontrar el punto más cercano en el path
    size_t closest_idx = 0;
    double closest_dist = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < global_plan.poses.size(); ++i) {
        double dx = global_plan.poses[i].pose.position.x - robot_pose.pose.position.x;
        double dy = global_plan.poses[i].pose.position.y - robot_pose.pose.position.y;
        double distance = std::sqrt(dx*dx + dy*dy);
        
        if (distance < closest_dist) {
            closest_dist = distance;
            closest_idx = i;
        }
    }

    // Buscar el punto de mira
    for (size_t i = closest_idx; i < global_plan.poses.size(); ++i) {
        double dx = global_plan.poses[i].pose.position.x - robot_pose.pose.position.x;
        double dy = global_plan.poses[i].pose.position.y - robot_pose.pose.position.y;
        double distance = std::sqrt(dx*dx + dy*dy);
        
        if (distance >= look_ahead_distance) {
            return global_plan.poses[i].pose.position;
        }
    }

    // Si no se encuentra, usar el último punto
    return global_plan.poses.back().pose.position;
}

double PurePursuit::CalculateCurvature(
    const geometry_msgs::msg::PoseStamped &robot_pose,
    const geometry_msgs::msg::Point &lookahead_point)
{
    // Obtener el yaw del robot
    double roll, pitch, yaw;
    tf2::Quaternion q(
        robot_pose.pose.orientation.x,
        robot_pose.pose.orientation.y,
        robot_pose.pose.orientation.z,
        robot_pose.pose.orientation.w
    );
    tf2::Matrix3x3 m(q);
    m.getRPY(roll, pitch, yaw);

    // Vector desde robot al punto de mira
    double dx = lookahead_point.x - robot_pose.pose.position.x;
    double dy = lookahead_point.y - robot_pose.pose.position.y;

    // Transformar a coordenadas del robot
    double robot_dx = dx * std::cos(-yaw) - dy * std::sin(-yaw);
    double robot_dy = dx * std::sin(-yaw) + dy * std::cos(-yaw);

    // Calcular curvatura
    double L_squared = robot_dx*robot_dx + robot_dy*robot_dy;
    if (L_squared > 0.001) {
        return 2.0 * robot_dy / L_squared;
    }
    
    return 0.0;
}

geometry_msgs::msg::Twist PurePursuit::CalculateVelocities(
    double curvature, const geometry_msgs::msg::PoseStamped &robot_pose)
{
    geometry_msgs::msg::Twist cmd_vel;
    
    // Velocidad angular
    double angular_vel = curvature * max_linear_vel;
    angular_vel = std::clamp(angular_vel, -max_angular_vel, max_angular_vel);
    
    // Velocidad lineal base
    double linear_vel = max_linear_vel;
    
    // Reducir velocidad en curvas cerradas
    if (std::abs(curvature) > 0.5) {
        linear_vel *= 0.5;
    }
    
    // Reducir velocidad cerca del goal
    auto goal_pose = global_plan.poses.back();
    double goal_dx = goal_pose.pose.position.x - robot_pose.pose.position.x;
    double goal_dy = goal_pose.pose.position.y - robot_pose.pose.position.y;
    double goal_distance = std::sqrt(goal_dx*goal_dx + goal_dy*goal_dy);
    
    if (goal_distance < 0.5) {
        linear_vel *= (goal_distance / 0.5);
        linear_vel = std::max(linear_vel, min_linear_vel);
    }
    
    // Velocidad mínima para girar
    if (std::abs(angular_vel) > 0.1 && linear_vel < min_linear_vel * 2) {
        linear_vel = min_linear_vel * 2;
    }
    
    cmd_vel.linear.x = linear_vel;
    cmd_vel.angular.z = angular_vel;
    
    return cmd_vel;
}

}

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<skid_steer_motion::PurePursuit>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}