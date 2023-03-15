#include <stdint.h>
#include <string.h>

#include "vmmemory.h"
#include "vmtermio.h"

uint8_t memory[MEMORY_SIZE];

void mem_init(void) {
    mem_erase(0);
}

void mem_erase(uint8_t data) {
    memset(memory, 0, sizeof(memory));
}

uint8_t mem_rd8(uint16_t addr) {
    switch (addr) {
        case TIO_ADDR_RBR_THR:
            return tio_read_rbr();
        case TIO_ADDR_LSR:
            return tio_read_lsr();
        case TIO_ADDR_IER:
            return tio_read_ier();
        case TIO_ADDR_IIR:
            return tio_read_iir();
        case TIO_ADDR_LCR:
            return tio_read_lcr();
        default:
            return memory[addr];
    }
}

void mem_wr8(uint16_t addr, uint8_t data) {
    switch (addr) {
        case TIO_ADDR_RBR_THR:
            tio_write_thr(data);
            break;
        case TIO_ADDR_LSR:
            /* read-only */
            break;
        case TIO_ADDR_IER:
            tio_write_ier(data);
            break;
        case TIO_ADDR_IIR:
            /* read-only */
            break;
        case TIO_ADDR_LCR:
            tio_write_lcr(data);
        default:
            memory[addr] = data;
    }
}

uint16_t mem_rd16(uint16_t addr) {
    return memory[addr]<<8 | memory[addr+1];
}

void mem_wr16(uint16_t addr, uint16_t data) {
    memory[addr] = data>>8;
    memory[addr+1] = data&0xff;
}
