#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <slam_toolbox/srv/save_map.hpp>  
#include <libserial/SerialPort.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

using NavigateToPose = nav2_msgs::action::NavigateToPose;
using GoalHandleNavigate = rclcpp_action::ClientGoalHandle<NavigateToPose>;

class LoraGoalReceiver : public rclcpp::Node {
public:
    LoraGoalReceiver() : Node("lora_goal_receiver") {
        initialize_serial();
        initialize_clients();
        initialize_timers();
        initialize_publishers();
        initialize_subscribers();
        
        RCLCPP_INFO(this->get_logger(), "Node initialized. Waiting for LoRa goal messages...");
    }

    ~LoraGoalReceiver() {
        if (serial_port_.IsOpen()) {
            sendSerialMessage("CC");
            serial_port_.Close();
            RCLCPP_INFO(this->get_logger(), "Serial port closed.");
        }
    }

private:
    // Initialization methods
    void initialize_serial() {
        try {
            serial_port_.Open("/dev/ttyUSB1");
            serial_port_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
            serial_port_.FlushIOBuffers();
            RCLCPP_INFO(this->get_logger(), "Serial port opened successfully");
        } catch (const std::exception& e) {
            RCLCPP_FATAL(this->get_logger(), "Failed to open serial port: %s", e.what());
            rclcpp::shutdown();
        }
    }

    void initialize_clients() {
        map_client_ = this->create_client<slam_toolbox::srv::SaveMap>("/slam_toolbox/save_map");
        nav_to_pose_client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
    }

    void initialize_timers() {
        // Timer for reading LoRa data (10Hz)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&LoraGoalReceiver::readLoraData, this)
        );
        
