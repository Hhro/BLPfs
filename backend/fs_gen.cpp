#include <cstring>
#include <iostream>

#include "dispatcher.hpp"
#include "fs.hpp"

int main() {
  FileSystemManager FSM;
  const char *flag = "THIS-IS-A-TEST-FLAG";

  std::ofstream output("blpfs");

  if (FSM.MountBLPfs("blpfs")) {
    FSM.CreateFile("flag.txt", 0x50, 1, 1);
    FSM.WriteFile("flag.txt", 0, 0x50, flag, 1);
    FSM.DismountBLPfs();
  };
}