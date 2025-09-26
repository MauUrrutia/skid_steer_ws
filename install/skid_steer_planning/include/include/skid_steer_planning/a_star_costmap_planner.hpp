#ifndef _A_STAR_COSTMAP_PLANNER_HPP_
#define _A_STAR_COSTMAP_PLANNER_HPP_


#include <rclcpp/rclcpp.hpp>
#include <rmw/qos_profiles.h>
#include <nav_msgs/msg/path.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

#include <queue>
namespace skid_steer_planning
{


struct GraphNode
{
    int x;
    int y;
    double heuristic;
    int cost;
    std::shared_ptr<GraphNode> previus_node;

    GraphNode(int in_x, int in_y): x(in_x), y(in_y), cost(0){

    }
    GraphNode(): GraphNode(0, 0){

    }

    bool operator>(const GraphNode &r_hs) const{
        return (cost + heuristic) > (r_hs.cost + r_hs.heuristic);
    }

    bool operator==(const GraphNode &r_hs) const{
        return x == r_hs.x && y == r_hs.y;
    }

    GraphNode operator+(std::pair<int, int> const &r_hs){
        GraphNode res(x + r_hs.first, y + r_hs.second);
        return res;
    }
    
};


class AStarCostmapPlanner: public rclcpp::Node
{
private:
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr map_subscriber;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_subscriber;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr path_publisher;
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr map_publisher;

    nav_msgs::msg::OccupancyGrid::SharedPtr map;
    nav_msgs::msg::OccupancyGrid visited_map;

    std::shared_ptr<tf2_ros::TransformListener> tf_listener;
    std::unique_ptr<tf2_ros::Buffer> tf_buffer;

    void map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr map);
    void goal_callback(const geometry_msgs::msg::PoseStamped::SharedPtr pose);

    nav_msgs::msg::Path plan_trajectory(const geometry_msgs::msg::Pose &start, const geometry_msgs::msg::Pose &goal);

    GraphNode WorldToGrid(const geometry_msgs::msg::Pose &pose);
    geometry_msgs::msg::Pose GridToWorld(const GraphNode &node);
    bool PoseOnMap(const GraphNode &node);

    unsigned int PoseToCell(const GraphNode &node);

    double ChebyshevDistance(const GraphNode &node, const GraphNode &goal_node);
public:
    AStarCostmapPlanner(const std::string &name);
    ~AStarCostmapPlanner();


};
}

#endif