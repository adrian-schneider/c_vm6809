#ifndef _OS_H_
#define _OS_H_

#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>

#undef ARCH_WINDOWS
#undef ARCH_LINUX
#undef ARCH_MACOSX

#ifdef _WIN32
#define ARCH_WINDOWS
#pragma message("### Building for Windows.")
#endif // _WIN32

#ifdef __linux__
#define ARCH_LINUX
#pragma message("### Building for Linux.")
#endif // __linux__

#ifdef __APPLE__
#define ARCH_MACOSX
#pragma message("### Building for Apple MacOSX.")
#endif // __APPLE__


void os_sprintf(char *buf, size_t bufsize, const char* fmt, ...);

void os_clear_input();
int os_getchar_nowait(char *ch);

#ifdef ARCH_MACOSX
#define OS_TERM_RAW os_term_raw()
void os_term_raw(void);

#define OS_TERM_NORM os_term_norm()
void os_term_norm(void);

#else
#define OS_TERM_RAW ;
#define OS_TERM_NORM ;
#endif // ARCH_MACOSX

#endif // _OS_H_
