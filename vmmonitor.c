#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "os.h"
#include "vmmonitor.h"
#include "vm6809.h"
#include "vmmemory.h"

#if (defined ARCH_MACOSX) || (defined ARCH_LINUX)
#define CMD_PATH_SEPARATOR '/'
#define CMD_CURRENT_PATH "./"
#define CMD_LIST_FILES "cd %s && ls -1 *.bin"

#elif defined ARCH_WINDOWS
#define CMD_PATH_SEPARATOR '\\'
#define CMD_CURRENT_PATH ".\\"
#define CMD_LIST_FILES "cd %s && dir /b *.bin"

#endif // ARCH_MACOSX

#define MON_NUMBER_OF_BREAKPOINTS 4

#define MON_CLEAR_BREAKPOINT 0
#define MON_SET_BREAKPOINT 1

monitor_state monitor;

typedef enum _breakpoint_status {
    bps_enabled=128, bps_set=64
} _breakpoint_status;

typedef struct _breakpoint_struct {
    uint16_t address;
    uint8_t status;
} _breakpoint_struct;

static _breakpoint_struct breakpoint[MON_NUMBER_OF_BREAKPOINTS];
static uint8_t _last_breakpoint = 0;


static volatile sig_atomic_t _sigint_received = 0;

static void _inst_signal_handler(void);

static void _sig_handler(int _) {
    (void)_;
    _sigint_received = 1;
    _inst_signal_handler();
}

static void _inst_signal_handler() {
    signal(SIGINT, _sig_handler);
#ifdef SIGBREAK
    signal(SIGBREAK, _sig_handler);
#endif // SIGBREAK
}

void mon_init(void) {
    memset(&monitor, 0, sizeof(monitor));
    cpu_regs._trap = tc_halt;
    monitor.m_chunk = 64;
    _inst_signal_handler();
}

void mon_printreg(void) {
    printf(" A:  %02x (%5d/%+6d)   B:  %02x (%5d/%+6d)\n",
           cpu_regs.a, cpu_regs.a, (int8_t)cpu_regs.a,
           cpu_regs.b, cpu_regs.b, (int8_t)cpu_regs.b);
    char aa = isprint(cpu_regs.a)?cpu_regs.a:'\0';
    char bb = isprint(cpu_regs.b)?cpu_regs.b:'\0';
    printf("    %c%c%c                     %c%c%c\n",
           aa?'\'':'-', aa?aa:'-', aa?'\'':'-',
           bb?'\'':'-', bb?bb:'-', bb?'\'':'-');
    printf(" D:%04x (%5d/%+6d)\n",
           cpu_regs.d, cpu_regs.d, (int16_t)cpu_regs.d);
    printf(" X:%04x (%5d/%+6d)   Y:%04x (%5d/%+6d)\n",
           cpu_regs.x, cpu_regs.x, (int16_t)cpu_regs.x,
           cpu_regs.y, cpu_regs.y, (int16_t)cpu_regs.y);
    printf(" S:%04x (%5d/%+6d)   U:%04x (%5d/%+6d)\n",
           cpu_regs.s, cpu_regs.s, (int16_t)cpu_regs.s,
           cpu_regs.u, cpu_regs.u, (int16_t)cpu_regs.u);
    printf("PC:%04x                 DP:  %02x                 CC:%02x",
           cpu_regs.pc, cpu_regs.dp, cpu_regs.cc);
    printf(" (%c%c%c%c%c%c%c%c)\n",
           cpu_regs.cc&cc_e?'E':'-',
           cpu_regs.cc&cc_f?'F':'-',
           cpu_regs.cc&cc_h?'H':'-',
           cpu_regs.cc&cc_i?'I':'-',
           cpu_regs.cc&cc_n?'N':'-',
           cpu_regs.cc&cc_z?'Z':'-',
           cpu_regs.cc&cc_v?'V':'-',
           cpu_regs.cc&cc_c?'C':'-');
}

