#include <rclcpp/rclcpp.hpp>
#include "skid_steer_msgs/srv/add_two_ints.hpp"

class SimpleServiceClient : public rclcpp::Node
{
private:
    rclcpp::Client<skid_steer_msgs::srv::AddTwoInts>::SharedPtr client;
public:
    SimpleServiceClient(int a, int b);
    void responseCallback(rclcpp::Client<skid_steer_msgs::srv::AddTwoInts>::SharedFuture);
    ~SimpleServiceClient();
};

SimpleServiceClient::SimpleServiceClient(int a, int b): Node("simple_service_client"){
    client = create_client<skid_steer_msgs::srv::AddTwoInts>("add_two_ints");
    auto request = std::make_shared<skid_steer_msgs::srv::AddTwoInts::Request>();
    request->a = a;
    request->b = b;
    while ( !client->wait_for_service(std::chrono::seconds(1))){
        if(rclcpp::ok()){
            RCLCPP_ERROR(get_logger(), "Interrupted while wating for th service");
            return;
        }
        RCLCPP_INFO(get_logger(), "Service not available, waiting again...");
    }
    auto result = client->async_send_request(request, std::bind(&SimpleServiceClient::responseCallback, this, std::placeholders::_1));
}

void SimpleServiceClient::responseCallback(rclcpp::Client<skid_steer_msgs::srv::AddTwoInts>::SharedFuture future){
    if (future.valid())
        RCLCPP_INFO_STREAM(get_logger(), "Service response " << future.get()->sum);
    else    
        RCLCPP_ERROR(get_logger(), "Service failure");
    
}

SimpleServiceClient::~SimpleServiceClient()
{
}
int main(int argc, char *argv[]){
    rclcpp::init(argc, argv);
    if(argc != 3){
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Wrong number of arguments! Usage: simple_serice_client A B");
        return 1;
    }
    auto node = std::make_shared<SimpleServiceClient>(atoi(argv[1]), atoi(argv[2]));
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}