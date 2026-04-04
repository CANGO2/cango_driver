#pragma once

#include <memory>
#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <cango_msgs/msg/robot_status.hpp>
#include <cango_msgs/msg/robot_control.hpp>

namespace cango_driver {



/**
 * @brief CangoDriver class
 */
class CangoDriver : public rclcpp::Node {

 public:

  CangoDriver();
  int num_threads_ = 1;

 private:

  /**
   * @brief Sets up subscribers, publishers, etc. to configure the node
   */
  void setup();
  void controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr& msg);

  void timerCallback();

 private:

  rclcpp::Subscription<cango_msgs::msg::RobotControl>::SharedPtr control_sub;
  rclcpp::Publisher<cango_msgs::msg::RobotStatus>::SharedPtr status_pub;

  rclcpp::TimerBase::SharedPtr timer_;
};


}
