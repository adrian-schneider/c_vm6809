#ifndef _VMTERMIO_H_
#define _VMTERMIO_H_

#ifndef TIO_BASE_ADDRESS
#define TIO_BASE_ADDRESS 0xff00
#endif // TIO_BASE_ADDRESS
#define TIO_ADDR_RBR_THR (TIO_BASE_ADDRESS)
/* Buffer Register.
   The CPU reads data bytes from this register.
*/
#define TIO_ADDR_RBR (TIO_BASE_ADDRESS)
/* Transmitter Holding Register.
   The CPU writes data bytes to this register.
*/
#define TIO_ADDR_THR (TIO_BASE_ADDRESS)
/* Line Status Register.
   The CPU reads from this register to find out whether the RBR is full,
   whether the THR is empty or whether an error condition has been detected.
*/
#define TIO_ADDR_LSR (TIO_BASE_ADDRESS+1)
/* Interrupt Enable Register.
   The CPU writes a byte to this register to tell the UART when to generate
   interrupts. Individual bits within the byte indicate whether to generate an
   interrupt when RBR is full, when THR is empty, or when an error condition
   is detected.
*/
#define TIO_ADDR_IER (TIO_BASE_ADDRESS+2)
/* Interrupt Identification Register.
   The CPU reads this register to find out why the UART generated an interrupt.
*/
#define TIO_ADDR_IIR (TIO_BASE_ADDRESS+3)
/* Line Configuration Register.
   The CPU writes bytes to this register to configure parameters like xxxx.
*/
/* TODO UART parameters. */
#define TIO_ADDR_LCR (TIO_BASE_ADDRESS+4)

uint8_t tio_read_rbr(void);
void tio_write_thr(uint8_t data);
uint8_t tio_read_lsr(void);
void tio_write_ier(uint8_t data);
uint8_t tio_read_ier(void);
uint8_t tio_read_iir(void);
void tio_write_lcr(uint8_t data);
uint8_t tio_read_lcr(void);

void tio_init(void);
void tio_exec(void);

#endif // _VMTERMIO_H_
