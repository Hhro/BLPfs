#ifndef DISPATCHER_HPP_
#define DISPATCHER_HPP_

#include "fs.hpp"
#include <vector>
#include <string>

class Dispatcher {
    private:
        FileSystemManager* fs_mgr_;
        void ExecuteEntryVM();
        std::string ExecuteScript(char *script);
        int HandleCommands(int socket, int ulevel);
    public:
        Dispatcher(FileSystemManager* fs_mgr);
};

#endif
