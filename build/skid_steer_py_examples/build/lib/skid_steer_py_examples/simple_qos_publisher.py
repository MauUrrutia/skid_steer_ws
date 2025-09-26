import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy
from std_msgs.msg import String

class SimpleQoSPublisher(Node):
    def __init__(self):
        super().__init__("simple_qos_publisher")
        self.declare_parameter("reliability", "system_default")
        self.declare_parameter("durability", "system_default")


        self.qos_profile_publisher = QoSProfile(depth=10)
        self.reliability = self.get_parameter("reliability").get_parameter_value().string_value
        self.durability = self.get_parameter("durability").get_parameter_value().string_value

        if self.reliability == "best_effort":
            self.qos_profile_publisher.reliability = QoSReliabilityPolicy.BEST_EFFORT
            self.get_logger().info("Reliability ==> Best effort")
        elif self.reliability == "reliable":
            self.qos_profile_publisher.reliability = QoSReliabilityPolicy.RELIABLE
            self.get_logger().info("Reliability ==> Reliable")
        elif self.reliability == "system_default": 
            self.qos_profile_publisher.reliability = QoSReliabilityPolicy.SYSTEM_DEFAULT
            self.get_logger().info("Reliability ==> System Default")
        else:
            self.get_logger().info("Selected reliability QoS: %s doesn't exist" %(self.reliability))
            return
        
        if self.durability == "volatile":
            self.qos_profile_publisher.durability = QoSDurabilityPolicy.VOLATILE
            self.get_logger().info("Durability ==> Volatile")
        elif self.durability == "transient_local":
            self.qos_profile_publisher.durability = QoSDurabilityPolicy.TRANSIENT_LOCAL
            self.get_logger().info("Durability ==> transient_local")
        elif self.durability == "system_default": 
            self.qos_profile_publisher.durability = QoSDurabilityPolicy.SYSTEM_DEFAULT
            self.get_logger().info("Durability ==> System Default")
        else:
            self.get_logger().info("Selected durability QoS: %s doesn't exist" %(self.durability))
            return
        
        self.pub = self.create_publisher(String, "Chatter", self.qos_profile_publisher)
        self.counter = 0
        self.frequency = 1.0
        self.get_logger().info("Publishing at %d Hz" % self.frequency)
        self.timer = self.create_timer(self.frequency, self.TimerCallback)
    
    def TimerCallback(self):
        msg = String()
        msg.data = "Hello ROS2 - counter: %d" % self.counter
        self.pub.publish(msg)
        self.counter += 1

def main():
    rclpy.init()
    simple_qos_publisher = SimpleQoSPublisher()
    rclpy.spin(simple_qos_publisher)
    simple_qos_publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__name__':
    main()