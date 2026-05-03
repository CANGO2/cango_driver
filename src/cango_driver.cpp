#include "cango_driver.hpp"

#include <cmath>
#include <unistd.h>

#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>

CangoDriver::CangoDriver(const std::string &network_interface)
: Node("cango_driver")
{
    unitree::robot::ChannelFactory::Instance()->Init(0, network_interface);

    sport_client = std::make_unique<unitree::robot::go2::SportClient>();
    sport_client->SetTimeout(10.0f);
    sport_client->Init();

    this->setup();
}

void CangoDriver::setup()
{
    control_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
        "/master2control", 10,
        std::bind(&CangoDriver::controlCallback, this, std::placeholders::_1));

    remote_sub = this->create_subscription<unitree_go::msg::WirelessController>(
        "/wirelesscontroller", 10,
        std::bind(&CangoDriver::remoteCallback, this, std::placeholders::_1));

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(10),
        std::bind(&CangoDriver::timerCallback, this));
}

void CangoDriver::controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr &msg)
{
    robot_up = msg->robot_up;

    target_vx = msg->linear_speed;
    target_vy = -msg->side_speed;
    target_vyaw = -msg->ang_speed;
}

void CangoDriver::remoteCallback(const unitree_go::msg::WirelessController::ConstSharedPtr &msg)
{
    bool stick_moved =
        std::abs(msg->lx) > 0.08 ||
        std::abs(msg->ly) > 0.08 ||
        std::abs(msg->rx) > 0.08 ||
        std::abs(msg->ry) > 0.08;

    bool button_pressed = msg->keys != 0;

    if (stick_moved || button_pressed)
    {
        remote_active = true;
        last_remote_time = this->now();

        RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            500,
            "Remote controller active. ROS2 Move command blocked.");
    }
}

void CangoDriver::publishStandUp()
{
    RCLCPP_WARN(this->get_logger(), "StandUp");
    int ret = sport_client->StandUp();
    RCLCPP_WARN(this->get_logger(), "StandUp ret: %d", ret);
}

void CangoDriver::publishClassicWalk()
{
    RCLCPP_WARN(this->get_logger(), "ClassicWalk ON");

    int ret = sport_client->ClassicWalk(true);
    RCLCPP_WARN(this->get_logger(), "ClassicWalk ret: %d", ret);
}

void CangoDriver::publishSitDown()
{
    RCLCPP_WARN(this->get_logger(), "StandDown");
    int ret = sport_client->StandDown();
    RCLCPP_WARN(this->get_logger(), "StandDown ret: %d", ret);
}

void CangoDriver::publishStopMove()
{
    RCLCPP_WARN(this->get_logger(), "StopMove");
    int ret = sport_client->StopMove();
    RCLCPP_WARN(this->get_logger(), "StopMove ret: %d", ret);
}

void CangoDriver::publishMove()
{
    int ret = sport_client->Move(target_vx, target_vy, target_vyaw);

    if (ret != 0)
    {
        RCLCPP_WARN_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            1000,
            "Move ret: %d", ret);
    }
}

void CangoDriver::timerCallback()
{
    try
    {
        if (remote_active)
        {
            double dt = (this->now() - last_remote_time).seconds();

            if (dt > 0.5)
            {
                remote_active = false;
                RCLCPP_WARN(this->get_logger(), "Remote inactive. ROS2 Move enabled.");
            }
        }

        if (robot_up && !prev_robot_up)
        {
            RCLCPP_WARN(this->get_logger(), "robot_up changed 0 -> 1");

            start_set = false;
            gait_set = false;
            up_count = 0;
        }

        if (!robot_up && prev_robot_up)
        {
            RCLCPP_WARN(this->get_logger(), "robot_up changed 1 -> 0");

            publishStopMove();
            publishSitDown();

            start_set = false;
            gait_set = false;
            up_count = 0;
        }

        prev_robot_up = robot_up;

        if (!robot_up)
        {
            return;
        }

        if (!start_set)
        {
            publishStandUp();
            start_set = true;
            up_count = 0;
            return;
        }

        up_count++;

        // 10ms timer 기준: 200 = 약 2초
        if (start_set && !gait_set && up_count >= 200)
        {
            publishClassicWalk();
            gait_set = true;
            return;
        }

        if (gait_set)
        {
            if (remote_active)
            {
                publishStopMove();
                return;
            }

            publishMove();
        }
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(this->get_logger(), "Error in timerCallback: %s", e.what());
    }
}

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    if (argc < 2)
    {
        std::cout << "Usage: ros2 run cango_driver cango_driver networkInterface" << std::endl;
        std::cout << "Example: ros2 run cango_driver cango_driver enp2s0" << std::endl;
        return -1;
    }

    auto node = std::make_shared<CangoDriver>(argv[1]);

    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}