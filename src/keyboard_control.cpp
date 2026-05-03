#include <rclcpp/rclcpp.hpp>
#include "cango_msgs/msg/robot_control.hpp"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>

class KeyboardControlNode : public rclcpp::Node
{
public:
  KeyboardControlNode() : Node("keyboard_control_node")
  {
    declare_parameter<double>("linear_speed", 0.3);
    declare_parameter<double>("side_speed", 0.2);
    declare_parameter<double>("ang_speed", 0.5);

    pub_ = create_publisher<cango_msgs::msg::RobotControl>("/master2control", 10);

    setTerminalRawMode();

    RCLCPP_INFO(get_logger(), "Keyboard control start");
    RCLCPP_INFO(get_logger(), "w/s: forward/backward");
    RCLCPP_INFO(get_logger(), "a/d: left/right turn");
    RCLCPP_INFO(get_logger(), "z/x: left/right side move");
    RCLCPP_INFO(get_logger(), "r: stand, e: sit");
    RCLCPP_INFO(get_logger(), "space: stop, q: quit");

    timer_ = create_wall_timer(
      std::chrono::milliseconds(50),
      std::bind(&KeyboardControlNode::timerCallback, this));
  }

  ~KeyboardControlNode()
  {
    publishStop();
    restoreTerminal();
  }

private:
  rclcpp::Publisher<cango_msgs::msg::RobotControl>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  struct termios old_tio_;
  bool terminal_set_ = false;

  double vx_ = 0.0;
  double vy_ = 0.0;
  double wz_ = 0.0;

  bool robot_up_ = true;

  void setTerminalRawMode()
  {
    if (tcgetattr(STDIN_FILENO, &old_tio_) == 0)
    {
      struct termios new_tio = old_tio_;
      new_tio.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

      terminal_set_ = true;
    }
  }

  void restoreTerminal()
  {
    if (terminal_set_)
    {
      tcsetattr(STDIN_FILENO, TCSANOW, &old_tio_);
    }
  }

  void publishCommand()
  {
    cango_msgs::msg::RobotControl msg;

    msg.mode = 1.0;
    msg.linear_speed = vx_;
    msg.side_speed = vy_;
    msg.ang_speed = wz_;
    msg.robot_angle = 0.0;
    msg.robot_up = robot_up_;
    msg.vibration = false;

    pub_->publish(msg);
  }

  void publishStop()
  {
    vx_ = 0.0;
    vy_ = 0.0;
    wz_ = 0.0;
    publishCommand();
  }

  void timerCallback()
  {
    char key;

    if (read(STDIN_FILENO, &key, 1) > 0)
    {
      double linear = get_parameter("linear_speed").as_double();
      double side = get_parameter("side_speed").as_double();
      double angular = get_parameter("ang_speed").as_double();

      if (key == 'w')
      {
        vx_ = linear;
        vy_ = 0.0;
        wz_ = 0.0;
      }
      else if (key == 's')
      {
        vx_ = -linear;
        vy_ = 0.0;
        wz_ = 0.0;
      }
      else if (key == 'a')
      {
        vx_ = 0.0;
        vy_ = 0.0;
        wz_ = angular;
      }
      else if (key == 'd')
      {
        vx_ = 0.0;
        vy_ = 0.0;
        wz_ = -angular;
      }
      else if (key == 'z')
      {
        vx_ = 0.0;
        vy_ = side;
        wz_ = 0.0;
      }
      else if (key == 'x')
      {
        vx_ = 0.0;
        vy_ = -side;
        wz_ = 0.0;
      }
      else if (key == ' ')
      {
        publishStop();
        RCLCPP_INFO(get_logger(), "STOP");
        return;
      }
      else if (key == 'r')
      {
        robot_up_ = true;
        vx_ = 0.0;
        vy_ = 0.0;
        wz_ = 0.0;
        RCLCPP_INFO(get_logger(), "ROBOT STAND");
      }
      else if (key == 'e')
      {
        robot_up_ = false;
        vx_ = 0.0;
        vy_ = 0.0;
        wz_ = 0.0;
        RCLCPP_INFO(get_logger(), "ROBOT SIT");
      }
      else if (key == 'q')
      {
        publishStop();
        RCLCPP_INFO(get_logger(), "QUIT");
        rclcpp::shutdown();
        return;
      }

      RCLCPP_INFO(
        get_logger(),
        "linear: %.2f, side: %.2f, angular: %.2f, robot_up: %d",
        vx_, vy_, wz_, robot_up_);
    }

    publishCommand();
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<KeyboardControlNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}