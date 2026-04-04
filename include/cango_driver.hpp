#ifndef CANGO_DRIVER_NODE_HPP
#define CANGO_DRIVER_NODE_HPP


#include <cango_msgs/msg/robot_control.hpp>
#include <cango_msgs/msg/robot_status.hpp>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>
#include <unistd.h>
#include <cmath>

#include "common/ros2_sport_client.h"
#include "unitree_api/msg/request.hpp"



class SportClient;

class CangoDriver : public rclcpp::Node {
 public:
  CangoDriver();
  int num_threads_ = 1;

 private:
  void setup();
  void controlCallback(
      const cango_msgs::msg::RobotControl::ConstSharedPtr& msg);

  void timerCallback();

 private:
  // with master
  rclcpp::Subscription<cango_msgs::msg::RobotControl>::SharedPtr control_sub;
  rclcpp::Publisher<cango_msgs::msg::RobotStatus>::SharedPtr status_pub;

  // with go2
  rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_puber;
  std::unique_ptr<SportClient> sport_req;

  rclcpp::TimerBase::SharedPtr timer_;


  float target_vx = 0.0f;
  float target_vy = 0.0f;
  float target_vyaw = 0.0f;

};


#endif  // CANGO_DRIVER_NODE_HPP