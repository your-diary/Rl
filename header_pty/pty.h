//This header implements `Pty` class which handles a pty (pseudo-terminal).
//See `pty.h_structure.png` for how the codes are structured.

//Requirement: -pthread

//NOTE: It should make more sense to wrap `cout` with `mutex`. But I don't do so now to archive a faster operation.

#ifndef is_pty_included

    #define is_pty_included

    using namespace std;
    #include <iostream>
    #include <unistd.h>
    #include <thread>
    #include <termios.h>
    #include <csignal>
    #include <fcntl.h>
    #include "../header/debug.h"

    namespace pty {

        class Pty {

            private: //data member

                int fd_master_;

                thread read_loop_thread_;

                pid_t pid_slave_;

            private: //member function

//                 static void signal_handler_(int signal) {
//                     if (signal == SIGINT) {
//                         debug::debug_print(__func__, "(): SIGINT is caught.");
//                     }
//                 }

                void read_from_master_loop_(int fd_master) {

//                     signal(SIGINT, signal_handler_);

                    debug::debug_print(__func__, "(): Started.");

                    FILE *master = fdopen(fd_master, "r");
                    int c;

                    while (true) {

                        c = fgetc(master); //read not as a line but as a single character to immediately get any prompts like "In[1]:= "

                        if (feof(master) || ferror(master)) {
                            break;
                        }

                        cout << static_cast<char>(c);

                    }

                }

            public:

                Pty(const char *command_for_slave, const char * const argv[]) {

                    fd_master_ = posix_openpt(O_RDWR);
                    grantpt(fd_master_);
                    unlockpt(fd_master_);

                    //set streams unbuffered (to immediately get any prompts like "In[1] := ")
                    cout << unitbuf;
                    cerr << unitbuf;

                    pid_slave_ = fork();

                    if (pid_slave_ == 0) { //slave

                        //Create a new session and make myself the leader of the session.
                        //This is needed to allow Ctrl+c to send SIGINT only to the master (i.e. the parent process).
                        //By default, signals are sent to both a parent and the child (see |https://unix.stackexchange.com/questions/176235/fork-and-how-signals-are-delivered-to-processes|).
                        debug::debug_print(__func__, "(): Creating a new session, using `setsid()`.");
                        setsid();

                        //connect standard input/output to slave
                        freopen(ptsname(fd_master_), "r", stdin);
                        freopen(ptsname(fd_master_), "w", stdout);
                        freopen(ptsname(fd_master_), "w", stderr);
                        
                        //By default, when you input string to the master, its contents are displayed twice (see |https://stackoverflow.com/questions/59007528/avoid-output-duplication-when-using-pty|).
                        //For one time because you've really typed it. (Imagine a terminal emulator. You type a command line while seeing what you've typed.)
                        //And for one more time because a pty simulates input. (`man 7 pty` says "Anything that is written on the master end is provided to the process on the slave end **as though it was input typed on a terminal**.")
                        //The second case can be disabled by unsetting the terminal `ECHO` flag.
                        //Note, however, some commands (e.g. `/usr/bin/wolfram`) still echo twice.
                        //This may be because they internally re-enable echoing.
                        termios terminal_attribute;
                        int fd_slave = fileno(fopen(ptsname(fd_master_), "r"));
                        tcgetattr(fd_slave, &terminal_attribute);
                        terminal_attribute.c_lflag &= ~ECHO;
                        tcsetattr(fd_slave, TCSANOW, &terminal_attribute);

                        int exit_status = execvp(command_for_slave, const_cast<char ** const>(argv));

                        if (exit_status == -1) { //when the command couldn't be started (If the control reaches here, it means `execvp()` has been failed.)
                            cout << "Failed to execute the command [ " << command_for_slave << " ].\n";
                        }

                        exit(exit_status); //This is needed to terminate the child process also when it fails.

                    } else { //master

                        debug::debug_print(__func__, "(): Forked. The pid of the slave is [ ", pid_slave_, " ].");

                        read_loop_thread_ = thread(&Pty::read_from_master_loop_, this, fd_master_);

                    }

                }

                ~Pty() {

                    //terminate the read loop
                    debug::debug_print(__func__, "(): Canceling the read loop thread...");
//                     pthread_kill(read_loop_thread_.native_handle(), SIGINT);
                    pthread_cancel(read_loop_thread_.native_handle());
                    if (read_loop_thread_.joinable()) {
                        read_loop_thread_.join();
                    }

                    //terminate the slave
                    debug::debug_print(__func__, "(): Killing the slave process...");
                    kill(pid_slave_, SIGINT);

                    debug::debug_print(__func__, "(): Done.");

                }

                int get_fd_master_() { //non-const to satisly the logical constness
                    return fd_master_;
                }

        };

    }

#endif