/* Newlines are at the beginning of the strings printed.
   This way, any character output by the 6809 appear after
   the trace text and not garbling it.
*/
void mon_printreg_short(uint8_t hdr) {
    static cpu_registers_6809 prev_regs;
    char spc[5] = ".   ";
    char sa[3] = ". ";
    char sac[2] = " ";
    char sb[3] = ". ";
    char sbc[2] = " ";
    char sdp[3] = ". ";
    char ss[5] = ".   ";
    char su[5] = ".   ";
    char sx[5] = ".   ";
    char sy[5] = ".   ";
    char scc[9] = ".       ";
    if (hdr) {
        printf("\nPC    @PC          D/A  D/B  DP S    U    X    Y    CC");
    }
    if ((cpu_regs.pc != prev_regs.pc) || hdr) {
        sprintf(spc, "%04x", cpu_regs.pc);
    }
    if ((cpu_regs.a != prev_regs.a) || hdr) {
        sprintf(sa, "%02x", cpu_regs.a);
        sprintf(sac, "%c", isprint(cpu_regs.a)?cpu_regs.a:' ');
    }
    if ((cpu_regs.b != prev_regs.b) || hdr) {
        sprintf(sb, "%02x", cpu_regs.b);
        sprintf(sbc, "%c", isprint(cpu_regs.b)?cpu_regs.b:' ');
    }
    if ((cpu_regs.dp != prev_regs.dp) || hdr) {
        sprintf(sdp, "%02x", cpu_regs.dp);
    }
    if ((cpu_regs.s != prev_regs.s) || hdr) {
        sprintf(ss, "%04x", cpu_regs.s);
    }
    if ((cpu_regs.u != prev_regs.u) || hdr) {
        sprintf(su, "%04x", cpu_regs.u);
    }
    if ((cpu_regs.x != prev_regs.x) || hdr) {
        sprintf(sx, "%04x", cpu_regs.x);
    }
    if ((cpu_regs.y != prev_regs.y) || hdr) {
        sprintf(sy, "%04x", cpu_regs.y);
    }
    if ((cpu_regs.cc != prev_regs.cc) || hdr) {
        char flags[] = "EFHINZVC0";
        memset(scc, '\0', sizeof(scc));
        uint8_t ii=0;
        if (cpu_regs.cc&cc_e) scc[ii++] = flags[0];
        if (cpu_regs.cc&cc_f) scc[ii++] = flags[1];
        if (cpu_regs.cc&cc_h) scc[ii++] = flags[2];
        if (cpu_regs.cc&cc_i) scc[ii++] = flags[3];
        if (cpu_regs.cc&cc_n) scc[ii++] = flags[4];
        if (cpu_regs.cc&cc_z) scc[ii++] = flags[5];
        if (cpu_regs.cc&cc_v) scc[ii++] = flags[6];
        if (cpu_regs.cc&cc_c) scc[ii++] = flags[7];
        if (!cpu_regs.cc) scc[ii] = flags[8];
    }
    printf("\n%4s  %02x %02x %02x %02x  %2s %1s %2s %1s %2s %4s %4s %4s %4s %s",
           spc,
           memory[cpu_regs.pc],
           memory[cpu_regs.pc+1],
           memory[cpu_regs.pc+2],
           memory[cpu_regs.pc+3],
           sa, sac, sb, sbc, sdp, ss, su, sx, sy, scc);
    prev_regs = cpu_regs;
}

void mon_printmem(uint16_t addr, uint16_t cnt) {
    printf("       0  1  2  3   4  5  6  7   8  9  a  b   c  d  e  f"
           "  0123456789abcdef\n");
    char pchars[17];
    memset(pchars, 0, sizeof(pchars));
    uint16_t ii;
    for (ii=0; ii<cnt; ii++) {
        uint16_t ofs = ii;
        uint8_t byte = mem_rd8(addr+ofs);
        if (ii%16 == 0) {
            printf("%s%s%s%04x", ii?"  ":"", ii?pchars:"", ii?"\n":"",
                   (addr+ii)&0xffff);
            memset(pchars, 0, sizeof(pchars));
        }
        pchars[ii%16] = isgraph(byte)?byte:'.';
        printf(byte?"%s%02x":"%s .", ii%4?" ":"  ", byte);
    }
    if (cnt%16) {
        for (ii=cnt%16; ii<16; ii++) {
            printf(ii%4?"   ":"    ");
        }
    }
    printf("  %s\n", pchars);
}

static void _pvector(uint16_t addr, char *name) {
    printf("  %4s:%04x %04x\n", name, addr, mem_rd16(addr));
}

