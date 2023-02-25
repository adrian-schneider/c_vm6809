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

#define WELCOME  "Virtual 6809 System Monitor\n"
#define WELCOME_ "---------------------------\n"
#ifdef DEBUG_VM6809
#undef WELCOME
#define WELCOME "Virtual 6809 System Monitor  DEBUG\n"
#endif // DEBUG_VM6809

#ifndef VM6809_DEFAULT_PATH
  #ifdef ARCH_WINDOWS
    #define VM6809_DEFAULT_PATH ".\\"
  #else
    #define VM6809_DEFAULT_PATH "./"
  #endif // ARCH_WINDOWS
#endif // VM6809_DEFAULT_PATH

#ifndef VM6809_DEFAULT_FILE
#define VM6809_DEFAULT_FILE "x6809.bin"
#endif // VM6809_DEFAULT_FILE

#ifndef VM6809_DEFAULT_START
#define VM6809_DEFAULT_START 0x4000
#endif // VM6809_DEFAULT_START

int main(int argc, char *argv[])
{
    cpu_init6809();
    mem_init();
    mon_init();
    tio_init();

    printf(WELCOME_);
    printf(WELCOME);
    printf(WELCOME_);

    mon_getexepath(argv[0], monitor.l_dftpath, sizeof(monitor.l_dftpath));
#ifdef VM6809_DEFAULT_PATH
    os_sprintf(monitor.l_dftpath, sizeof(monitor.l_dftpath), "%s", VM6809_DEFAULT_PATH);
#endif // VM6809_DEFAULT_PATH
    os_sprintf(monitor.l_path, sizeof(monitor.l_path), "%s", monitor.l_dftpath);
    mon_setfile(VM6809_DEFAULT_FILE);
    monitor.l_start = VM6809_DEFAULT_START;
    mon_read(monitor.l_file, monitor.l_start);

    cpu_reset(CPU_SET);
    cpu_execirq();
    while (1) {
        tio_exec();
        cpu_execirq();
        mon_execmon();
        cpu_execop();
    }
    printf("\n");
    return 0;
}
