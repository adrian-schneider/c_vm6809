/* Implement an 6809 virtual machine.

Enable debugging behaviours.
DEBUG_VM6809

Default path to read files from.
VM6809_DEFAULT_PATH=\"C:\\Users\\asc\\Documents\\6809asm\"

Name of the bin file read at startup.
VM6809_DEFAULT_FILE=\"6809.bin\"

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "os.h"
#include "vm6809.h"
#include "vmmemory.h"
#include "vmmonitor.h"
#include "vmtermio.h"

#ifndef VM6809_DEFAULT_PATH
  #ifdef ARCH_WINDOWS
    #define VM6809_DEFAULT_PATH "."
  #else
    #define VM6809_DEFAULT_PATH "."
  #endif // ARCH_WINDOWS
#endif // VM6809_DEFAULT_PATH

#ifndef VM6809_DEFAULT_FILE
#define VM6809_DEFAULT_FILE "x6809.bin"
#endif // VM6809_DEFAULT_FILE

#ifndef VM6809_DEFAULT_START
#define VM6809_DEFAULT_START 0x4000
#endif // VM6809_DEFAULT_START

int main_perform_tio = 0;

static void _welcome(void) {
    puts("                   __   ___   ___   ___  ");
    puts("                  / /  / _ \\ / _ \\ / _ \\ ");
    puts("__   ___ __ ___  / /_ | (_) | | | | (_) |");
    puts("\\ \\ / / '_ ` _ \\| '_ \\ > _ <| | | |\\__, |");
    puts(" \\ V /| | | | | | (_) | (_) | |_| |  / / ");
    puts("  \\_/ |_| |_| |_|\\___/ \\___/ \\___/  /_/  ");

    // puts("db    db .88b  d88.    dD   .d888b.  .d88b.  .d888b. ");
    // puts("88    88 88'YbdP`88   d8'   88   8D .8P  88. 88' `8D ");
    // puts("Y8    8P 88  88  88  d8'    `VoooY' 88  d'88 `V8o88' ");
    // puts("`8b  d8' 88  88  88 d8888b. .d~~~b. 88 d' 88    d8'  ");
    // puts(" `8bd8'  88  88  88 88' `8D 88   8D `88  d8'   d8'   ");
    // puts("   YP    YP  YP  YP `8888P  `Y888P'  `Y88P'   d8'    ");

#ifdef DEBUG_VM6809
    puts("Debug");
#endif // DEBUG_VM6809

    puts("");
}

static void _load_default_bin(char *from_path) {
    mon_getexepath(from_path, monitor.l_dftpath, sizeof(monitor.l_dftpath));
#ifdef VM6809_DEFAULT_PATH
    os_sprintf(monitor.l_dftpath, sizeof(monitor.l_dftpath), "%s", VM6809_DEFAULT_PATH);
#endif // VM6809_DEFAULT_PATH
    os_sprintf(monitor.l_path, sizeof(monitor.l_path), "%s", monitor.l_dftpath);
    mon_setfile(VM6809_DEFAULT_FILE);
    monitor.l_start = VM6809_DEFAULT_START;
    mon_read(monitor.l_file, monitor.l_start);
}

int main(int argc, char *argv[])
{
    cpu_init6809();
    mem_init();
    mon_init();
    tio_init();

    _welcome();
    _load_default_bin(argv[0]);

    cpu_reset(CPU_SET);
    cpu_execirq();
    while (1) {
        if (main_perform_tio)
            tio_exec();
        cpu_execirq();
        mon_trap_on_sigint();
        if (cpu_regs._trap)
            mon_execmon();
        cpu_execop();
    }
    printf("\n");
    return 0;
}
