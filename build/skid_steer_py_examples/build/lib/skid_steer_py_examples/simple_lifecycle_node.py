import rclpy
from rclpy.lifecycle import Node, State, TransitionCallbackReturn
from std_msgs.msg import String
import time
class SimpleLifecycleNode(Node):
    def __init__(self, node_name, **kwargs):
        super().__init__(node_name, **kwargs)

    def on_configure(self, state: State) -> TransitionCallbackReturn:
        self.subscriber = self.create_subscription(String, "chatter", self.msg_callback, 10)
        self.get_logger().info("Lifecycle node on_configure() state called")
        return TransitionCallbackReturn.SUCCESS
    
    def on_shutdown(self, state: State) -> TransitionCallbackReturn:
        self.destroy_subscription(self.subscriber)
        self.get_logger().info("Lifecycle node on_shutdown() state called")
        return TransitionCallbackReturn.SUCCESS
    
    def on_cleanup(self, state: State) -> TransitionCallbackReturn:
        self.destroy_subscription(self.subscriber)
        self.get_logger().info("Lifecycle node on_cleanup() state called")
        return TransitionCallbackReturn.SUCCESS
    
    def on_activate(self, state: State) -> TransitionCallbackReturn:
        self.get_logger().info("Lifecycle node on_activate() state called")
        time.sleep(2)
        return super().on_activate(state)   
    
    def on_deactivate(self, state: State) -> TransitionCallbackReturn:
        self.get_logger().info("Lifecycle node on_deactivate() state called")
        return super().on_deactivate(state)
    
    def msg_callback(self, msg):
        current_state = self._state_machine.current_state
        if current_state[1]== "active":
            self.get_logger().info("msg called: %s" % msg.data)


def main():
    rclpy.init()
    executor = rclpy.executors.SingleThreadedExecutor()
    simple_lyfecycle_node = SimpleLifecycleNode("simple_lifecycle_node")
    executor.add_node(simple_lyfecycle_node)
    try:
        executor.spin()
    except (KeyboardInterrupt, rclpy.executors.ExternalShutdownException):
        simple_lyfecycle_node.destroy_node()


if __name__ == '__main__':
    main()
