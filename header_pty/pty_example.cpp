//This code is a minimal example to use `pty.h`.
//This is essentially the same as `rl.cpp` though there is no `readline` support.

//Requirement: -pthread

using namespace std;
#include <iostream>

// #define NDEBUG

#include "./pty.h"

int main() {

    const char *command_for_slave = "ssh";
    const char * argv_for_command[1000] = { nullptr };
    argv_for_command[0] = "ssh";
    argv_for_command[1] = "-t";
    argv_for_command[2] = "pi@192.168.11.6";
    argv_for_command[3] = "/usr/bin/wolfram";
    argv_for_command[4] = "-rawterm";

    pty::Pty pseudo_terminal(command_for_slave, argv_for_command);

    int fd_master = pseudo_terminal.get_fd_master_();

    FILE *master = fdopen(fd_master, "w");
    string buf;

    while (true) {

        getline(cin, buf);

        if (!cin) {
            break;
        }

        fprintf(master, "%s\n", buf.c_str());

    }

}

