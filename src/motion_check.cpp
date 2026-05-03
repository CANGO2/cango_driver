#include <iostream>
#include <unistd.h>

#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>

int main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " networkInterface" << std::endl;
    return -1;
  }

  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

  unitree::robot::go2::SportClient sport_client;
  sport_client.SetTimeout(10.0f);
  sport_client.Init();

  std::cout << "StandUp" << std::endl;
  std::cout << "ret: " << sport_client.StandUp() << std::endl;
  sleep(2);

  // 👉 안정적인 gait 설정
  std::cout << "EconomicGait (stable walking)" << std::endl;
  std::cout << "ret: " << sport_client.EconomicGait() << std::endl;
  sleep(1);

  std::cout << "Move forward" << std::endl;
  std::cout << "ret: " << sport_client.Move(0.25, 0.0, 0.0) << std::endl;
  sleep(3);

  std::cout << "StopMove" << std::endl;
  std::cout << "ret: " << sport_client.StopMove() << std::endl;

  return 0;
}