#include "fs.hpp"

int main() {
  FileSystemManager FSM;
  if (FSM.MountBLPfs("hello")) {
    std::cout << "YEAH";
  };
}