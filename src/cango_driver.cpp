#include <chrono>
#include <functional>
#include <thread>

#include "cango_driver.hpp"


namespace cango_driver {


CangoDriver::CangoDriver() : Node("cango_driver") {

  this->setup();
}


void CangoDriver::setup() {

  rclcpp::SubscriptionOptions subscriber_options;
  subscriber_options.callback_group = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  control_sub = this->create_subscription<cango_msgs::msg::RobotControl>("~/input", 10, std::bind(&CangoDriver::controlCallback, this, std::placeholders::_1), subscriber_options);
  status_pub = this->create_publisher<cango_msgs::msg::RobotStatus>("~/output", 10);

  timer_ = this->create_wall_timer(std::chrono::duration<double>(1.0), std::bind(&CangoDriver::timerCallback, this));
}


void CangoDriver::controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr& msg) {

  RCLCPP_INFO(this->get_logger(), "Message received");

    // // publish message
    // cango_msgs::msg::RobotStatus out_msg;
    // out_msg = *msg;
    // status_pub->publish(out_msg);
}


void CangoDriver::timerCallback() {

  // RCLCPP_INFO(this->get_logger(), "Timer triggered");
}


}


int main(int argc, char *argv[]) {

  rclcpp::init(argc, argv);
  auto node = std::make_shared<cango_driver::CangoDriver>();
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), node->num_threads_);
  RCLCPP_INFO(node->get_logger(), "Spinning node '%s' with %s (%d threads)", node->get_fully_qualified_name(), "MultiThreadedExecutor", node->num_threads_);
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();

  return 0;
}
