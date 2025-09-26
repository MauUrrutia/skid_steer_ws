import rclpy
from rclpy.node import Node
from turtlesim.msg import Pose
import math

class SimpleTurtlesimKinematics(Node):
    def __init__(self):
        super().__init__("simple_turtlesim_kinematics")
        self.turtle1_pose_subscription = self.create_subscription(Pose, "/turtle1/pose", self.turtle1_pose_callback, 10)
        self.turtle2_pose_subscription = self.create_subscription(Pose, "/turtle2/pose", self.turtle2_pose_callback, 10)
        self.last_turtle1_pose = Pose()
        self.last_turtle2_pose = Pose()

    def turtle1_pose_callback(self, msg):
        self.last_turtle1_pose = msg


    def turtle2_pose_callback(self, msg):
         self.last_turtle2_pose = msg
         Tx = self.last_turtle2_pose.x - self.last_turtle1_pose.x
         Ty = self.last_turtle2_pose.y - self.last_turtle1_pose.y
         Theta_rad = self.last_turtle2_pose.theta - self.last_turtle1_pose.theta
         Theta_deg = 180 * Theta_rad / 3.1416

         self.get_logger().info("""
                                \n=======Translation vector Turtle1 -----> Turtle2 \n 
                                Tx: %f  \n 
                                Ty: %f  \n
                                =======Rotation matrix=======     
                                Theta[rad]: %f \n 
                                Theta[deg]: %f \n
                                [R11    R12]: [%f   %f]\n
                                [R21    R22]: [%f   %f]""" % (Tx, Ty, Theta_rad, Theta_deg, math.cos(Theta_rad), 
                                                              -math.sin(Theta_rad), math.sin(Theta_rad), math.cos(Theta_rad)))
         
def main():
    rclpy.init()
    simple_turtlesim_kinematics = SimpleTurtlesimKinematics()
    rclpy.spin(simple_turtlesim_kinematics)
    simple_turtlesim_kinematics.destroy_node()
    rclpy.shutdown()
if __name__ == '__main__':
    main()
