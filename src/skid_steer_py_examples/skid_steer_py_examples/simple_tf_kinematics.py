import rclpy
from rclpy.node import Node
from tf2_ros.static_transform_broadcaster import StaticTransformBroadcaster 
from tf2_ros.transform_broadcaster import TransformBroadcaster 
from geometry_msgs.msg import TransformStamped
from skid_steer_msgs.srv import GetTransform
from tf2_ros.buffer import Buffer
from tf2_ros import TransformException
from tf2_ros.transform_listener import TransformListener
from tf_transformations import quaternion_from_euler, quaternion_multiply, quaternion_inverse

class SimpleTfKInematics(Node):
        def __init__(self):
            super().__init__("simple_tf_kinematics")
            self.static_tf_broadcaster = StaticTransformBroadcaster(self)
            self.dynamic_tf_broadcaster = TransformBroadcaster(self)
            self.dynamic_transform_stamped = TransformStamped()
            self.tf_buffer = Buffer()
            self.tf_listener = TransformListener(self.tf_buffer, self)
            self.x_increment = 0.05
            self.last_x = 0
            self.rotations_counter = 0
            self.last_orientation = quaternion_from_euler(0, 0 ,0)
            self.orientation_increment = quaternion_from_euler(0, 0.05 ,0)
            self.static_transform_stamped = TransformStamped()
            self.static_transform_stamped.header.stamp = self.get_clock().now().to_msg()
            self.static_transform_stamped.header.frame_id = "skid_steer_base"
            self.static_transform_stamped.child_frame_id = "skid_steer_top"
            self.static_transform_stamped.transform.translation.x = 0.0
            self.static_transform_stamped.transform.translation.y = 0.0
            self.static_transform_stamped.transform.translation.z = 0.3
            self.static_transform_stamped.transform.rotation.x = 0.0
            self.static_transform_stamped.transform.rotation.y = 0.0
            self.static_transform_stamped.transform.rotation.z = 0.0
            self.static_transform_stamped.transform.rotation.w = 1.0
            self.static_tf_broadcaster.sendTransform(self.static_transform_stamped)
            self.get_logger().info("Publishing static transform between %s and %s " 
                                    %(self.static_transform_stamped.header.frame_id, self.static_transform_stamped.child_frame_id))
            self.timer = self.create_timer(0.1, self.timer_callback)
            self.get_transform_server =  self.create_service(GetTransform, "get_transform", self.get_transform_callback)

        def timer_callback(self):
            self.dynamic_transform_stamped.header.stamp = self.get_clock().now().to_msg()
            self.dynamic_transform_stamped.header.frame_id = "odom"
            self.dynamic_transform_stamped.child_frame_id = "skid_steer_base"
            self.dynamic_transform_stamped.transform.translation.x = self.last_x + self.x_increment
            self.dynamic_transform_stamped.transform.translation.y = 0.0
            self.dynamic_transform_stamped.transform.translation.z = 0.0
            q = quaternion_multiply(self.last_orientation, self.orientation_increment)
            self.dynamic_transform_stamped.transform.rotation.x = q[0]
            self.dynamic_transform_stamped.transform.rotation.y = q[1]
            self.dynamic_transform_stamped.transform.rotation.z = q[2]
            self.dynamic_transform_stamped.transform.rotation.w = q[3]
            self.dynamic_tf_broadcaster.sendTransform(self.dynamic_transform_stamped)
            self.last_x = self.dynamic_transform_stamped.transform.translation.x
            self.rotations_counter += 1
            self.last_orientation = q

            if self.rotations_counter >= 100:
                self.orientation_increment = quaternion_inverse(self.orientation_increment)
                self.rotations_counter = 0


        def get_transform_callback(self, request, response):
            self.get_logger().info("Request transform between %s and %s" % (request.frame_id, request.child_frame_id))
            requested_transform = TransformStamped()
            try:
                requested_transform = self.tf_buffer.lookup_transform(request.frame_id, request.child_frame_id, rclpy.time.Time())
            except TransformException as e:
                  self.get_logger().error("An error ocurred while transforming %s and %s" % (request.frame_id, request.child_frame_id))
                  response.success = False
                  return response
            response.transform = requested_transform
            response.success = True
            return response


def main():
    rclpy.init()
    simple_tf_kinematics = SimpleTfKInematics()
    rclpy.spin(simple_tf_kinematics)
    simple_tf_kinematics.destroy_node()
    rclpy.shutdown()
if __name__ == '__main__':
    main()