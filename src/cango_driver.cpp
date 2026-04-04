#include "cango_driver.hpp"

#include "common/ros2_sport_client.h"

CangoDriver::CangoDriver() : Node("cango_driver") { this->setup(); }

void CangoDriver::setup() {
  this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  control_sub = this->create_subscription<cango_msgs::msg::RobotControl>(
      "/cango_control", 10,
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
}

void CangoDriver::state_cb(unitree_go::msg::LowState::SharedPtr data) {
  if (INFO_IMU) {
    // Info IMU states
    // RPY euler angle(ZYX order respected to body frame)
    // Quaternion
    // Gyroscope (raw data)
    // Accelerometer (raw data)
    imu = data->imu_state;

    RCLCPP_INFO(this->get_logger(),
                "Euler angle -- roll: %f; pitch: %f; yaw: %f", imu.rpy[0],
                imu.rpy[1], imu.rpy[2]);
    RCLCPP_INFO(this->get_logger(),
                "Quaternion -- qw: %f; qx: %f; qy: %f; qz: %f",
                imu.quaternion[0], imu.quaternion[1], imu.quaternion[2],
                imu.quaternion[3]);
    RCLCPP_INFO(this->get_logger(), "Gyroscope -- wx: %f; wy: %f; wz: %f",
                imu.gyroscope[0], imu.gyroscope[1], imu.gyroscope[2]);
    RCLCPP_INFO(this->get_logger(), "Accelerometer -- ax: %f; ay: %f; az: %f",
                imu.accelerometer[0], imu.accelerometer[1],
                imu.accelerometer[2]);
  }

  if (INFO_MOTOR) {
    // Info motor states
    // q: angluar (rad)
    // dq: angluar velocity (rad/s)
    // ddq: angluar acceleration (rad/(s^2))
    // tau_est: Estimated external torque

    for (int i = 0; i < 12; i++) {
      motor[i] = data->motor_state[i];
      RCLCPP_INFO(this->get_logger(),
                  "Motor state -- num: %d; q: %f; dq: %f; ddq: %f; tau: %f", i,
                  motor[i].q, motor[i].dq, motor[i].ddq, motor[i].tau_est);
    }
  }

  if (INFO_FOOT_FORCE) {
    // Info foot force value (int not true value)
    for (int i = 0; i < 4; i++) {
      foot_force[i] = data->foot_force[i];
      foot_force_est[i] = data->foot_force_est[i];
    }

    RCLCPP_INFO(this->get_logger(),
                "Foot force -- foot0: %d; foot1: %d; foot2: %d; foot3: %d",
                foot_force[0], foot_force[1], foot_force[2], foot_force[3]);
    RCLCPP_INFO(
        this->get_logger(),
        "Estimated foot force -- foot0: %d; foot1: %d; foot2: %d; foot3: %d",
        foot_force_est[0], foot_force_est[1], foot_force_est[2],
        foot_force_est[3]);
  }

  if (INFO_BATTERY) {
    // Info battery states
    // battery current
    // battery voltage
    battery_current = data->power_a;
    battery_voltage = data->power_v;

    RCLCPP_INFO(this->get_logger(), "Battery state -- current: %f; voltage: %f",
                battery_current, battery_voltage);
  }
}

void CangoDriver::controlCallback(
    const cango_msgs::msg::RobotControl::ConstSharedPtr &msg) {
  target_vx = msg->linear_speed;
  target_vy = msg->side_speed;
  target_vyaw = msg->ang_speed;
}

void CangoDriver::timerCallback() {
  if (!sport_req) return;

  try {
    if (!gait_set) {
      unitree_api::msg::Request gait_req;
      unitree_api::msg::Request gait_req1;
      unitree_api::msg::Request gait_req2;
      unitree_api::msg::Request gait_req3;

      sport_req->StandUp(gait_req);
      req_puber->publish(gait_req);

      sport_req->BalanceStand(gait_req1);
      req_puber->publish(gait_req1);

      sport_req->SwitchGait(gait_req2, 3);
      req_puber->publish(gait_req2);

      sport_req->ContinuousGait(gait_req3, true);
      req_puber->publish(gait_req3);
      gait_set = true;
    }
    unitree_api::msg::Request fresh_req;
    sport_req->Move(fresh_req, target_vx, target_vy, target_vyaw);
    req_puber->publish(fresh_req);
  } catch (const std::exception &e) {
    RCLCPP_ERROR(this->get_logger(), "Error in timer_callback: %s", e.what());
  }
}

// 3. 메인 함수
int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<CangoDriver>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}