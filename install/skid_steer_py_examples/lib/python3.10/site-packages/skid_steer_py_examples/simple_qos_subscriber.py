import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy
from std_msgs.msg import String

class SimpleQoSSubscriber(Node):
    def __init__(self):
        super().__init__("simple__qos_subscriber")
        self.declare_parameter("reliability", "system_default")
        self.declare_parameter("durability", "system_default")


        self.qos_profile_subscriber = QoSProfile(depth=10)
        self.reliability = self.get_parameter("reliability").get_parameter_value().string_value
        self.durability = self.get_parameter("durability").get_parameter_value().string_value

        if self.reliability == "best_effort":
            self.qos_profile_subscriber.reliability = QoSReliabilityPolicy.BEST_EFFORT
            self.get_logger().info("Reliability ==> Best effort")
        elif self.reliability == "reliable":
            self.qos_profile_subscriber.reliability = QoSReliabilityPolicy.RELIABLE
            self.get_logger().info("Reliability ==> Reliable")
        elif self.reliability == "system_default": 
            self.qos_profile_subscriber.reliability = QoSReliabilityPolicy.SYSTEM_DEFAULT
            self.get_logger().info("Reliability ==> System Default")
        else:
            self.get_logger().info("Selected reliability QoS: %s doesn't exist" %(self.reliability))
            return
        
        if self.durability == "volatile":
            self.qos_profile_subscriber.durability = QoSDurabilityPolicy.VOLATILE
            self.get_logger().info("Durability ==> Volatile")
        elif self.durability == "transient_local":
            self.qos_profile_subscriber.durability = QoSDurabilityPolicy.TRANSIENT_LOCAL
            self.get_logger().info("Durability ==> transient_local")
        elif self.durability == "system_default": 
            self.qos_profile_subscriber.durability = QoSDurabilityPolicy.SYSTEM_DEFAULT
            self.get_logger().info("Durability ==> System Default")
        else:
            self.get_logger().info("Selected durability QoS: %s doesn't exist" %(self.durability))
            return
        self.sub = self.create_subscription(String, "Chatter", self.MsgCallback, self.qos_profile_subscriber)
    def MsgCallback(self, msg):
        self.get_logger().info("I heard: %s" % msg.data)
   
def main():
    rclpy.init()
    simple__qos_subscriber = SimpleQoSSubscriber()
    rclpy.spin(simple__qos_subscriber)
    simple__qos_subscriber.destroy_node()
    rclpy.shutdown()

if __name__ == '__name__':
    main()