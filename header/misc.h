#ifndef is_misc_included

    #define is_misc_included

    using namespace std;
    #include <iostream>
    #include <sstream>

    namespace misc {

        const char *no_color = "\e[0m";
        const char *color_end = no_color;
        const char *fg_green = "\e[032m";
        const char *fg_blue_bright = "\e[094m";

        void set_terminal_title(const string &title = "Terminal") {
            cout << "\033]0;" << title << "\a";
        }

        void check_if_program_exist(const string &program_name) {
            ostringstream oss;
            oss << "type " << program_name << " > /dev/null 2>&1";
            if (system(oss.str().c_str())) {
                cout << "Program [ " << program_name << " ] was not found.\n";
                exit(1);
            }
        }

    }

#endif

