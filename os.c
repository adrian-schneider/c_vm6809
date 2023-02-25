#include "os.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef ARCH_WINDOWS
#include <conio.h>
#endif // ARCH_WINDOWS

#ifdef ARCH_MACOSX
#include <termios.h>
#include <unistd.h>
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

#ifdef ARCH_MACOSX
int _mygetch(void) {
    char ch;
    int error;
    static struct termios Otty, Ntty;

    fflush(stdout);
    tcgetattr(0, &Otty);
    Ntty = Otty;

    Ntty.c_iflag  =  0;     /* input mode       */
    //Ntty.c_oflag  =  0;     /* output mode      */
    Ntty.c_lflag &= ~(ICANON);    /* line settings    */
    Ntty.c_lflag &= ~(ECHO);  /* disable echo     */
    Ntty.c_cc[VMIN]  = 0;    /* minimum chars to wait for */
    Ntty.c_cc[VTIME] = 0;   /* minimum wait time    */

    if ((tcsetattr(0, TCSANOW, &Ntty)) == 0) {
        error  = read(0, &ch, 1);        /* get char from stdin */
        tcsetattr(0, TCSANOW, &Otty);   /* restore old settings */
    }

    return (error == 1 ? (int) ch : -1 );
}
#endif // ARCH_MACOSX

void os_clear_input() {
    #ifdef ARCH_WINDOWS
    while (_kbhit()) _getche();

    #elif defined ARCH_MACOSX
    //while (_mygetch() >= 0);

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
    int _ch;
    if ((_ch = _mygetch()) >= 0) {
      *ch = _ch;
      return 1;
    }
    return 0;

    #else
    #pragma message("### not implemented")
    #endif // ARCH_WINDOWS
}