void mon_printvectors(void) {
    _pvector(0xfff2, "swi3");
    _pvector(0xfff4, "swi2");
    _pvector(0xfff6, "firq");
    _pvector(0xfff8, "irq");
    _pvector(0xfffa, "swi");
    _pvector(0xfffc, "nmi");
    _pvector(0xfffe, "rst");
}

void mon_printstatus(void) {
    printf("Status:");
    switch (cpu_status.code) {
        case sc_ok:
            printf("ok");
            break;
        case sc_invalid_opcode:
            printf("invalid opcode");
            break;
        case sc_invalid_postcode:
            printf("invalid post-code");
            break;
        default:
            printf("undefined");
    }
    printf("\n");
}

#define NO_INPUT 0
#define INPUT_OK 1
#define TOO_LONG 2
/* https://stackoverflow.com/questions/4023895/how-do-i-read-a-string-entered-by-the-user-in-c
*/
static int _getline(char *prmpt, char *buff, size_t sz) {
    int ch, extra;
    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL) {
        return NO_INPUT;
    }
    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : INPUT_OK;
    }
    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return INPUT_OK;
}

/* Return the value given by a string in hex or decimal.*/
static int _getval(char *str) {
    int val = 0;
    if (str[0] == '.') {
        sscanf(str+1, "%d", &val);
    }
    else {
        sscanf(str, "%x", &val);
    }
    return val;
}

static uint8_t _readoptval(int *pval) {
    char cmdline[CMD_LINE_LENGTH+1];

    int rc = _getline("?", cmdline, sizeof(cmdline));
    if ((rc != NO_INPUT) && (cmdline[0])) {
        *pval = _getval(cmdline);
        return 1;
    }
    return 0;
}

static uint8_t *_pregfromc8(char c) {
    if (c == 'a') return &cpu_regs.a;
    if (c == 'b') return &cpu_regs.b;
    if (c == 'c') return &cpu_regs.cc;
    if (c == 'p') return &cpu_regs.dp;
    return NULL;
}

static uint16_t *_pregfromc16(char c) {
    if (c == 'd') return &cpu_regs.d;
    if (c == 'u') return &cpu_regs.u;
    if (c == 's') return &cpu_regs.s;
    if (c == 'x') return &cpu_regs.x;
    if (c == 'y') return &cpu_regs.y;
    return NULL;
}

void mon_read(char *file, uint16_t start) {
    int bytes_read = mon_read_bin(file, start);
    if (bytes_read) {
        printf("  Read %d bytes\n  from %s\n  to %04x\n",
               bytes_read, file, start);
        mem_wr16(0xfffe, start);
    }
}

int mon_read_bin(char *file, uint16_t start) {
    int nret = 0;
    int addr = start;
    FILE *fbin;
    if ((fbin = fopen(file, "rb"))) {
        int c;
        while ((EOF != (c = fgetc(fbin))) && (addr < sizeof(memory))) {
            memory[addr++] = c;
            nret++;
        }
        fclose(fbin);
    }
    else {
        printf("  could not open %s\n", file);
    }
    return nret;
}

static uint8_t _is_print_hdr(int *plines) {
    return (((*plines)%TRACE_PAGE_LINES) == 0);
}

static void _printreg_short(int *plines) {
    mon_printreg_short(_is_print_hdr(plines));
    *plines = (cpu_regs._trap&tc_step)?0:*plines+1;
}

static uint8_t _is_trace_or_step(void) {
    return cpu_regs._trap&(tc_trace|tc_step);
}

static uint8_t _is_trace_only(void) {
    return _is_trace_or_step() == tc_trace;
}

static uint8_t _is_halt_or_step(void) {
    return cpu_regs._trap&(tc_halt|tc_step);
}

static int _execcmd_g(char *cmdline, int *plines) {
    cpu_regs._trap &= ~(tc_halt|tc_step);
    if (cmdline[0]) {
        cpu_regs.pc = _getval(cmdline);
        printf("  Run:%04x\n", cpu_regs.pc);
        *plines = 0;
        if (_is_trace_only()) _printreg_short(plines);
    }
    return 1;
}

