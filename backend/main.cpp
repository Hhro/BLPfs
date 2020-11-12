#include <iostream>

#include "fs.hpp"

int main() {
  FileSystemManager FSM;
  char *buf = "ABCDEFGH";
  char *out = new char[8];
  if (FSM.MountBLPfs("blpfs")) {
    FSM.CreateFile("flag.txt", 8, 3, 1);
    FSM.WriteFile("flag.txt", 0, 8, buf, 1);
    FSM.ReadFile("flag.txt", 0, 8, 3, out);

    FSM.CreateFile("flag.txt", 8, 3, 1);
    FSM.CreateFile("aaaaaaaa", 8, 3, 1);
    FSM.CreateFile("bbbbbbbb", 8, 3, 1);
    FSM.CreateFile("cccccccc", 8, 3, 1);
    FSM.CreateFile("dddddddd", 8, 3, 1);
    FSM.WriteFile("dddddddd", 0, 8, buf, 1);
    FSM.ReadFile("dddddddd", 0, 8, 3, out);
    std::cout << out;
  };
}