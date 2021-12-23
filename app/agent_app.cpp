#include "agent/agent.hpp"
#include <iostream>

int main(){
  int result = agent::add_one(1);
  std::cout << "1 + 1 = " << result << std::endl;
}