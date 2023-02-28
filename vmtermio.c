/* Simplistic Terminal IO
*/

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "os.h"
#include "vm6809.h"
#include "vmtermio.h"
#include "vmmemory.h"

typedef enum lsr_flags {
    /* TODO: error condition */
    /* These values are used with LSR, IER and IIR. */
    xxr_rbr_full=128, xxr_thr_empty=64
} lsr_flags;

typedef struct uart_registers {
    uint8_t rbr;
    uint8_t thr;
    uint8_t lsr;
    uint8_t ier;
    uint8_t iir;
    uint8_t lcr;
} uart_registers;

static uart_registers regs;

static void _none(void) {}

static void (*issue_irq)(void) = _none;

/* TODO: error if read when not full */
uint8_t tio_read_rbr(void) {
    regs.lsr &= ~xxr_rbr_full;
    return regs.rbr;
}

/* TODO: error if written when not empty */
void tio_write_thr(uint8_t data) {
    regs.lsr &= ~xxr_thr_empty;
    regs.thr = data;
}

uint8_t tio_read_lsr(void) {
    return regs.lsr;
}

void tio_write_ier(uint8_t data) {
    regs.ier = data;
}

uint8_t tio_read_ier(void) {
    return regs.ier;
}

uint8_t tio_read_iir(void) {
    return regs.iir;
}

void tio_write_lcr(uint8_t data) {
    regs.lcr = data;
}

uint8_t tio_read_lcr(void) {
    return regs.lcr;
}

static void _irq(void) {
    cpu_irq(1);
}

void tio_init(void) {
    regs.lsr = xxr_thr_empty;
    issue_irq = _irq;
}

/* Usually implemented by the CPU. */
static void _cpusendc(char cc) {
    mem_wr8(TIO_ADDR_THR, cc);
}

/* Usually implemented by the CPU. */
static uint8_t _cpurecvc(char *cc) {
    uint8_t full = mem_rd8(TIO_ADDR_LSR)&xxr_rbr_full;
    if (full) {
        *cc = mem_rd8(TIO_ADDR_RBR);
    }
    return full;
}

static void _generate_interrupt(uint8_t flag) {
    if (regs.ier & flag) {
        regs.iir |= flag;
        (*issue_irq)();
    }
}

/* Handle a request to send a character to the terminal. */
static void _sendc(void) {
    if (!(regs.lsr&xxr_thr_empty)) {
        char cc = regs.thr;
        regs.lsr |= xxr_thr_empty;
        _generate_interrupt(xxr_thr_empty);
        os_putchar_nowait(cc);
        if (cc == '\r') {
            os_putchar_nowait('\n');
        }
    }
}

/* Handle a received character from the terminal.
   Optionally issue an interrupt to the CPU.
*/
static void _recvc(void) {
    char cc;
    if (os_getchar_nowait(&cc)) {
        if (!(regs.lsr&xxr_rbr_full)) {
            regs.rbr = toupper(cc);
            regs.lsr |= xxr_rbr_full;
            _generate_interrupt(xxr_rbr_full);
        }
    }
}

static void _internal_loopback(void) {
    char cc;
    if (_cpurecvc(&cc)) {
        _cpusendc(cc);
    }
}

void tio_exec(void) {
    _recvc();
    _sendc();
    //_internal_loopback();
}
