#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
using std::placeholders::_1;
class SimpleQoSSubscriber : public rclcpp::Node{
public:
    SimpleQoSSubscriber() : Node("Simple_QoS_subscriber"), qos_profile_subscriber{10}{
        declare_parameter<std::string>("reliability", "system_default");
        declare_parameter<std::string>("durability", "system_default");

        const auto reliability = get_parameter("reliability").as_string();
        const auto durability = get_parameter("durability").as_string();
        
        if(reliability == "best_effort"){
            qos_profile_subscriber.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
            RCLCPP_INFO(get_logger(), "Reliability ==> Best effort");
        }else if (reliability == "reliable"){
            qos_profile_subscriber.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
            RCLCPP_INFO(get_logger(), "Reliability ==> Reliable");
        }else if (reliability == "system_default"){
            qos_profile_subscriber.reliability(RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT);
            RCLCPP_INFO(get_logger(), "Reliability ==> System default");
        }else{
            RCLCPP_ERROR_STREAM(get_logger(), "Selected Reliability QoS: " << reliability << " doesn't exist");
            return;
        }

        if(durability == "volatile"){
            qos_profile_subscriber.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
            RCLCPP_INFO(get_logger(), "Durability ==> Volatile");
        }else if (durability == "transient_local"){
            qos_profile_subscriber.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);
            RCLCPP_INFO(get_logger(), "Durability ==> Transient local");
        }else if (durability == "system_default"){
            qos_profile_subscriber.durability(RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT);
            RCLCPP_INFO(get_logger(), "Durability ==> System default");
        }else{
            RCLCPP_ERROR_STREAM(get_logger(), "Selected Durability QoS: " << durability << " doesn't exist");
            return;
        }
        sub = create_subscription<std_msgs::msg::String>("Chatter", qos_profile_subscriber, std::bind(&SimpleQoSSubscriber::MsgCallback, this, _1));
    }
private:
    rclcpp::QoS qos_profile_subscriber;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub;
    void MsgCallback(const std_msgs::msg::String &msg) const{
        RCLCPP_INFO_STREAM(get_logger(), "I heard3232: " << msg.data.c_str());
    }
};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleQoSSubscriber>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}