static void _execcmd_r(char *cmdline) {
    if (cmdline[0] == CMDOPT_NONE) {
        mon_printreg_short(1);
        puts("");
        return;
    }
    if (cmdline[0] == CMDOPT_ASK) {
        mon_printreg();
        return;
    }
    if (cmdline[0]) {
        uint8_t *preg8 = _pregfromc8(cmdline[0]);
        uint16_t *preg16 = _pregfromc16(cmdline[0]);
        int val = 0;
        if (preg8) {
            printf("  %c:$%02x ", cmdline[0], *preg8);
            if (_readoptval(&val)) {
                *preg8 = val;
            }
            return;
        }
        if (preg16) {
            printf("  %c:$%04x ", cmdline[0], *preg16);
            if (_readoptval(&val)) {
                *preg16 = val;
            }
            return;
        }
        puts("  invalid register\n");
        return;
    }
}

static void _showset(char *name, uint8_t set) {
    printf("  %s %s\n", name, set?"on":"off");
}

static void _execcmd_t(char *cmdline) {
    if (cmdline[0] == CMDOPT_SET) {
        cpu_regs._trap |= tc_trace;
    }
    else if (cmdline[0] == CMDOPT_CLEAR) {
        cpu_regs._trap &= ~tc_trace;
    }
    _showset("trace", cpu_regs._trap&tc_trace);
}

static int _execcmd_s(char *cmdline, int *plines) {
    int leave = 1;
    cpu_regs._trap &= ~tc_halt;
    cpu_regs._trap |= tc_step;
    if (cmdline[0]) {
        cpu_regs.pc = _getval(cmdline);
        printf("  Step:%04x\n", cpu_regs.pc);
        *plines = 0;
        _printreg_short(plines);
        puts("");
        leave = 0;
    }
    return leave;
}

static void _execcmd_m(char *cmdline) {
    if (cmdline[0] == CMDOPT_NONE) {
        int val = 0;
        printf("  %04x:%02x ", monitor.m_address, mem_rd8(monitor.m_address));
        if (_readoptval(&val)) {
            mem_wr8(monitor.m_address, val);
        }
        monitor.m_address++;
    }
    else if (cmdline[0] == CMDOPT_SET) {
        monitor.m_chunk = _getval(cmdline+1);
    }
    else if (cmdline[0] == CMDOPT_ASK) {
        if (cmdline[1] == CMDOPT_AT) {
            monitor.m_address = cpu_regs.pc;
        }
        else if (cmdline[1] != 0) {
            monitor.m_address = _getval(cmdline+1);
        }
        mon_printmem(monitor.m_address, monitor.m_chunk);
        monitor.m_address += monitor.m_chunk;
    }
    else if (cmdline[0] == 'v') {
        mon_printvectors();
    }
    else if (cmdline[0] == '=') {
        int addr = _getval(cmdline+1);
        int val = 0;
        printf("  %04x:%02x ", addr, mem_rd8(addr));
        if (_readoptval(&val)) {
            printf("  %04x-%04x:%02x\n", monitor.m_address, addr, val);
            for (uint16_t ii = monitor.m_address; ii <= addr; ii++) {
                mem_wr8(ii, val);
            }
        }
    }
    else {
        uint8_t twobyte = 0;
        if (cmdline[0] == ':') {
            cmdline++;
            twobyte = 1;;
        }
        int val = 0;
        if (cmdline[0] == '@') {
            monitor.m_address = cpu_regs.pc;
        }
        else if (cmdline[0] != 0) {
            monitor.m_address = _getval(cmdline);
        }
        printf(twobyte?"  %04x:%04x ":"  %04x:%02x ",
               monitor.m_address,
               twobyte?mem_rd16(monitor.m_address):mem_rd8(monitor.m_address)
        );
        if (_readoptval(&val)) {
            if (twobyte) {
                mem_wr16(monitor.m_address, val);
            }
            else {
                mem_wr8(monitor.m_address, val);
            }
        }
        monitor.m_address += twobyte?2:1;
    }
}

