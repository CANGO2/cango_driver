#ifndef CANGO_DRIVER_NODE_HPP
#define CANGO_DRIVER_NODE_HPP

#include <unistd.h>

#include <cango_msgs/msg/robot_control.hpp>
#include <cango_msgs/msg/robot_status.hpp>
#include <cmath>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

#include "common/ros2_sport_client.h"
#include "unitree_api/msg/request.hpp"

#include "unitree_go/msg/low_state.hpp"
#include "unitree_go/msg/imu_state.hpp"
#include "unitree_go/msg/motor_state.hpp"

#define INFO_IMU 1        // Set 1 to info IMU states
#define INFO_MOTOR 1      // Set 1 to info motor states
#define INFO_FOOT_FORCE 1 // Set 1 to info foot force states
#define INFO_BATTERY 1    // Set 1 to info battery states

#define HIGH_FREQ 0 // Set 1 to subscribe to low states with high frequencies (500Hz)

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
  void robot_up_motion();
  bool robot_up = false;
 private:
  // with master
  rclcpp::Subscription<cango_msgs::msg::RobotControl>::SharedPtr control_sub;
  rclcpp::Publisher<cango_msgs::msg::RobotStatus>::SharedPtr status_pub;

  // with go2
  rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_puber;
  std::unique_ptr<SportClient> sport_req;
  
  void state_cb(unitree_go::msg::LowState::SharedPtr data);
  rclcpp::TimerBase::SharedPtr timer_;

  bool start_set = false;
  bool start_set2 = false;
  bool mode_set = false;
  bool gait_set_ = false;
  float target_vx = 0.0f;
  float target_vy = 0.0f;
  float target_vyaw = 0.0f;

  rclcpp::Subscription<unitree_go::msg::LowState>::SharedPtr state_suber;

  unitree_go::msg::IMUState imu;         // Unitree go2 IMU message
  unitree_go::msg::MotorState motor[12]; // Unitree go2 motor state message
  int16_t foot_force[4];                 // External contact force value (int)
  int16_t foot_force_est[4];             // Estimated  external contact force value (int)
  float battery_voltage;                 // Battery voltage
  float battery_current;                 // Battery current

};

#endif  // CANGO_DRIVER_NODE_HPP