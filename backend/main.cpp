#include <iostream>

#include "fs.hpp"

int main() {
  FileSystemManager FSM;
  if (FSM.MountBLPfs("blpfs")) {
    FSM.CreateFile("flag.txt", 3);
    FSM.CreateFile("flag.txt", 3);
  };
}