static void _execcmd_i(char *cmdline) {
    char irq = cmdline[1];
    if (cmdline[0] == CMDOPT_NONE) {
        printf("  %c%c%c%c%c%c%c\n",
               cpu_regs._irq&irq_rst?'r':'-',
               cpu_regs._irq&irq_nmi?'n':'-',
               cpu_regs._irq&irq_firq?'f':'-',
               cpu_regs._irq&irq_irq?'i':'-',
               cpu_regs._irq&irq_swi?'s':'-',
               cpu_regs._irq&irq_swi2?'2':'-',
               cpu_regs._irq&irq_swi3?'3':'-'
        );
        return;
    }
    if ((cmdline[0] == CMDOPT_SET) || (cmdline[0] == CMDOPT_CLEAR)) {
        uint8_t set = (cmdline[0] == CMDOPT_SET)?1:0;
        switch (irq) {
            case 'r':
                cpu_reset(set);
                _showset("reset", set);
                break;
            case 'n':
                cpu_nmi(set);
                _showset("nmi", set);
                break;
            case 'f':
                cpu_firq(set);
                _showset("firq", set);
                break;
            case 'i':
                cpu_irq(set);
                _showset("irq", set);
                break;
            default:
                puts("  invalid interrupt");
                break;
        }
    }
    return;
}

static void _execcmd_l(char *cmdline) {
    if (cmdline[0] == CMDOPT_ASK) {
        mon_listdir(monitor.l_path);
    }
    else if (cmdline[0] == CMDOPT_SET) {
        if (cmdline[1] == CMDOPT_DEFAULT) {
            os_sprintf(monitor.l_path, sizeof(monitor.l_path), "%s", monitor.l_dftpath);
        }
        else {
            mon_getpath(monitor.l_path, sizeof(monitor.l_path));
        }
    }
    else if (cmdline[0] == CMDOPT_AT) {
        printf("  start:%04x ", monitor.l_start);
        int val = 0;
        if (_readoptval(&val)) {
            monitor.l_start = val;
        }
    }
    else {
        if (cmdline[0] != CMDOPT_DEFAULT) {
            mon_setfile(cmdline);
        }
        mon_read(monitor.l_file, monitor.l_start);
    }
}

static int _find_breakpoint(uint16_t addr) {
    for (int ii=0; ii<MON_NUMBER_OF_BREAKPOINTS; ii++) {
        if (breakpoint[ii].address == addr) {
            return ii;
        }
    }
    return -1;
}

static uint8_t _find_nextbest(uint16_t addr) {
    for (int ii=0; ii<MON_NUMBER_OF_BREAKPOINTS; ii++) {
        if (!(breakpoint[ii].status & bps_set)) {
            return ii;
        }
    }
    for (int ii=0; ii<MON_NUMBER_OF_BREAKPOINTS; ii++) {
        if (!(breakpoint[ii].status & bps_enabled)) {
            return ii;
        }
    }
    uint8_t bpindx = _last_breakpoint++;
    _last_breakpoint %= MON_NUMBER_OF_BREAKPOINTS;
    return bpindx;
}

static void _setclr_breakpoint(uint16_t addr, uint8_t setclr) {
    int bpindx = _find_breakpoint(addr);
    if (setclr) {
        if (bpindx < 0) {
            bpindx = _find_nextbest(addr);
        }
        breakpoint[bpindx].address = addr;
        breakpoint[bpindx].status |= bps_set|bps_enabled;
    }
    else {
        if (bpindx < 0) {
            return;
        }
        breakpoint[bpindx].status &= ~(bps_set|bps_enabled);
    }
    printf("  %01d:%04x %s %s\n"
           , bpindx
           , addr
           , (breakpoint[bpindx].status&bps_set)?"set ":"free"
           , (breakpoint[bpindx].status%bps_enabled)?"enabled ":"disabled"
    );
}

static void _toggle_breakpoint(uint16_t addr) {
    int bpindx = _find_breakpoint(addr);
    if (bpindx > -1) {
        breakpoint[bpindx].status ^= bps_enabled;
        breakpoint[bpindx].status |= bps_set;
        printf("  %01d:%04x %s\n"
               , bpindx
               , addr
               , (breakpoint[bpindx].status&bps_enabled)?"enabled":"disabled"
        );
    }
}

static void _list_breakpoints(void) {
    for (uint8_t ii=0; ii < MON_NUMBER_OF_BREAKPOINTS; ii++) {
        printf("  %01d:%04x %s %s\n"
               , ii
               , breakpoint[ii].address
               , (breakpoint[ii].status&bps_set)?"set ":"free"
               , (breakpoint[ii].status&bps_enabled)?"enabled ":"disabled"
        );
    }
}

