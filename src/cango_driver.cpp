#include "cango_driver.hpp"
#include "common/ros2_sport_client.h"

<<<<<<< HEAD
CangoDriver::CangoDriver() : Node("cango_driver") { this->setup(); }

void CangoDriver::setup() {
  this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  control_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
      "/master2control", 10,
      std::bind(&CangoDriver::controlCallback, this, std::placeholders::_1));

  /////////////Publish request to go2////////////////////
  req_puber = this->create_publisher<unitree_api::msg::Request>(
      "/api/sport/request", 10);

  sport_req = std::make_unique<SportClient>();

  /////////////////Read state from go2////////////////////
  auto lowstate_cb = "lf/lowstate";
  if (HIGH_FREQ) {
    lowstate_cb = "lowstate";
  }
  state_suber = this->create_subscription<unitree_go::msg::LowState>(
      lowstate_cb, 10,
      std::bind(&CangoDriver::state_cb, this, std::placeholders::_1));
  //////// ///Timer for sending request to go2//////////////////////
  timer_ =
      this->create_wall_timer(std::chrono::milliseconds(20),  // 50Hz
                              std::bind(&CangoDriver::timerCallback, this));
=======
// 1. 생성자: 클래스 이름(CangoDriver::)을 반드시 붙여야 합니다.
CangoDriver::CangoDriver() : Node("cango_driver")
{
    this->setup();
>>>>>>> 635b8e7 (1)
}

// 2. 모든 함수 앞에 CangoDriver:: 를 붙여서 클래스 소속임을 명시합니다.
void CangoDriver::setup()
{
    // 이제 'this'와 클래스 멤버 변수들을 정상적으로 인식합니다.
    this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    control_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
        "~/cango_control", 10,
        std::bind(&CangoDriver::controlCallback, this, std::placeholders::_1));

    req_puber = this->create_publisher<unitree_api::msg::Request>(
        "/api/sport/request", 10);

    // SportClient 초기화
    sport_req = std::make_unique<SportClient>();

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100), // 50Hz
        std::bind(&CangoDriver::timerCallback, this));
}

<<<<<<< HEAD
void CangoDriver::controlCallback(
    const cango_msgs::msg::RobotControl::ConstSharedPtr &msg) {
  if(msg->robot_up == true){
    robot_up();
    return;
  }
  target_vx = msg->linear_speed;
  target_vy = msg->side_speed;
  target_vyaw = msg->ang_speed;
}

void CangoDriver::robot_up() {
  unitree_api::msg::Request gait_req;
  sport_req->StandUp(gait_req);
  req_puber->publish(gait_req);
}

void CangoDriver::timerCallback() {
  if (!sport_req) return;

  try {
    if (!gait_set) {
      // unitree_api::msg::Request gait_req;
      unitree_api::msg::Request gait_req1;
      unitree_api::msg::Request gait_req2;
      unitree_api::msg::Request gait_req3;

      // sport_req->StandUp(gait_req);
      // req_puber->publish(gait_req);
=======
void CangoDriver::controlCallback(const cango_msgs::msg::RobotControl::ConstSharedPtr &msg)
{
    std::cout << "Received control command: linear_speed=" << msg->linear_speed << std::endl;
    target_vx = msg->linear_speed;
    target_vy = -msg->side_speed;
    target_vyaw = msg->ang_speed;
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
>>>>>>> 635b8e7 (1)

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