#include "dispatcher.hpp"

#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#ifndef FRONTEND_DIR
#define FRONTEND_DIR "./frontend/"
#endif

#define FRONTEND_ENTRYPOINT "./main.py"

#define DEFAULT_FD 3

#define OP_READ 0
#define OP_WRITE 1
#define OP_REMOVE 2
#define OP_SIZE 3
#define OP_EXECUTE 4

Dispatcher::Dispatcher(FileSystemManager *fs_mgr) {
  this->fs_mgr_ = fs_mgr;
  this->ExecuteEntryVM();
}

/**
int Dispatcher::ExecuteVM(std::string &filename) {
    int conn[2];
    int output[2];
    pid_t pid = fork();

}
**/

void Dispatcher::ExecuteEntryVM() {
  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork error");
    exit(1);
  } else if (pid == 0) {
    printf("%d %d\n", sockets[0], sockets[1]);
    close(sockets[0]);
    chdir(FRONTEND_DIR);
    dup2(sockets[1], DEFAULT_FD);
    execl(FRONTEND_ENTRYPOINT, FRONTEND_ENTRYPOINT, (char *)NULL);
    exit(1);
  } else {
    close(sockets[1]);
    sockets[1] = NULL;

    while (1) {
      if (this->HandleCommands(sockets[0], 1) != 0) break;
    }
  }
}

std::string Dispatcher::ExecuteScript(char *script) {
  int sockets[2];
  int stdout_pipe[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork error");
    exit(1);
  } else if (pid == 0) {
    printf("%d %d\n", sockets[0], sockets[1]);
    close(sockets[0]);
    chdir(FRONTEND_DIR);
    dup2(sockets[1], DEFAULT_FD);
    execl(FRONTEND_ENTRYPOINT, FRONTEND_ENTRYPOINT, (char *)NULL);
    exit(1);
  } else {
    close(sockets[1]);
    sockets[1] = NULL;

    while (1) {
      if (this->HandleCommands(sockets[0], 1) != 0) break;
    }
  }
}

int Dispatcher::HandleCommands(int socket, int ulevel) {
  uint8_t opcode;
  char filename_buf[9] = {
      0,
  };

  if (read(socket, &opcode, sizeof(opcode)) != sizeof(opcode)) return -1;

  switch (opcode) {
    case OP_READ: {  // suppress compiler complains
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      uint32_t pos, len;
      if (read(socket, &pos, sizeof(pos)) != sizeof(pos)) return -1;
      if (read(socket, &len, sizeof(len)) != sizeof(len)) return -1;
      char *buf = (char *)malloc(len);
      int ret;
      if ((ret = this->fs_mgr_->ReadFile(filename, pos, len, ulevel, buf)) !=
          -1) {
        // TODO: Change check when https://github.com/Hhro/BLPfs/issues/3 is
        // fixed.
        len = 0;
      }
      if (write(socket, &len, sizeof(len)) != sizeof(len)) return -1;
      if (len != 0) {
        if (write(socket, buf, len) != len) return -1;
      }
      free(buf);
      break;
    }
    case OP_WRITE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      uint32_t pos, len;
      if (read(socket, &pos, sizeof(pos)) != sizeof(pos)) return -1;
      if (read(socket, &len, sizeof(len)) != sizeof(len)) return -1;
      char *buf = (char *)malloc(len);
      if (read(socket, buf, len) != len) return -1;
      this->fs_mgr_->WriteFile(filename, pos, len, (const char *)buf, ulevel);
      free(buf);
      break;
    }
    case OP_REMOVE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      // this->fs_mgr_->RemoveFile(filename_buf);
      break;
    }
    case OP_SIZE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      // int size = this->fs_mgr_->SizeFile(filename_buf);
      int size = 0;
      if (size < 0) size = 0;
      if (write(socket, &size, sizeof(size)) != sizeof(size)) return -1;
      break;
    }
    case OP_EXECUTE: {
      // TODO: implement execute logic
    }
  }
  return 0;
}