static uint8_t _breakpoint_hit(uint16_t addr) {
    for (uint8_t ii=0; ii < MON_NUMBER_OF_BREAKPOINTS; ii++) {
        if (((breakpoint[ii].status & (bps_enabled|bps_set)) == (bps_enabled|bps_set))
            && (breakpoint[ii].address == addr))
        {
            return 1;
        }
    }
    return 0;
}

static void _execcmd_b(char *cmdline) {
    if (cmdline[0] == CMDOPT_NONE) {
        _list_breakpoints();
    }
    else if (cmdline[0] == '*') {
        _toggle_breakpoint(_getval(cmdline+1));
    }
    else if (cmdline[0] == CMDOPT_SET) {
        _setclr_breakpoint(_getval(cmdline+1), MON_SET_BREAKPOINT);
    }
    else if (cmdline[0] == CMDOPT_CLEAR) {
        _setclr_breakpoint(_getval(cmdline+1), MON_CLEAR_BREAKPOINT);
    }
}

static void _execcmd_question(char *cmdline) {
    switch (cmdline[0]) {
        case CMDOPT_NONE:
            puts("Commands");
            puts("  b    Break points.");
            puts("  g    Program execution.");
            puts("  i    Interrupt control and reset.");
            puts("  l    File management.");
            puts("  m    Memory inspection.");
            puts("  r    Register operations.");
            puts("  s    Single stepping control.");
            puts("  t    Tracing control.");
            puts("  ?X   Show help for the X command.");
            puts("  bye  End the simulator.");
            break;
        case 'b':
            puts("b: Break points.");
            puts("  b       List break points (there are four).");
            puts("          Format .N:XXXX* N:number, XXXX:address.");
            puts("          '.' position is free.");
            puts("          '*' break point is enabled.");
            puts("  b!XXXX  Set break point at address XXXX.");
            puts("  b.XXXX  Clear break point at address XXXX.");
            puts("  b*XXXX  Toggle break point at address XXXX.");
            break;
        case 'g':
            puts("g: Program execution.");
            puts("  g      Continue from current PC.");
            puts("  gXXXX  Restart from XXXX.");
            puts("  Hit ctrl-c to return to the monitor.");
            break;
        case 'r':
            puts("r: Register operations.");
            puts("  r   Print registers short.");
            puts("  r?  Print registers full.");
            puts("  rR  Print register content and allow modification.");
            puts("  R: a, b, c=cc, d, p=dp, x, y, u, s");
            break;
        case 't':
            puts("t: Tracing control.");
            puts("  t   Show tracing status.");
            puts("  t!  Begin tracing mode.");
            puts("  t.  End tracing mode.");
            break;
        case 's':
            puts("s: Single stepping control.");
            puts("  s      Single step from current PC.");
            puts("  sXXXX  Restart single stepping from XXXX.");
            break;
        case 'm':
            puts("m: Memory inspection.");
            puts("  mXXXX  Show memory byte at address XXXX.");
            puts("  m:XXXX Show memory byte at the PC.");
            puts("  m:@    Show memory word at the PC.");
            puts("         Optionally supply a new value after the '?'.");
            puts("         Hit Enter to accept the current value.");
            puts("  m      Show next byte of memory.");
            puts("  m=XXXX Fill memory bytes from current address until");
            puts("         address XXXX.");
            puts("         Hit Enter to cancel the filling.");
            puts("  m?XXXX Dump a chunk of memory at address XXXX.");
            puts("  m?@    Dump a chunk of memory at the PC.");
            puts("  m?     Dump next chunk of memory.");
            puts("  m!XXXX Set memory dump chunk size.");
            puts("  mv     Show the vector table.");
            break;
        case 'i':
            puts("i: Interrupt control and reset.");
            puts("  i?   Show interrupt flags.");
            puts("  i!X  Set interrupt X.");
            puts("  i.X  Cleaar X interrupt X.");
            puts("  X: r=reset, n=nmi, f=firq, i=irq, s=swi, 2=swi2, 3=swi3");
            break;
        case 'l':
            puts("l: File management.");
            puts("  lNAME Load the file NAME to the start address");
            puts("        and set the reset vector accordingly.");
            puts("  l*    Reload the previous file.");
            puts("  l?    List bin, hex and asm files.");
            puts("  l!    Show the current directory.");
            puts("        Optionally supply a new path after the '?'.");
            puts("  l!*   Set the path to the default path.");
            puts("        Hit Enter to accept the current value.");
            puts("  l@    Show the start address.");
            puts("        Optionally supply a new start address after the '?'.");
            puts("        Hit Enter to accept the current value.");
            puts("  lNAME Load the file NAME.");
            break;
        default:
            puts("  can't help");
    }
}

