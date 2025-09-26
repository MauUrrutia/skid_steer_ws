#!/usr/bin/env python3

import rclpy
import rclpy.time
from rclpy.qos import QoSProfile, DurabilityPolicy
from rclpy.node import Node
from nav_msgs.msg import OccupancyGrid, Path
from geometry_msgs.msg import PoseStamped, Pose

from tf2_ros import Buffer, TransformListener, LookupException

from queue import PriorityQueue

class graph_node:
    def __init__(self, x, y, cost = 0, previus_node = None):
        self.x = x
        self.y = y
        self.cost = cost
        self.previus_node = previus_node

    def __lt__(self, other):
        return self.cost < other.cost
    
    def __eq__(self, other):
        return self.x == other.x and self.y == other.y
    
    def __hash__(self):
        return hash((self.x, self.y))
    
    def __add__(self, other):
        return graph_node(self.x + other[0], self.y + other[1])

class dijkstra_planner(Node):
    def __init__(self):
        super().__init__("dijkstra_planner")

        self.map_qos = QoSProfile(depth = 10, )
        self.map_qos.durability = DurabilityPolicy.TRANSIENT_LOCAL
        self.map_subscriber = self.create_subscription(OccupancyGrid, "/map", self.map_callback, self.map_qos)
        self.pose_subscription = self.create_subscription(PoseStamped, "/goal_pose", self.goal_callback, 10)
        self.path_publisher = self.create_publisher(Path, "/dijkstra/path", 10)
        self.map_publisher = self.create_publisher(OccupancyGrid, "/dijkstra/visited_map", 10)

        self.map = None
        self.visited_map = OccupancyGrid()

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)


    def map_callback(self, map_msg: OccupancyGrid):
        self.map = map_msg
        self.visited_map.header.frame_id = map_msg.header.frame_id
        self.visited_map.info = map_msg.info
        self.visited_map.data = [-1] * (map_msg.info.height * map_msg.info.width)

    def goal_callback(self, pose_msg: PoseStamped):
        if self.map is None:
            self.get_logger().error("No map recieved")
            return
        
        self.visited_map.data = [-1] * (self.map.info.height * self.map.info.width)
        
        try:
            map_to_base_footprint_tf = self.tf_buffer.lookup_transform(self.map.header.frame_id, "base_footprint", rclpy.time.Time())
        except LookupException:
            self.get_logger().error("Could not transform from map to base_footprint")
            return
        
        map_to_base_pose = Pose()
        map_to_base_pose.position.x = map_to_base_footprint_tf.transform.translation.x
        map_to_base_pose.position.y = map_to_base_footprint_tf.transform.translation.y
        map_to_base_pose.position.z = map_to_base_footprint_tf.transform.translation.z
        map_to_base_pose.orientation = map_to_base_footprint_tf.transform.rotation

        path = self.plan(map_to_base_pose, pose_msg.pose)

        if path.poses:
            self.get_logger().info("Shortest path found")
            self.path_publisher.publish(path)
        else:
            self.get_logger().warn("No path found to the goal")

    
    def plan(self, start, goal):
        explore_directions = [(-1, 0), (1, 0), (0, 1), (0, -1), (1, 1), (-1, 1), (-1, -1), (1, -1)]
        pending_nodes = PriorityQueue()
        visited_nodes = set()
        start_node = self.world_to_grid(start)
        pending_nodes.put(start_node)

        while not pending_nodes.empty() and rclpy.ok():
            active_node = pending_nodes.get()

            if active_node == self.world_to_grid(goal):
                break

            for dir_x, dir_y in explore_directions:
                new_node: graph_node = active_node + (dir_x, dir_y)
                if new_node not in visited_nodes and self.pose_to_map(new_node) and self.map.data[self.pose_to_cell(new_node)] == 0:
                    new_node.cost = active_node.cost + 1
                    new_node.previus_node = active_node
                    pending_nodes.put(new_node)
                    visited_nodes.add(new_node)

            self.visited_map.data[self.pose_to_cell(active_node)] = 10
            self.map_publisher.publish(self.visited_map)

        path = Path()
        path.header.frame_id = self.map.header.frame_id
        while active_node and active_node.previus_node and rclpy.ok():
            last_pose: Pose = self.grid_to_world(active_node) 
            last_pose_stamped = PoseStamped()
            last_pose_stamped.header.frame_id = self.map.header.frame_id
            last_pose_stamped.pose = last_pose
            path.poses.append(last_pose_stamped)
            active_node = active_node.previus_node
        
        path.poses.reverse()
        return path
    

    def world_to_grid(self, pose_msg: Pose)->graph_node:
        grid_x = int((pose_msg.position.x - self.map.info.origin.position.x) / self.map.info.resolution)
        grid_y = int((pose_msg.position.y - self.map.info.origin.position.y) / self.map.info.resolution)
        return graph_node(grid_x, grid_y)

    
    def grid_to_world(self, node: graph_node)->Pose:
        pose = Pose()
        pose.position.x = node.x * self.map.info.resolution + self.map.info.origin.position.x
        pose.position.y = node.y * self.map.info.resolution + self.map.info.origin.position.y
        return pose
    

    def pose_to_map(self, node: graph_node)->bool:
        return self.map.info.width > node.x >= 0 and self.map.info.height > node.y >= 0 

    def pose_to_cell(self, node: graph_node)->int:
        return node.y * self.map.info.width + node.x

    

def main():
    rclpy.init()
    node = dijkstra_planner()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
        main()