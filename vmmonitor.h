#ifndef _VMMONITOR_H_
#define _VMMONITOR_H_

typedef enum {
    mf_halt=128, mf_trace=64, mf_step=32
} monitor_flags;

#define NUMBER_OF_BREAK_POINTS 4 /* maximum 16 as b_enables is 16 bit. */
#define TRACE_PAGE_LINES 25
#define CMD_LINE_LENGTH 80
#define CMD_PROMPT_LENGTH 8

#define CMDOPT_NONE  '\0'
#define CMDOPT_SET  '!'
#define CMDOPT_CLEAR  '.'
#define CMDOPT_ASK  '?'
#define CMDOPT_DEFAULT '*'
#define CMDOPT_AT '@'

typedef struct monitor_state {
    uint16_t b_adress[NUMBER_OF_BREAK_POINTS];
    uint16_t b_enabled;
    uint16_t m_address;
    uint16_t m_chunk;
    uint8_t flags;
    uint16_t l_start;
    char l_file[CMD_LINE_LENGTH];
    char l_path[CMD_LINE_LENGTH];
    char l_dftpath[CMD_LINE_LENGTH];
} monitor_state;

monitor_state monitor;

void mon_init(void);
void mon_printreg(void);
void mon_printreg_short(uint8_t hdr);
void mon_printmem(uint16_t addr, uint16_t cnt);
void mon_printvectors();
void mon_printstatus();
void mon_read(char * file, uint16_t start);
int mon_read_bin(char *file, uint16_t start);
void mon_listdir(char *path);
void mon_setfile(char *file);
void mon_getexepath(char *path, char *exepath, size_t size);
void mon_getpath(char *path, size_t size);
void mon_assemble(char *file);
void mon_execmon(void);

#endif // _VMMONITOR_H_
