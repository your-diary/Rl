//Requirment: -pthread -lreadline

//See `README.md` for the specifications.
//See `pty.h_structure.png` for the code structure.

//history file is under === named ~~~

//.inputrc recommended

using namespace std;

#include <iostream>
#include <sstream>
#include <fstream>

#include <map>

#include <memory>

#include <readline/readline.h>
#include <readline/history.h>

#include "./header_pty/pty.h"
#include "./header/parse_string.h"
#include "./header/readline_helper.h"

namespace misc {

    volatile sig_atomic_t has_caught_sigint = 0;
    
    void signal_handler(int signal) {
        if (signal == SIGINT) {
            debug::debug_print(__func__, "(): Caught SIGINT.");
            misc::has_caught_sigint = 1;
        }
    }

    vector<string> input_history;
    bool should_restart_program = false;

}

namespace prm {

    const string usage_message = "\
\e[;100mUsage\e[0m\n\
  rl [<option(s)>] <command> [<option(s) to command>]\n\
  rl {-h|--help}\n\
  \n\
\e[;100mOptions\e[0m\n\
  \e[94m--title <title>\e[0m                      Set the terminal title.\n\
  \e[94m--dict <file name>\e[0m                   Use the specified file as a dictionary for tab-completion.\n\
  \e[94m--special-string-file <file name>\e[0m    Define special strings from the file.\n\
  \e[94m--enable-exclamation-command\e[0m         Execute an input as a shell command if it starts with '!'.\n\
  \e[94m--rl-output-prefix <prefix>\e[0m          Set a string which is prepended to the first line of `RlOutput` command's output.\n\
  \e[94m--first-input <string>\e[0m               Set a string which is automatically input when the session starts.\n\
  \e[94m--comment-char <char>\e[0m                Set a starting character of a comment line.\n\
  \e[94m--help,-h\e[0m                            Print this help message.\n\
  \n\
\e[;100mSpecial Commands\e[0m\n\
  \e[94mRlOutput\e[0m      Output to standard output every string which was input in this session.\n\
  \e[94mRlSave\e[0m        Same as above, but output to a file (additional input of the file name is required).\n\
  \e[94mRlRestart\e[0m     Restart the session.\
";

    void print_usage() {
        cout << usage_message << "\n";
    }

    const char *history_file_name_prefix = "/.rl_history/";

    const char *readline_prompt = NULL;

}

namespace option {

    map<string, string> special_string_list;

    bool should_specially_interpret_exclamation_mark = false; //When this is true, execute the buffer as a shell command if it starts with '!'.

    //a string which is automatically input when a session starts
    //This corresponds to `--first-input` option.
    string first_input_string = "";

    //a starting character of a comment
    char comment_char = '\0';

    string rl_output_prefix = ""; //A string which is prepended to the first line of `RlOutput` command's output. This is set by `--rl-output-prefix` option.

}

int main(int argc, char **argv) {

    signal(SIGINT, misc::signal_handler);

    //set the application name (i.e. that of this program)
    rl_readline_name = "Rl";

    //parse option {

    if (argc == 1) {
        prm::print_usage();
        return 0;
    }

    bool has_option_parse_error_occured = false;

    unsigned index_of_target_program_name_in_argv = -1;

    for (int i = 1; i < argc; ++i) {

        string option = argv[i];

        if (option == "-h" || option == "--help") {
            prm::print_usage();
            return 0;
        } else if (option == "--dict") {
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                readline_helper::set_completion(argv[++i]);
            }
        } else if (option == "--title") {
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                misc::set_terminal_title(argv[++i]);
            }
        } else if (option == "--special-string-file") {
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                parse_string::set_special_string_list(argv[++i], option::special_string_list);
            }
        } else if (option == "--rl-output-prefix") { //Set a string which is prepended to the first line of `RlOutput` command's output.
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                option::rl_output_prefix = argv[++i];
            }
//         } else if (option == "--prompt") {
//             if (!(i + 1 < argc)) {
//                 has_option_parse_error_occured = true;
//             } else {
//                 prm::readline_prompt = argv[++i];
//             }
        } else if (option == "--enable-exclamation-command") {
            option::should_specially_interpret_exclamation_mark = true;
        } else if (option == "--first-input") {
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                option::first_input_string = argv[++i];
            }
        } else if (option == "--comment-char") {
            if (!(i + 1 < argc)) {
                has_option_parse_error_occured = true;
            } else {
                option::comment_char = argv[++i][0];
            }
        } else if (option[0] == '-') { //case of unknown option
            cout << "Unknown option [ " << option << " ] is specified.\n";
            return 1;
        } else {
            misc::check_if_program_exist(option);
            index_of_target_program_name_in_argv = i;
            break;
        }

        if (has_option_parse_error_occured) {
            cout << "An error occured while parsing the option [ " << option << " ].\n";
            return 1;
        }

    }

    if (index_of_target_program_name_in_argv == (unsigned)-1) {
        cout << "Target program wasn't specified.\n";
        return 1;
    }

    string pure_program_name;
    {
        string full_program_name = argv[index_of_target_program_name_in_argv];
        string::size_type found_index = full_program_name.rfind('/', full_program_name.size() - 1);
        if (found_index == string::npos) {
            pure_program_name = full_program_name;
        } else {
            pure_program_name.assign(full_program_name.begin() + found_index + 1, full_program_name.end());
        }
    }

    //} parse option

