#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

static struct termios _term_norm;
static struct termios _term_raw;
static int _initialized = 0;

static int _fd_stdin;
static int _fd_stdout;

void term_init(void) {
    if (!_initialized) {
        _fd_stdin = fileno(stdin);
        _fd_stdout = fileno(stdout);
        tcgetattr(_fd_stdin, &_term_norm);
        _term_raw = _term_norm;
        _term_raw.c_lflag &= ~(ICANON | ECHO);
        _term_raw.c_cc[VMIN] = 0;
        _term_raw.c_cc[VTIME] = 0;
        _initialized = 1;
    }
}

void term_norm(void) {
    term_init();
    tcsetattr(_fd_stdin, TCSANOW, &_term_norm);
    puts("Norm");
}

void term_raw(void) {
    term_init();
    tcsetattr(_fd_stdin, TCSANOW, &_term_raw);
    puts("Raw");
}

int main(int argc, char *argv[]) {
    term_raw();

    char ch = '\0';
    char chup;
    while (ch != 27) {
        if (read(_fd_stdin, &ch, 1)) {
            chup = toupper(ch);
            if (ch != 27)
                write(_fd_stdout, &chup, 1);
        }
    }

    term_norm();
}
