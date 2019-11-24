#ifndef is_debug_included

    #define is_debug_included

    #include "./misc.h"

    namespace debug {

        template <typename T>
        void debug_print(const T &t) {
            #ifndef NDEBUG
            cout << misc::fg_green << t << misc::no_color << "\n";
            #endif
        }

        template <typename T1, typename T2>
        void debug_print(const T1 &t1, const T2 &t2) {
            #ifndef NDEBUG
            cout << misc::fg_green << t1 << t2 << misc::no_color << "\n";
            #endif
        }

        template <typename T1, typename T2, typename T3>
        void debug_print(const T1 &t1, const T2 &t2, const T3 &t3) {
            #ifndef NDEBUG
            cout << misc::fg_green << t1 << t2 << t3 << misc::no_color << "\n";
            #endif
        }

        template <typename T1, typename T2, typename T3, typename T4>
        void debug_print(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
            #ifndef NDEBUG
            cout << misc::fg_green << t1 << t2 << t3 << t4 << misc::no_color << "\n";
            #endif
        }

    }

#endif

