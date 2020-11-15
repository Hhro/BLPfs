#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "dispatcher.hpp"
#include "fs.hpp"

int main(int argc, char *argv[]) {
  FileSystemManager FSM;

  char *fname = argv[1];
  char *flag = argv[2];
  int flag_len = strlen(flag);

  std::ofstream blpfs(fname, std::ios::out | std::ios::trunc);

  if (FSM.MountBLPfs(fname)) {
    FSM.CreateFile("flag.txt", flag_len, 2, 2);
    FSM.WriteFile("flag.txt", 0, flag_len, flag, 2);
    FSM.DismountBLPfs();
  };
}
