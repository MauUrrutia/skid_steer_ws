#!/usr/bin/env python3
import rclpy 
import rclpy.time
from rclpy.node import Node
from nav_msgs.msg import Path
from geometry_msgs.msg import Twist, PoseStamped
from tf2_ros import Buffer, TransformListener
class pd_motion_planner(Node):
    def __init__(self):
        super().__init__("pd_motion_planner")
        self.declare_parameter("kp", 2.0)
        self.declare_parameter("kd", 0.1)
        self.declare_parameter("step_size", 0.2)
        self.declare_parameter("max_linear_velocity", 3.0)
        self.declare_parameter("max_angular_velocity", 1.5)

        self.kp = self.get_parameter("kp").value
        self.kd = self.get_parameter("kd").value
        self.step_size = self.get_parameter("step_size").value
        self.max_linear_velocity = self.get_parameter("max_linear_velocity").value
        self.max_angular_velocity = self.get_parameter("max_angular_velocity").value


        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.path_subscriber = self.create_subscription(Path, "/a_star/path", self.path_callback, 10)
        self.command_publisher = self.create_publisher(Twist, "/cmd_vel_nav", 10)
        self.next_pose_publisher = self.create_publisher(PoseStamped, "/pd/next_pose", 10)

        self.timer = self.create_timer(0.1, self.control_loop)
        
        self.global_plan = None

    def path_callback(self, path: Path):
        self.global_plan = path

    def control_loop(self):
        if not self.global_plan or not self.global_plan.poses:
            return
        try:
            robot_pose_transform = self.tf_buffer.lookup_transform("odom", "base_footprint", rclpy.time.Time())
        except Exception as e:
            self.get_logger().warn(f"Could not transform: {e}")
            return
        
        self.get_logger().info(f"Frame_id robot pose: {robot_pose_transform.header.frame_id}")
        self.get_logger().info(f"Frame_id global plan: {self.global_plan.header.frame_id}")

def main():
    rclpy.init()
    node = pd_motion_planner()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()