tag_program_start:

    const string history_file_name = getenv("HOME") + string(prm::history_file_name_prefix) + pure_program_name;

    //check the size of the history file {
    //In the previous version, there was a bug in which a vast quantity of duplicate lines were written to the history file and its size reached hundreds of MB.
    //Though this bug has already been removed, just in case, I check the file size.

    //get the file size
    ifstream ifs(history_file_name.c_str());
    ifs.seekg(0, ios_base::end);
    const double kilo_byte_size_of_history_file = ifs.tellg() / 1024.0;
    ifs.close();

    if (kilo_byte_size_of_history_file > 2000) { //if over 2MB (this is about 50 characters * 200,000 lines)
        cout << "The size of the history file [ " << history_file_name << " ] is [ " << (kilo_byte_size_of_history_file / 1024) << " ](MB).\n";
        cout << "Exit because it's too large.\n";
        return 0;
    } else if (kilo_byte_size_of_history_file > 500) { //if over 500KB (this is about 50 characters * 10,000 lines)
        cout << "The size of the history file [ " << history_file_name << " ] is [ " << kilo_byte_size_of_history_file << " ](KB).\n";
        cout << "It is suspected that the history file has swelled much due to a bug.\n";
        cout << "Proceeding...\n";
    }

    //} check the size of the history file

    if (read_history(history_file_name.c_str()) != 0) {
        cout << "Couldn't read the history file [ " << history_file_name << " ].\n";
        cout << "Proceeding...\n";
    }

    //setup a pty {

    const char *command_for_slave = argv[index_of_target_program_name_in_argv];

    const char * argv_for_command[1000] = { nullptr };
    for (int i = index_of_target_program_name_in_argv; i < argc; ++i) {
        argv_for_command[i - index_of_target_program_name_in_argv] = argv[i];
    }

    pty::Pty pseudo_terminal(command_for_slave, argv_for_command);

    int fd_master = pseudo_terminal.get_fd_master_();
    FILE *master = fdopen(fd_master, "w");

    //} setup a pty

    if (option::first_input_string != "") {
        fprintf(master, "%s\n", option::first_input_string.c_str());
    }

    string previous_buf;

    while (true) {

        unique_ptr<char, void(*)(void *)> buf(readline(prm::readline_prompt), free);

        if (misc::has_caught_sigint) {
            debug::debug_print(__func__, "(): Sending SIGINT to the slave...");
            char sigint_string[2] = {'', '\0'};
            fprintf(master, "%s", sigint_string);
            debug::debug_print(__func__, "(): SIGINT has been sent to the slave.");
            misc::has_caught_sigint = false;
            continue;
        }
        
        if (buf.get() == NULL) { //if reading input failed
            break;
        }

        if (buf.get()[0] == '\0') { //if just a newline is input
            cout << "\n";
            fprintf(master, "\n");
            continue;
        } else {
            if (buf.get() != previous_buf) { //Add the contents to the history only when that is different from the previous buffer.
                previous_buf = buf.get();
                add_history(buf.get());
            }
        }

        if (buf.get()[0] == option::comment_char) {
            fprintf(master, "\n");
            continue;
        }

        if (option::should_specially_interpret_exclamation_mark) {
            if (buf.get()[0] == '!') {
                system(&(buf.get()[1]));
                cout << "\n";
                continue;
            }
        }

        if (parse_string::process_special_string(buf.get(), option::special_string_list)) {
            fprintf(master, "\n");
            continue;
        }

        if (parse_string::parse_rl_special_command(buf.get(), option::rl_output_prefix, misc::input_history, misc::should_restart_program)) {
            if (misc::should_restart_program) {
                break;
            } else {
                continue;
            }
        }

        misc::input_history.push_back(buf.get()); //add to the input history (this is independent of the history list of `Readline`)

        fprintf(master, "%s\n", buf.get());

    }

    fclose(master);

    if (write_history(history_file_name.c_str()) != 0) {
        cout << "Couldn't write to the hsitory file [ " << history_file_name << " ].\n";
        return 1;
    }

    if (misc::should_restart_program) {
        misc::should_restart_program = false;
        clear_history(); //Initialize the history list before restarting the session. Without this, the length of the history list doubles every time the session restarts.
        goto tag_program_start;
    }

}

