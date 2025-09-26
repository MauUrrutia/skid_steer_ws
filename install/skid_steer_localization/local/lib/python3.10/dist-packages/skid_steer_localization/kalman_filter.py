#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu
class KalmanFilter(Node):
    def __init__(self):
        super().__init__("kalman_filter")

        self.odom_subscriber = self.create_subscription(Odometry, "skid_steer_controller/odom_noisy", self.odom_callback, 10)
        self.imu_subscriber = self.create_subscription(Imu, "imu/out", self.imu_callback, 10)
        self.odom_publisher = self.create_publisher(Odometry, "skid_steer_controller/odom_filtered", 10)

        self.mean = 0.0
        self.variance = 1000.0
        self.imu_vyaw = 0.0
        self.is_first_odom = True
        self.last_vyaw = 0.0

        self.motion = 0.0
        self.odom_filtered = Odometry()
        
        self.motion_variance = 4.0
        self.measurement_variance = 0.5


    def update_measurements(self):
        self.mean = (self.measurement_variance * self.mean * self.variance * self.imu_vyaw) / (self.variance + self.measurement_variance)
        self.variance = (self.variance * self. measurement_variance) / (self.variance + self.measurement_variance)


    def state_prediction(self):
        self.mean = self.mean + self.motion
        self.variance = self.variance + self.motion_variance

    def imu_callback(self, imu):    
        self.imu_vyaw = imu.angular_velocity.z


    def odom_callback(self, odom):
        self.odom_filtered = odom
        if self.is_first_odom:
            self.mean = odom.twist.twist.angular.z
            self.last_vyaw = odom.twist.twist.angular.z
            self.is_first_odom = False
            return
        self.motion = odom.twist.twist.angular.z - self.last_vyaw

        self.state_prediction()

        self.update_measurements()

        self.odom_filtered.twist.twist.angular.z = self.mean
        self.odom_publisher.publish(self.odom_filtered)


def main():
    rclpy.init()
    kalman_filter = KalmanFilter()
    rclpy.spin(kalman_filter)
    kalman_filter.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()