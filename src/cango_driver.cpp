#include "cango_driver.hpp"
#include "common/ros2_sport_client.h"

// 1. 생성자: 클래스 이름(CangoDriver::)을 반드시 붙여야 합니다.
CangoDriver::CangoDriver() : Node("cango_driver")
{
    this->setup();
}

// 2. 모든 함수 앞에 CangoDriver:: 를 붙여서 클래스 소속임을 명시합니다.
void CangoDriver::setup()
{
    // 이제 'this'와 클래스 멤버 변수들을 정상적으로 인식합니다.
    this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    control_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
        "/master2control", 10,
        std::bind(&CangoDriver::controlCallback, this, std::placeholders::_1));

    req_puber = this->create_publisher<unitree_api::msg::Request>(
        "/api/sport/request", 10);

    // SportClient 초기화
    sport_req = std::make_unique<SportClient>();

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(10), // 100Hz
        std::bind(&CangoDriver::timerCallback, this));
}

void CangoDriver::controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr &msg)
{
    std::cout << "Received control command: linear_speed=" << msg->linear_speed << std::endl;
    target_vx = msg->linear_speed;
    target_vy = -msg->side_speed;
    target_vyaw = -msg->ang_speed;
}

void CangoDriver::timerCallback()
{
    try
    {
        // if (!start_set)
        // {
        //     RCLCPP_WARN(this->get_logger(), "start_set ...");

        //     unitree_api::msg::Request gait_req;
        //     sport_req->StandUp(gait_req);
        //     req_puber->publish(gait_req);

        start_set = true;
        // }
        if (start_set && !start_set2)
        {
            RCLCPP_WARN(this->get_logger(), "start_set2 ...");

            unitree_api::msg::Request gait_req1;
            sport_req->BalanceStand(gait_req1);
            req_puber->publish(gait_req1);
            start_set2 = true;
        }
        if (start_set2 && !mode_set)
        {
            RCLCPP_WARN(this->get_logger(), "mode_set ...");
            unitree_api::msg::Request gait_req2;
            sport_req->SwitchGait(gait_req2, 3);
            req_puber->publish(gait_req2);
            mode_set = true;
        }
        // if (mode_set && !gait_set_)
        // {
        //     RCLCPP_WARN(this->get_logger(), "Setting gait...");

        //     unitree_api::msg::Request gait_req3;
        //     sport_req->ContinuousGait(gait_req3, true);
        //     req_puber->publish(gait_req3);
        //     gait_set_ = true;
        // }
        else
        {
            unitree_api::msg::Request fresh_req;

            sport_req->Move(fresh_req, target_vx, target_vy, target_vyaw);
            req_puber->publish(fresh_req);
        }
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(this->get_logger(), "Error in timer_callback: %s", e.what());
    }
}

// 3. 메인 함수
int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CangoDriver>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}