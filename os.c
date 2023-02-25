#include "os.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef ARCH_WINDOWS
#include <conio.h>
#endif // ARCH_WINDOWS

void _assert(int cond, char *message) {
    if (!cond) {
        fprintf(stderr, "*** Assertion failure: %s\n", message);
        exit(-99);
    }
}

int _vsprintf(char *buf, size_t bufsize, const char* fmt, va_list argp) {
    #ifdef ARCH_WINDOWS
    return vsprintf_s(buf, bufsize, fmt, argp);

    #elif defined ARCH_MACOSX
    return vsnprintf(buf, bufsize, fmt, argp);

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}

void os_sprintf(char *buf, size_t bufsize, const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    int ret = _vsprintf(buf, bufsize, fmt, argp);
    va_end(argp);

    _assert((ret >= 0) && (ret < bufsize), "sprintf buffer too short.");
}

void os_clear_input() {
    #ifdef ARCH_WINDOWS
    while (_kbhit()) _getche();

    #elif defined ARCH_MACOSX
    #pragma message("### not implemented")

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}

int os_getchar_nowait(char *ch) {
    #ifdef ARCH_WINDOWS
    if (_kbhit()) {
        *ch = _getch();
        return 1;
    }
    return 0;

    #elif defined ARCH_MACOSX
    #pragma message("### not implemented")

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}
