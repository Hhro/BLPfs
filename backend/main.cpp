#include <cstring>
#include <iostream>

#include "dispatcher.hpp"
#include "fs.hpp"

int main() {
  setvbuf(stdin, 0LL, 2, 0LL);
  setvbuf(stdout, 0LL, 2, 0LL);
  setvbuf(stderr, 0LL, 2, 0LL);

  FileSystemManager FSM;
  char *flagtxt = "flag.txt";
  char *buf = new char[32];
  char *res = new char[32];

  if (FSM.MountBLPfs("blpfs")) {
    for (int i = 0; i < 4; i++) {
      FSM.CreateFile("aaaa.txt", 0x100, 1, 1);
      FSM.CreateFile("bbbbbbbb", 0x50, 1, 1);
      FSM.CreateFile("cccccccc", 0x50, 1, 1);
      FSM.CreateFile("dddddddd", 0x50, 1, 1);
    }
    FSM.CreateFile("bbbbbbbb", 0x50, 1, 1);

    FSM.WriteFile("aaaa.txt", 0x100, 0x8, flagtxt, 1);

    FSM.ReadFile("flag.txt", 0, 0x20, 3, buf);

    FSM.CreateFile("flagflag", 0x20, 3, 3);
    FSM.WriteFile("flagflag", 0, 0x20, buf, 3);

    FSM.RemoveFile("flag.txt", 1);
    for (int i = 0; i < 4; i++) {
      FSM.CreateFile("flag.txt", 0x50, 1, 1);
    }
    FSM.SizeFile("flag.txt", 1);
    FSM.RemoveFile("bbbbbbbb", 1);

    FSM.ReadFile("flagflag", 0, 0x20, 3, buf);
    FSM.CreateFile("flag2222", 0x20, 3, 3);
    FSM.CreateFile("leaked", 0x20, 3, 3);
    FSM.WriteFile("leaked", 0, 0x20, buf, 3);

    FSM.ReadFile("flag.txt", 0, 0x20, 1, res);

    std::cout << res;

    FSM.DismountBLPfs();
    // Dispatcher Dispatcher(&FSM);
  };
}