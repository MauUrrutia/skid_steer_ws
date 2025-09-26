#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import time
imu_publisher = None


def imu_callback(imu):
    global imu_publisher
    imu.header.frame_id = "base_footprint_ekf"
    imu_publisher.publish(imu)


def main():
    global imu_publisher
    rclpy.init()
    node = Node("imu_republisher_node")
    time.sleep(1)

    imu_publisher = node.create_publisher(Imu, "imu_ekf", 10)
    imu_subscriber = node.create_subscription(Imu, "imu/out", imu_callback, 10)

    rclpy.spin(node)
    rclpy.shutdown()
if __name__ == '__main__':
    main()