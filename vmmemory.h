#ifndef _VMMEMORY_H_
#define _VMMEMORY_H_

#define MEMORY_SIZE 64*1024
#define EYECATCHER_16 0x5555

uint8_t memory[MEMORY_SIZE];

void mem_init(void);
void mem_erase(uint8_t data);
uint8_t mem_rd8(uint16_t addr);
void mem_wr8(uint16_t addr, uint8_t data);
uint16_t mem_rd16(uint16_t addr);
void mem_wr16(uint16_t addr, uint16_t data);

#endif // _VMMEMORY_H_
