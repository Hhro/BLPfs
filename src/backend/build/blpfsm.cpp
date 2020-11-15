#include <cstring>
#include <iostream>

#include "dispatcher.hpp"
#include "fs.hpp"

int main(int argc, char *argv[]) {
  setvbuf(stdin, 0LL, 2, 0LL);
  setvbuf(stdout, 0LL, 2, 0LL);
  setvbuf(stderr, 0LL, 2, 0LL);

  FileSystemManager FSM;

  if (FSM.MountBLPfs(argv[1])) {
    Dispatcher Dispatcher(&FSM);
  };
}