#include <iostream>

#include "fs.hpp"
#include "dispatcher.hpp"

int main() {
  setvbuf(stdin, 0LL, 2, 0LL);
  setvbuf(stdout, 0LL, 2, 0LL);
  setvbuf(stderr, 0LL, 2, 0LL);
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

    Dispatcher Dispatcher(&FSM);
  };
}