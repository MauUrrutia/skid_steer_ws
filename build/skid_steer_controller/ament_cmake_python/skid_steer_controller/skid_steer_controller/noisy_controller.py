#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from geometry_msgs.msg import  TransformStamped
from nav_msgs.msg import Odometry
from rclpy.time import Time
from rclpy.constants import S_TO_NS
from tf_transformations import quaternion_from_euler
from tf2_ros import TransformBroadcaster
import numpy as np
import math

class NoisyController(Node):
    def __init__(self):
        super().__init__("simple_controller")

        self.declare_parameter("wheel_radius", 0.048)
        self.declare_parameter("wheel_separation", 0.665)

        self.wheel_radius = self.get_parameter("wheel_radius").get_parameter_value().double_value
        self.wheel_separation = self.get_parameter("wheel_separation").get_parameter_value().double_value

        self.get_logger().info("Using wheel_radius %f" % self.wheel_radius)
        self.get_logger().info("Using wheel_separation %f" % self.wheel_separation)

        self.left_wheels_array_previous_pos = np.array([0.0, 0.0, 0.0])
        self.right_wheels_array_previous_pos = np.array([0.0, 0.0, 0.0])
        self.previous_time = self.get_clock().now()

        self.x = 0
        self.y = 0
        self.theta = 0


        self.joint_subscriber = self.create_subscription(JointState, "joint_states", self.joint_callback, 10)
        self.odom_publisher = self.create_publisher(Odometry, "skid_steer_controller/odom_noisy", 10)

        
        self.odometry_msg = Odometry()
        self.odometry_msg.header.frame_id = "odom"
        self.odometry_msg.child_frame_id = "base_footprint_ekf"
        self.odometry_msg.pose.pose.orientation.x = 0.0
        self.odometry_msg.pose.pose.orientation.y = 0.0
        self.odometry_msg.pose.pose.orientation.z = 0.0
        self.odometry_msg.pose.pose.orientation.w = 1.0

        self.broadcaster = TransformBroadcaster(self)
        self.transform_stamped = TransformStamped()
        self.transform_stamped.header.frame_id = "odom"
        self.transform_stamped.child_frame_id = "base_footprint_noisy"



    
    def joint_callback(self, msg):
        left_wheel_encoder_array = np.array([msg.position[1], msg.position[3], msg.position[5]]) + np.random.normal(0, 0.005)
        right_wheel_encoder_array = np.array([msg.position[0], msg.position[2], msg.position[4]]) + np.random.normal(0, 0.005)
        dp_left_array = left_wheel_encoder_array - self.left_wheels_array_previous_pos
        dp_right_array = right_wheel_encoder_array - self.right_wheels_array_previous_pos
        dt = Time.from_msg(msg.header.stamp) - self.previous_time

        self.left_wheels_array_previous_pos = np.array([msg.position[1], msg.position[3], msg.position[5]])
        self.right_wheels_array_previous_pos = np.array([msg.position[0], msg.position[2], msg.position[4]])
        self.previous_time = Time.from_msg(msg.header.stamp)

        fi_left_array = dp_left_array / (dt.nanoseconds / S_TO_NS)
        fi_right_array = dp_right_array / (dt.nanoseconds / S_TO_NS)

        fi_left_avg = np.mean(fi_left_array) 
        fi_right_avg = np.mean(fi_right_array) 

        linear = (self.wheel_radius * fi_right_avg + self.wheel_radius * fi_left_avg) / 2
        angular = (self.wheel_radius * fi_right_avg - self.wheel_radius * fi_left_avg) / self.wheel_separation

        d_s_array = (self.wheel_radius * dp_right_array + self.wheel_radius * dp_left_array) / 2
        d_theta_array = (self.wheel_radius * dp_right_array - self.wheel_radius * dp_left_array) / self.wheel_separation
        d_s = np.mean(d_s_array) * np.size(d_s_array)
        d_theta = np.mean(d_theta_array) * np.size(d_theta_array)
        self.theta += d_theta
        self.x += d_s * math.cos(self.theta)
        self.y += d_s * math.sin(self.theta)

        q = quaternion_from_euler(0, 0, self.theta)
        self.odometry_msg.pose.pose.orientation.x = q[0]
        self.odometry_msg.pose.pose.orientation.y = q[1]
        self.odometry_msg.pose.pose.orientation.z = q[2]
        self.odometry_msg.pose.pose.orientation.w = q[3]
        self.odometry_msg.header.stamp = self.get_clock().now().to_msg()
        self.odometry_msg.pose.pose.position.x = self.x
        self.odometry_msg.pose.pose.position.y = self.y
        self.odometry_msg.pose.pose.position.z = 0.0
        self.odometry_msg.twist.twist.linear.x = linear
        self.odometry_msg.twist.twist.angular.z = angular

        self.transform_stamped.transform.translation.x = self.x
        self.transform_stamped.transform.translation.y = self.y
        self.transform_stamped.transform.translation.z = 0.0
        self.transform_stamped.transform.rotation.x = q[0]
        self.transform_stamped.transform.rotation.y = q[1]
        self.transform_stamped.transform.rotation.z = q[2]
        self.transform_stamped.transform.rotation.w = q[3]
        self.transform_stamped.header.stamp = self.get_clock().now().to_msg()
        self.odom_publisher.publish(self.odometry_msg)
        self.broadcaster.sendTransform(self.transform_stamped)


def main():
    rclpy.init()
    noisy_controller = NoisyController()
    rclpy.spin(noisy_controller)
    noisy_controller.destroy_node()
    rclpy.shutdown()
    
if __name__ == '__main__':
    main()