static uint8_t _execcmd(char *cmdline, int *plines) {
    char cmd = cmdline[0];
    uint8_t leave = 0;
    if (!strcmp(cmdline, "bye")) exit(0);
    cmdline++;
    switch (cmd) {
        case 'g':
            leave = _execcmd_g(cmdline, plines);
            break;
        case 'r':
            _execcmd_r(cmdline);
            break;
        case 't':
            _execcmd_t(cmdline);
            break;
        case 's':
            leave = _execcmd_s(cmdline, plines);
            break;
        case '?':
            _execcmd_question(cmdline);
            break;
        case 'i':
            _execcmd_i(cmdline);
            break;
        case 'm':
            _execcmd_m(cmdline);
            break;
        case 'l':
            _execcmd_l(cmdline);
            break;
        case 'b':
            _execcmd_b(cmdline);
            break;
        default:
            puts("  invalid command\n");
            break;
    }
    return leave;
}

void mon_listdir(char *path) {
    char cmd[CMD_LINE_LENGTH];
    os_sprintf(cmd, sizeof(cmd), CMD_LIST_FILES, path);
#ifdef DEBUG_VM6809
    printf("DBG:system ''%s''\n", cmd);
#endif // DEBUG_VM6809
    system(cmd);
    puts("");
}

void mon_setfile(char *file) {
    char sep[] = "x";
    *sep = CMD_PATH_SEPARATOR;
    os_sprintf(monitor.l_file, sizeof(monitor.l_file), "%s%s%s"
               , monitor.l_path
               , sep
               , file);
}

void mon_getexepath(char *path, char *exepath, size_t size) {
    os_sprintf(exepath, size, "%s", path);
    char *lastpos = NULL;
    char *pos = exepath;
    while ((pos = strchr(pos, CMD_PATH_SEPARATOR))) {
        lastpos = pos++;
    }
    if (lastpos) {
        *(lastpos+1) = '\0';
    }
    else {
        os_sprintf(exepath, size, "%s", CMD_CURRENT_PATH);
    }
}

void mon_getpath(char *path, size_t size) {
    char line[CMD_LINE_LENGTH];
    char prompt[] = "  ?";
    printf("path:%s\n", path);
    if (_getline(prompt, line, size) == INPUT_OK) {
        if (line[0] != '\0') {
            os_sprintf(path, size, "%s", line);
        }
    }
}

extern int main_perform_tio;

void mon_execmon(void) {
    static int lines = 0;
    static char cmdline[CMD_LINE_LENGTH+1];
    static char cmdprmpt[CMD_PROMPT_LENGTH+1];
    static char brkcode = '\0';

    if (cpu_status.code != sc_ok) {
        cpu_regs._trap |= tc_halt;
        cpu_status.code = sc_ok;
        brkcode = 'E';
    }

    if (_sigint_received) {
        cpu_regs._trap |= tc_halt;
        _sigint_received = 0;
        brkcode = 'H';
    }

    if (_breakpoint_hit(cpu_regs.pc)) {
        cpu_regs._trap |= tc_halt;
        brkcode = 'B';
    }

    if (_is_trace_or_step()) {
        if (_is_trace_only() & _is_print_hdr(&lines)) putchar('\n');
        _printreg_short(&lines);
    }

    if (_is_halt_or_step()) {
        OS_TERM_NORM;

        os_clear_input();
        puts("");
        sprintf(cmdprmpt, "%c@%04x>", brkcode?brkcode:'_', cpu_regs.pc);
        uint8_t leave = 0;
        while (!leave) {
            int rc = _getline(cmdprmpt, cmdline, sizeof(cmdline));
            if ((rc != NO_INPUT) && (cmdline[0])) {
                leave = _execcmd(cmdline, &lines);
            }
        }
        main_perform_tio = 1;
        brkcode = '\0';

        OS_TERM_RAW;
    }
}
