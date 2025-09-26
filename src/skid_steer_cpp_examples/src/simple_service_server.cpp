#include <rclcpp/rclcpp.hpp>
#include <skid_steer_msgs/srv/add_two_ints.hpp>
class SimpleServiceServer : public rclcpp::Node{
private:
    rclcpp::Service<skid_steer_msgs::srv::AddTwoInts>::SharedPtr service;
    void service_callback(std::shared_ptr<skid_steer_msgs::srv::AddTwoInts::Request> request, std::shared_ptr<skid_steer_msgs::srv::AddTwoInts::Response> response){
        RCLCPP_INFO_STREAM(get_logger(), "New request received a: " << request->a << " b: " << request->b);
        response->sum = request->a + request->b;
        RCLCPP_INFO_STREAM(get_logger(), "Response sum: " << response->sum );
    }
public:
    SimpleServiceServer(): Node("simple_service_server"){
        service = create_service<skid_steer_msgs::srv::AddTwoInts>("add_two_ints", 
                                std::bind(&SimpleServiceServer::service_callback, this, std::placeholders::_1, std::placeholders::_2));
        RCLCPP_INFO_STREAM(get_logger(), "Service add_two_ints ready");                            
        
    }
};

int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleServiceServer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}