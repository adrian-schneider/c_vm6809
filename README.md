# c_vm6809

Virtual Machine implementing a Motorola 6809 CPU.

The virtual machine reads 6809 machine code programs from binary files \*.bin.
An external assembler is required to produce a bin-file from assembly source.code.
E.g. the one here: https://www.6809.org.uk/asm6809/

Terminal IO is provided by a simplified memory mapped UART.

Resources:
Data sheets by Motorola.
Online 6809 Emulator: http://6809.uk/
A portable cross assembler targeting the Motorola 6809 and Hitachi 6309: https://www.6809.org.uk/asm6809/
Motorola 6809 and Hitachi 6309 Programmer's Reference, by Darren Atkinson: https://colorcomputerarchive.com/repo/Documents/Books/Motorola%206809%20and%20Hitachi%206309%20Programming%20Reference%20%28Darren%20Atkinson%29.pdf

