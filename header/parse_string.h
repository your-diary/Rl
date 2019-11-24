#ifndef is_parse_string_included

    #define is_parse_string_included

    using namespace std;
    #include <iostream>
    #include <fstream>
    #include <vector>
    #include <map>
    #include "./misc.h"

    namespace parse_string {

        //Extract a numerical argument from a string (e.g. extract 12 from "verbosity 12").
        //The string is scanned from `start_index` till the next character right after a series of numbers (e.g. the scan ends in 'a' if the string is "hello123abc").
        //When it fails, `(unsigned)-1` is returned.
        unsigned extract_numerical_argument(const string &str, unsigned start_index = 0) {

            unsigned first_index = -1;
            unsigned last_index = -1;

            for (int i = start_index; i < str.size(); ++i) {

                switch (str[i]) {

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if (first_index == -1) {
                            first_index = i;
                        }
                        break;

                    default:
                        if (first_index != -1) {
                            last_index = i - 1;
                            goto end_of_for;
                        }

                }
            }

            end_of_for:

            if (first_index == -1) {
                return -1;
            } else if (last_index == -1) {
                last_index = str.size() - 1;
            }

            char numerical_part[str.size()];

            int index;
            for (index = first_index; index <= last_index; ++index) {
                numerical_part[index - first_index] = str[index];
            }
            numerical_part[(index - first_index)] = '\0';

            return static_cast<unsigned>(atoi(numerical_part));

        }

        void set_special_string_list(const string &special_string_list_file_name, map<string, string> &special_string_list) {

            ifstream ifs(special_string_list_file_name.c_str());
            if (!ifs) {
                cout << "Couldn't open a file for a list of special strings [ " << special_string_list_file_name << " ].\n";
                ifs.close();
                exit(1);
            }

            const unsigned maximum_line_length = 200;
            char line[maximum_line_length];

            while (true) {

                ifs.getline(line, maximum_line_length);
                if (!ifs) {
                    break;
                }
                if (line[0] == '\0') {
                    continue;
                }

                string line_str = line;
                string::size_type index_of_separator = line_str.find(" ");
                if (index_of_separator == string::npos) {
                    cout << "Parsing the definition of a special string [ " << line << " ] failed.\n";
                    ifs.close();
                    exit(1);
                }

                special_string_list[line_str.substr(0, index_of_separator)] = line_str.substr(index_of_separator);

            }

        }

        bool process_special_string(const string &str, map<string, string> &special_string_list) {
            if (special_string_list.count(str)) {
                system(special_string_list[str].c_str());
                cout << "\n";
                return true;
            } else {
                return false;
            }
        }

        bool parse_rl_special_command(const string &str, const string &rl_output_prefix, const vector<string> &input_history, bool &should_restart_program) {

            if (str.substr(0, 8) == "RlOutput") {

                cout << "----Input History----\n";
                cout << misc::fg_blue_bright;
                if (rl_output_prefix != "") {
                    cout << rl_output_prefix << "\n";
                }
                int start_index = parse_string::extract_numerical_argument(str, /* scan_start = */ 9);
                if (start_index == -1) {
                    start_index = 0;
                } else {
                    start_index = input_history.size() - start_index;
                }
                for (int i = start_index; i < input_history.size(); ++i) {
                    cout << input_history[i] << "\n";
                }
                cout << misc::color_end;
                cout << "----------------\n";
                cout << "\n";

            } else if (str == "RlSave") {

                cout << "Output File: ";
                string output_file_name;
                cin >> output_file_name;
                ofstream ofs(output_file_name.c_str());

                if (!ofs) {

                    cout << "Couldn't open the file [ " << output_file_name << " ].\n";
                    cout << "NOTE: A local file should be specified even when you connect to a remote host (e.g. via `ssh`).\n";
                    cout << "Command cancelled.\n";
                } else {

                    if (rl_output_prefix != "") {
                        ofs << rl_output_prefix << "\n";
                    }
                    for (int i = 0; i < input_history.size(); ++i) {
                        ofs << input_history[i] << "\n";
                    }
                    ofs.close();
                    cout << misc::fg_blue_bright << "rl: Input history has been output to [ " << output_file_name << " ].\n" << misc::color_end;

                }

                cout << "\n";

            } else if (str == "RlRestart") {

                cout << "Restarting the session...\n";
                cout << "\n";
                cout << "------------------------------------------------------\n";

                should_restart_program = true;

            } else {

                return false;

            }

            return true;

        }


    }

#endif

