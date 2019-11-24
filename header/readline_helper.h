//Requirement: -lreadline

#ifndef is_readline_helper_included

    #define is_readline_helper_included

    using namespace std;
    #include <iostream>
    #include <fstream>
    #include <cstring>
    #include <readline/readline.h>

    namespace readline_helper {

        //tab-completion {

        char *dictionary[500];
        unsigned len_dictionary;
        unsigned len_word_maximum = 0;

        //Generating function for the completion.
        char * completion_generator(const char *target_word, int) {

            static unsigned index = 0; //If this is used, the second argument isn't needed (so it's un-named).

            //not append a space after the completion
            rl_completion_append_character = '\0';

            const unsigned len_target_word = strlen(target_word);

            if (target_word[0] != '\0') {
                for ( ; index < readline_helper::len_dictionary; ++index) {
                    if (strncmp(target_word, readline_helper::dictionary[index], len_target_word) == 0) {
                        char *ret = reinterpret_cast<char *>(calloc(readline_helper::len_word_maximum + 1, sizeof(char)));
                        strcpy(ret, readline_helper::dictionary[index]);
                        ++index;
                        return ret;
                    }
                }
            }

            index = 0;
            return (char *)NULL;

        }

        //a list of word separators used for completion
        char word_separator[] = " \t\n\"\'`@$><=;|&{}()[]!?";

        char * return_word_separator_list() {
            return readline_helper::word_separator;
        }

        void set_completion(const string &dictionary_file_name) {

            //set a completion function
            rl_completion_entry_function = readline_helper::completion_generator;

            //a function which returns a list of word separators
            rl_completion_word_break_hook = readline_helper::return_word_separator_list;

            ifstream ifs(dictionary_file_name.c_str());
            if (!ifs) {
                cout << "Couldn't open the dictionary file [ " << dictionary_file_name << " ].\n";
                ifs.close();
                exit(1);
            }

            const unsigned maximum_line_length = 200;
            char line[maximum_line_length];

            unsigned index = 0;

            while (true) {
                ifs.getline(line, maximum_line_length);
                if (!ifs) {
                    break;
                }
                if (line[0] == '\0') {
                    continue;
                }
                readline_helper::len_word_maximum = max(readline_helper::len_word_maximum, (unsigned)strlen(line));
                char *p = reinterpret_cast<char *>(calloc(maximum_line_length, sizeof(char)));
                strcpy(p, line);
                readline_helper::dictionary[index] = p;
                ++index;
            }

            readline_helper::len_dictionary = index;

        }

        //} tab-completion

    }

#endif

