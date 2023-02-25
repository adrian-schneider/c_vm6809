#ifndef __VM6809_H__
#define __VM6809_H__

#define CPU_CLEAR 0
#define CPU_SET 1

#define CPU_TRANSFER 0
#define CPU_TRANSFER_SWAP 1

typedef enum {
    sc_ok=0
    , sc_invalid_opcode
    , sc_invalid_postcode
    , sc_invalid_logicop
    , sc_invalid_register
    /* codes after are considered program errors. */
    , sc_program_error
    , sc_undefined_adressing_mode
    , sc_unsupported_page2_opcode
    , sc_unsupported_page3_opcode
    , sc_unsupported_opcode
} cpu_status_code;

typedef struct {
    cpu_status_code code;
} cpu_machine_status;

cpu_machine_status cpu_status;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t u;
    uint16_t s;
    uint16_t pc;
    union {
        struct {
            uint8_t b;
            uint8_t a;
        };
        uint16_t d;
    };
    uint8_t dp;
    uint8_t cc;
    uint8_t _irq; /* Interrupt request flags */
} cpu_registers_6809;

cpu_registers_6809 cpu_regs;

typedef enum {
    cc_e=128, cc_f=64, cc_h=32, cc_i=16, cc_n=8, cc_z=4, cc_v=2, cc_c=1
} cpu_ccflags_6809;

typedef enum {
    irq_rst=128, irq_irq=64, irq_firq=32, irq_nmi=16, irq_swi=8, irq_swi2=4,
    irq_swi3=2
} cpu_irqflags_6809;

void cpu_init6809(void);
void cpu_execirq(void);
void cpu_execop(void);
void cpu_reset(uint8_t set);
void cpu_irq(uint8_t set);
void cpu_firq(uint8_t set);
void cpu_nmi(uint8_t set);

#endif // __VM6809_H__
