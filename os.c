#include "os.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef ARCH_WINDOWS
#include <conio.h>
#endif // ARCH_WINDOWS

#ifdef ARCH_MACOSX
#include <termios.h>
#include <sys/ioctl.h>
#endif // ARCH_MACOSX

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

void os_clear_screen(void) {
#ifdef ARCH_WINDOWS
    system("cls");
#else
    system("clear");
#endif // ARCH_WINDOWS
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
    return read(_fd_stdin, &ch, 1) == 1;

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}

#ifdef ARCH_MACOSX
static struct termios _term_norm;
static struct termios _term_raw;
static int _fd_stdin;
static int _fd_stdout;
static uint8_t _term_initialized = 0;
#endif // ARCH_MACOSX

void os_putchar_nowait(char ch) {
    #ifdef ARCH_WINDOWS
    putchar(ch);

    #elif defined ARCH_MACOSX
    write(_fd_stdout, &ch, 1)

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}

#ifdef ARCH_MACOSX
static void _init_term(void) {
    if (! _term_initialized) {
        _fd_stdin = fileno(stdin);
        _fd_stdout = fileno(stdout);
        tcgetattr(_fd_stdin, &_term_norm);
        _term_raw = _term_norm;
        _term_raw.c_lflag &= ~(ICANON | ECHO);
        _term_raw.c_cc[VMIN] = 0;
        _term_raw.c_cc[VTIME] = 0;
        _term_initialized = 1;
    }
}

void os_term_raw(void) {
    _init_term();
    tcsetattr(_fd_stdin, TCSANOW, &_term_raw);
#ifdef DEBUG_VM6809
    puts("DBG:term_raw");
#endif // DEBUG_VM6809
}

void os_term_norm(void) {
    _init_term();
    tcsetattr(_fd_stdin, TCSANOW, &_term_norm);
    #ifdef DEBUG_VM6809
        puts("DBG:term_norm");
    #endif // DEBUG_VM6809
}
#endif // ARCH_MACOSX
