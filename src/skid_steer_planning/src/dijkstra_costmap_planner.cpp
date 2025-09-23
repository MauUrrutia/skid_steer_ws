#include "skid_steer_planning/dijkstra_costmap_planner.hpp"

namespace skid_steer_planning
{
DijkstraCostmapPlanner::~DijkstraCostmapPlanner(){}

DijkstraCostmapPlanner::DijkstraCostmapPlanner(const std::string &name):Node(name){

    tf_buffer = std::make_unique<tf2_ros::Buffer>(get_clock()); 
    tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);
    map_subscriber = create_subscription<nav_msgs::msg::OccupancyGrid>("/costmap/costmap", rclcpp::QoS(rclcpp::KeepLast(10)).transient_local().reliable(), 
                     std::bind(&DijkstraCostmapPlanner::map_callback, this, std::placeholders::_1));
    
    pose_subscriber = create_subscription<geometry_msgs::msg::PoseStamped>("/goal_pose", 10, 
                      std::bind(&DijkstraCostmapPlanner::goal_callback, this, std::placeholders::_1));
    
    path_publisher = create_publisher<nav_msgs::msg::Path>("/dijkstra/path", 10);
    map_publisher = create_publisher<nav_msgs::msg::OccupancyGrid>("/dijkstra/visited_map", 10);


}
void DijkstraCostmapPlanner::map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr map){
    this->map = map;
    visited_map.header.frame_id = map->header.frame_id;
    visited_map.info = map->info;
    visited_map.data = std::vector<int8_t>(visited_map.info.height * visited_map.info.width, -1);
}
void DijkstraCostmapPlanner::goal_callback(const geometry_msgs::msg::PoseStamped::SharedPtr pose){
    if (!map){
        RCLCPP_ERROR(get_logger(), "no map recieved");
        return;
    }

    visited_map.data = std::vector<int8_t>(visited_map.info.height * visited_map.info.width, -1);
    geometry_msgs::msg::TransformStamped map_to_base_footprint_tf;

    try{
       map_to_base_footprint_tf = tf_buffer->lookupTransform(map->header.frame_id, "base_footprint", tf2::TimePointZero);
    }
    catch(const tf2::TransformException &e){
        RCLCPP_ERROR(get_logger(), "could not transform from map to base_footprint");
        return;
    }
    
    geometry_msgs::msg::Pose map_to_base_pose;
    map_to_base_pose.position.x = map_to_base_footprint_tf.transform.translation.x;
    map_to_base_pose.position.y = map_to_base_footprint_tf.transform.translation.y;
    map_to_base_pose.position.z = map_to_base_footprint_tf.transform.translation.z;
    map_to_base_pose.orientation = map_to_base_footprint_tf.transform.rotation;

    auto path = plan_trajectory(map_to_base_pose, pose->pose);
        if(!path.poses.empty()){
            RCLCPP_INFO(get_logger(), "Shortest path found");
            path_publisher->publish(path);
        } else{
            RCLCPP_WARN(get_logger(), "No path found to the goal");
        }
}



nav_msgs::msg::Path DijkstraCostmapPlanner::plan_trajectory(const geometry_msgs::msg::Pose &start, const geometry_msgs::msg::Pose &goal){
    
    std::vector<std::pair<int, int>> explore_vecinity = {
        {0, 1}, {-1, 0}, {0, -1}, {1, 0}, {1, 1}, {-1, 1}, {-1, -1}, {1, -1}
    };
    
    std::priority_queue<GraphNode, std::vector<GraphNode>, std::greater<GraphNode>> pending_nodes;
    std::vector<GraphNode> visited_nodes;
    pending_nodes.push(WorldToGrid(start));
    GraphNode active_node;

    while (!pending_nodes.empty() && rclcpp::ok()){
        active_node = pending_nodes.top();
        pending_nodes.pop();
        
        if (active_node==WorldToGrid(goal))
            break;

        for (const auto &dir : explore_vecinity){
            GraphNode new_node = active_node + dir;
            if(std::find(visited_nodes.begin(), visited_nodes.end(), new_node) == visited_nodes.end() 
                    && PoseOnMap(new_node) && map->data.at(PoseToCell(new_node)) < 99 && map->data.at(PoseToCell(new_node)) >= 0)
            {
                new_node.cost = active_node.cost + 1 + map->data.at(PoseToCell(new_node));
                new_node.previus_node = std::make_shared<GraphNode>(active_node);
                pending_nodes.push(new_node);
                visited_nodes.push_back(new_node);
            }
        }
        
        visited_map.data.at(PoseToCell(active_node)) = 10;
        map_publisher->publish(visited_map);
    }

    nav_msgs::msg::Path path;
    path.header.frame_id = map->header.frame_id;
    while (active_node.previus_node && rclcpp::ok()){
        geometry_msgs::msg::Pose last_pose = GridToWorld(active_node);
        geometry_msgs::msg::PoseStamped last_pose_stamped;
        last_pose_stamped.header.frame_id = map->header.frame_id;
        last_pose_stamped.pose = last_pose;
        path.poses.push_back(last_pose_stamped);
        active_node = *active_node.previus_node;
    }
    
    std::reverse(path.poses.begin(), path.poses.end());
    return path;
    
}

bool DijkstraCostmapPlanner::PoseOnMap(const GraphNode &node){
    return node.x >= 0 && node.x < static_cast<int>(map->info.width) && node.y >= 0 && node.y < static_cast<int>(map->info.height);
}

unsigned int DijkstraCostmapPlanner::PoseToCell(const GraphNode &node){
    return node.y * map->info.width + node.x;
}


geometry_msgs::msg::Pose DijkstraCostmapPlanner::GridToWorld(const GraphNode &node){
    geometry_msgs::msg::Pose pose;
    pose.position.x = node.x * map->info.resolution + map->info.origin.position.x;
    pose.position.y = node.y * map->info.resolution + map->info.origin.position.y;
    return pose;
}


GraphNode DijkstraCostmapPlanner::WorldToGrid(const geometry_msgs::msg::Pose &pose){
    int grid_x = static_cast<int>((pose.position.x - map->info.origin.position.x) / map->info.resolution);
    int grid_y = static_cast<int>((pose.position.y - map->info.origin.position.y) / map->info.resolution);
    return GraphNode(grid_x, grid_y);
}

}


int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<skid_steer_planning::DijkstraCostmapPlanner>("dijkstra_planner");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}