#include "dispatcher.hpp"

#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <iostream>

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
#define OP_CREATEFILE 5

Dispatcher::Dispatcher(FileSystemManager *fs_mgr) {
  this->fs_mgr_ = fs_mgr;
  this->ExecuteEntryVM();
}

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

std::string* Dispatcher::ExecuteScript(char *script, int ulevel) {
  int sockets[2];
  int stdout_pipe[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if (pipe(stdout_pipe) != 0) {
    perror("opening pipe");
    exit(1);
  }

  int flags = fcntl(stdout_pipe[1], F_GETFD);
  flags |= O_NONBLOCK;
  if (fcntl(stdout_pipe[1], F_SETFD, flags)) {
    perror("fcntl");
    exit(1);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork error");
    exit(1);
  } else if (pid == 0) {
    close(sockets[0]);
    close(stdout_pipe[0]);
    chdir(FRONTEND_DIR);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(sockets[1], DEFAULT_FD);
    execl(FRONTEND_ENTRYPOINT, FRONTEND_ENTRYPOINT, script, NULL);
    exit(1);
  } else {
    close(sockets[1]);
    close(stdout_pipe[1]);
    sockets[1] = NULL;

    while (1) {
      if (this->HandleCommands(sockets[0], ulevel) != 0) break;
    }
    char *buf = (char *) malloc(256);
    int len = read(stdout_pipe[0], buf, 256);
    if (len <= 0)
      len = 0;
    std::string *output = new std::string(buf, len);
    return output;
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
      if ((ret = this->fs_mgr_->ReadFile(filename, pos, len, ulevel, buf)) > 0) {
        len = ret;
      }
      else {
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
      this->fs_mgr_->RemoveFile(filename_buf, ulevel);
      break;
    }
    case OP_SIZE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      int size = this->fs_mgr_->SizeFile(filename_buf, ulevel);
      if (size < 0) size = 0;
      if (write(socket, &size, sizeof(size)) != sizeof(size)) return -1;
      break;
    }
    case OP_EXECUTE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      std::string *output = NULL;
      int flevel = this->fs_mgr_->GetFileLevel(filename);
      if (flevel >= 0) {
        int size = this->fs_mgr_->SizeFile(filename, flevel);
        if (size > 0) {
          char *buf = (char *) malloc(size);
          this->fs_mgr_->ReadFile(filename, 0, size, flevel, buf);
          output = this->ExecuteScript(buf, flevel);
        }
      }
      if (ulevel >= flevel && output != NULL) {
        uint32_t size = output->length();
        if (write(socket, &size, sizeof(size)) != sizeof(size)) return -1;
        if (size > 0) {
          write(socket, output->c_str(), size);
        }
        delete output;
      }
      else {
        uint32_t size = 0;
        if (write(socket, &size, sizeof(size)) != sizeof(size)) return -1;
      }
      break;
    }
    case OP_CREATEFILE: {
      if (read(socket, filename_buf, sizeof(filename_buf) - 1) !=
          sizeof(filename_buf) - 1)
        return -1;
      std::string filename(filename_buf);
      uint32_t len, flevel;
      if (read(socket, &len, sizeof(len)) != sizeof(len)) return -1;
      if (read(socket, &flevel, sizeof(flevel)) != sizeof(flevel)) return -1;
      this->fs_mgr_->CreateFile(filename, len, flevel, ulevel);
      break;
    }
  }
  return 0;
}
