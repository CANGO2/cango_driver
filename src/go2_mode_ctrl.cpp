
#include <unistd.h>

#include <cmath>

#include "cango_msgs/msg/robot_control.hpp"
#include "common/ros2_sport_client.h"
#include "rclcpp/rclcpp.hpp"
#include "unitree_api/msg/request.hpp"
#include "unitree_go/msg/sport_mode_state.hpp"

using std::placeholders::_1;
// Create a soprt_request class for soprt commond request
class soprt_request : public rclcpp::Node {
 public:
  soprt_request() : Node("req_sender") {
    // the state_suber is set to subscribe "sportmodestate" topic
    // state_suber = this->create_subscription<unitree_go::msg::SportModeState>(
    //     "sportmodestate", 10,
    //     std::bind(&soprt_request::state_callback, this, _1));
    cango_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
        "cango_control", 10,
        std::bind(&soprt_request::cango_control_callback, this, _1));

    req_puber = this->create_publisher<unitree_api::msg::Request>(
        "/api/sport/request", 10);
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(int(dt * 1000)),
        std::bind(&soprt_request::timer_callback, this));
  };

 private:
  float target_vx = 0.0, target_vy = 0.0, target_vyaw = 0.0;

  void cango_control_callback(cango_msgs::msg::RobotControl::SharedPtr msg) {
    target_vx = msg->linear_speed;
    target_vy = msg->side_speed;
    target_vyaw = msg->ang_speed;
  }
  void timer_callback() {
    req.header.identity.id = 1031;
    req.header.identity.api_id = 1031;
    req.parameter = "{\"value\":1}";
    req_puber->publish(req);

    sport_req.Move(req, target_vx, target_vy, target_vyaw);
    req_puber->publish(req);
  };
  // void state_callback(unitree_go::msg::SportModeState::SharedPtr data) {}

  //   rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr
  //   state_suber;
  rclcpp::Subscription<cango_msgs::msg::RobotControl>::SharedPtr cango_sub;

  rclcpp::TimerBase::SharedPtr timer_;  // ROS2 timer
  rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_puber;

  unitree_api::msg::Request req;  // Unitree Go2 ROS2 request message
  SportClient sport_req;

  double dt = 0.002;  // control time step
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);             // Initialize rclcpp
  rclcpp::TimerBase::SharedPtr timer_;  // Create a timer callback object to
                                        // send sport request in time intervals

  rclcpp::spin(std::make_shared<soprt_request>());  // Run ROS2 node

  rclcpp::shutdown();
  return 0;
}