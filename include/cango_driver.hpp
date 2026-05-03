#ifndef CANGO_DRIVER_HPP
#define CANGO_DRIVER_HPP

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "cango_msgs/msg/robot_control.hpp"
#include "unitree_go/msg/wireless_controller.hpp"

namespace unitree::robot::go2
{
class SportClient;
}

class CangoDriver : public rclcpp::Node
{
public:
    explicit CangoDriver(const std::string &network_interface);

private:
    void setup();

    void controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr &msg);
    void remoteCallback(const unitree_go::msg::WirelessController::ConstSharedPtr &msg);
    void timerCallback();

    void publishStandUp();
   void publishClassicWalk();
    void publishSitDown();
    void publishStopMove();
    void publishMove();

    rclcpp::Subscription<cango_msgs::msg::RobotControl>::SharedPtr control_sub;
    rclcpp::Subscription<unitree_go::msg::WirelessController>::SharedPtr remote_sub;
    rclcpp::TimerBase::SharedPtr timer_;

    std::unique_ptr<unitree::robot::go2::SportClient> sport_client;

    bool robot_up = false;
    bool prev_robot_up = false;

    bool start_set = false;
    bool gait_set = false;

    bool remote_active = false;
    rclcpp::Time last_remote_time;

    int up_count = 0;

    float target_vx = 0.0;
    float target_vy = 0.0;
    float target_vyaw = 0.0;
};

#endif