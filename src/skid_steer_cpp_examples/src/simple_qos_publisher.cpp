#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <chrono>
using namespace std::chrono_literals;
class SimpleQoSPublisher : public rclcpp::Node{
public:
    SimpleQoSPublisher() : Node("simple_QoS_publisher"), qos_profile_publisher{10}, counter{0}
    {
        declare_parameter<std::string>("reliability", "system_default");
        declare_parameter<std::string>("durability", "system_default");

        const auto reliability = get_parameter("reliability").as_string();
        const auto durability = get_parameter("durability").as_string();
        
        if(reliability == "best_effort"){
            qos_profile_publisher.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
            RCLCPP_INFO(get_logger(), "Reliability ==> Best effort");
        }else if (reliability == "reliable"){
            qos_profile_publisher.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
            RCLCPP_INFO(get_logger(), "Reliability ==> Reliable");
        }else if (reliability == "system_default"){
            qos_profile_publisher.reliability(RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT);
            RCLCPP_INFO(get_logger(), "Reliability ==> System default");
        }else{
            RCLCPP_ERROR_STREAM(get_logger(), "Selected Reliability QoS: " << reliability << " doesn't exist");
            return;
        }

        if(durability == "volatile"){
            qos_profile_publisher.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
            RCLCPP_INFO(get_logger(), "Durability ==> Volatile");
        }else if (durability == "transient_local"){
            qos_profile_publisher.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
            RCLCPP_INFO(get_logger(), "Durability ==> Transient local");
        }else if (durability == "system_default"){
            qos_profile_publisher.durability(RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT);
            RCLCPP_INFO(get_logger(), "Durability ==> System default");
        }else{
            RCLCPP_ERROR_STREAM(get_logger(), "Selected Durability QoS: " << durability << " doesn't exist");
            return;
        }
        
        
        
        pub = create_publisher<std_msgs::msg::String>("Chatter", qos_profile_publisher);
        timer = create_wall_timer(1s, std::bind(&SimpleQoSPublisher::TimerCallback, this));
        RCLCPP_INFO(get_logger(), "Publishing at 1 Hz");
    }

private:
    rclcpp::QoS qos_profile_publisher;
    unsigned int counter;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub;
    rclcpp::TimerBase::SharedPtr timer;

    void TimerCallback(){
        auto message = std_msgs::msg::String();
        message.data = "Hello ROS2 - counter: " + std::to_string(counter++);
        pub->publish(message);
    }
};


int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleQoSPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}