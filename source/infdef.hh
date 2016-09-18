#ifndef __INF251_INFDEF__54090249
#define __INF251_INFDEF__54090249

/**
 * This header defines the defaults for the INF251 project and is
 * meant to be included in every translation unit.
 */

//#if __cplusplus < 201103L
//#error "A C++11 compiler is needed to compile this project"
//#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

inline namespace fmt {
    /* fmtlib doesn't have a println function, so we add our own */
    inline void println(CStringRef format, ArgList args) {
        print(format, args);
        print("\n");
    }

    inline void println(std::FILE *fh, CStringRef format, ArgList args) {
        print(fh, format, args);
        print(fh, "\n");
    }

    /* write something and promptly die */
    inline void fatal(CStringRef format, ArgList args) {
        println(stderr, format, args);
        std::terminate();
    }

    FMT_VARIADIC(void, println, CStringRef)
    FMT_VARIADIC(void, println, std::FILE*, CStringRef)
    FMT_VARIADIC(void, fatal, CStringRef)
}

#endif //__INF251_INFDEF__54090249