        // Timer for continuous velocity commands (20Hz)
        cmd_vel_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),
            [this]() {
                if (active_manual_) {
                    geometry_msgs::msg::Twist cmd_vel;
                    cmd_vel.linear.x = current_linear_;
                    cmd_vel.angular.z = current_angular_;
                    cmd_vel_pub_->publish(cmd_vel);
                }
            }
        );
    }

    void initialize_publishers() {
        cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    }

    void initialize_subscribers() {
        pose_sub_ = create_subscription<geometry_msgs::msg::PoseWithCovarianceStamped>(
            "/pose", 10,  
            [this](const geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr msg) {
                current_pose_ = msg;
            });
    }

    // Main functionality
    void readLoraData() {
        if (!serial_port_.IsOpen() || !serial_port_.IsDataAvailable()) return;

        try {
            std::string raw_message;
            serial_port_.ReadLine(raw_message);
            process_raw_message(raw_message);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Serial read error: %s", e.what());
        }
    }

    void process_raw_message(const std::string& raw_message) {
        // Clean message
        std::string message = raw_message;
        message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
        message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

        RCLCPP_DEBUG(this->get_logger(), "Raw message received: '%s'", message.c_str());

        if (message == "save_map") {
            handle_save_map();
            return;
        }

        parse_message_content(message);
    }

    void handle_save_map() {
        RCLCPP_INFO(this->get_logger(), "Saving map");
        sendSerialMessage("MS");
        saveMap("/home/chocho1/maps/map_saved");
    }

    void parse_message_content(const std::string& message) {
        std::stringstream ss(message);
        std::string segment;
        float x = 0.0f, y = 0.0f, theta = 0.0f, h = -1.0f, m = -1.0f;
        bool parse_success = true;
        bool return_to_origin = false;

        while (std::getline(ss, segment, '/')) {
            if (segment.empty()) continue;

            if (!process_segment(segment, x, y, theta, h, m, parse_success, return_to_origin)) {
                return;
            }
        }

        if (parse_success) {
            handle_parsed_goal(x, y, theta, return_to_origin);
        } else {
            RCLCPP_WARN(this->get_logger(), "Partial/invalid message received: %s", message.c_str());
        }
    }

    bool process_segment(const std::string& segment, float& x, float& y, float& theta, 
                        float& h, float& m, bool& parse_success, bool& return_to_origin) {
        if (segment.length() == 1) {
            return handle_manual_command(segment[0]);
        }

        try {
            char prefix = segment[0];
            float value = std::stof(segment.substr(1));

            switch (prefix) {
                case 'x': x = value; break;
                case 'y': y = value; break;
                case 'w': theta = value; break;
                case 'h': h = value; break;
                case 'm': m = value; break;
                default:
                    RCLCPP_WARN(this->get_logger(), "Unknown prefix '%c' in segment: %s", prefix, segment.c_str());
                    parse_success = false;
                    break;
            }

            if (h >= 0 && m >= 0 && h == 0 && m <= 20) {
                RCLCPP_WARN(this->get_logger(), "Low battery! Returning to origin (h=%.0f, m=%.0f)", h, m);
                return_to_origin = true;
                x = y = theta = 0.0f;
            }
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error parsing segment '%s': %s", segment.c_str(), e.what());
            parse_success = false;
        }

        return true;
    }

    bool handle_manual_command(char command) {
        switch (command) {
            case 'q': publishVelocity(0.5, 0.0); return false;
            case 's': publishVelocity(-0.5, 0.0); return false;
            case 'a': publishVelocity(0.0, 1.0); return false;
            case 'd': publishVelocity(0.0, -1.0); return false;
            case 'x': publishVelocity(0.0, 0.0); return false;
            default:
                RCLCPP_WARN(this->get_logger(), "Unknown command: %c", command);
                return false;
        }
    }

    void handle_parsed_goal(float x, float y, float theta, bool return_to_origin) {
        RCLCPP_INFO(this->get_logger(), "Goal received - X: %.2f, Y: %.2f, θ: %.2f", x, y, theta);
        sendGoal(x, y, theta, return_to_origin);
    }

    // Navigation methods
    void sendGoal(float x, float y, float theta, bool is_origin = false) {
        if (is_origin) {
            send_origin_goal();
            return;
        }

        if (!nav_to_pose_client_->wait_for_action_server(std::chrono::seconds(3))) {
            RCLCPP_ERROR(this->get_logger(), "Action server is not available.");
            return;
        }

        auto goal_msg = create_goal_message(x, y, theta * M_PI / 180.0);
        auto send_goal_options = create_goal_options(x, y, theta);
        
        nav_to_pose_client_->async_send_goal(goal_msg, send_goal_options);
    }

    void send_origin_goal() {
        nav_to_pose_client_->async_cancel_all_goals();
        RCLCPP_INFO(this->get_logger(), "Sending robot to origin (low battery)");

        auto goal_msg = create_goal_message(0.0, 0.0, 0.0);
        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        
        send_goal_options.result_callback = [this](const GoalHandleNavigate::WrappedResult& result) {
            if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
                RCLCPP_INFO(this->get_logger(), "Origin reached!");
                sendSerialMessage("BO");
            } else {
                RCLCPP_ERROR(this->get_logger(), "Failed to reach origin.");
                sendSerialMessage("CO");
            }
        };

        nav_to_pose_client_->async_send_goal(goal_msg, send_goal_options);
    }

    NavigateToPose::Goal create_goal_message(float x, float y, float theta) {
        NavigateToPose::Goal goal_msg;
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();
        goal_msg.pose.pose.position.x = x;
        goal_msg.pose.pose.position.y = y;

        tf2::Quaternion q;
        q.setRPY(0, 0, theta);
        goal_msg.pose.pose.orientation = tf2::toMsg(q);

        RCLCPP_INFO(this->get_logger(), "Sending goal: (%.2f, %.2f, θ=%.2f rad)", x, y, theta);
        
        return goal_msg;
    }

    rclcpp_action::Client<NavigateToPose>::SendGoalOptions create_goal_options(float x, float y, float theta) {
        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        
        send_goal_options.result_callback = [this, x, y, theta](const GoalHandleNavigate::WrappedResult& result) {
            std::ostringstream oss;
            
            switch (result.code) {
                case rclcpp_action::ResultCode::SUCCEEDED:
                    RCLCPP_INFO(this->get_logger(), "Goal reached.");
                    oss << "AUX" << std::fixed << std::setprecision(2) << x << "Y" << y << "W" << (theta * 180.0 / M_PI);
                    break;

                case rclcpp_action::ResultCode::ABORTED:
                    handle_aborted_goal(oss, "AUE1");
                    break;

                case rclcpp_action::ResultCode::CANCELED:
                    handle_aborted_goal(oss, "AUE2");
                    break;

                default:
                    handle_aborted_goal(oss, "AUE3");
                    break;
            }
            
            sendSerialMessage(oss.str());
        };
        
        return send_goal_options;
    }

    void handle_aborted_goal(std::ostringstream& oss, const std::string& prefix) {
        if (current_pose_) {
            auto& pose = current_pose_->pose.pose;
            double yaw = tf2::getYaw(pose.orientation);
            oss << prefix << std::fixed << std::setprecision(2)
                << "X" << pose.position.x 
                << "Y" << pose.position.y 
                << "W" << (yaw * 180.0 / M_PI);
        } else {
            oss << prefix;
        }
    }

    // Utility methods
    void sendSerialMessage(const std::string &message) {
        if (serial_port_.IsOpen()) {
            serial_port_.Write(message + "\n");
            RCLCPP_INFO(this->get_logger(), "Message sent via serial: %s", message.c_str());
        }
    }

    void saveMap(const std::string &map_path) {
        auto request = std::make_shared<slam_toolbox::srv::SaveMap::Request>();
        request->name.data = map_path;

        if (!map_client_->wait_for_service(std::chrono::seconds(5))) {
            RCLCPP_ERROR(this->get_logger(), "Map saver service not available.");
            return;
        }

        auto result = map_client_->async_send_request(request);
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result) ==
            rclcpp::FutureReturnCode::SUCCESS) {
            RCLCPP_INFO(this->get_logger(), "Map saved successfully!");
        } else {
            RCLCPP_ERROR(this->get_logger(), "Error saving the map.");
        }
    }

    void publishVelocity(double linear, double angular) {
        current_linear_ = linear;
        current_angular_ = angular;
        active_manual_ = (linear != 0.0 || angular != 0.0);
        
        geometry_msgs::msg::Twist cmd_vel;
        cmd_vel.linear.x = current_linear_;
        cmd_vel.angular.z = current_angular_;
        cmd_vel_pub_->publish(cmd_vel);
    }

    // Member variables
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::TimerBase::SharedPtr cmd_vel_timer_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    bool active_manual_ = false;
    double current_linear_ = 0.0;
    double current_angular_ = 0.0;
    
    LibSerial::SerialPort serial_port_;
    rclcpp::Client<slam_toolbox::srv::SaveMap>::SharedPtr map_client_; 
    rclcpp_action::Client<NavigateToPose>::SharedPtr nav_to_pose_client_;
    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pose_sub_;
    geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr current_pose_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LoraGoalReceiver>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}