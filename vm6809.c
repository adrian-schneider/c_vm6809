/* Implement an 6809 virtual machine.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vm6809.h"
#include "vmmemory.h"

cpu_machine_status cpu_status;
cpu_registers_6809 cpu_regs;


typedef enum cpu_opcode_6809 {
    op_neg_d=0x00
    ,op_neg_n=0x60
    ,op_neg_x=0x70

    ,op_nega=0x40
    ,op_negb=0x50

    , op_invalid=0x01

    , op_page2=0x10
    , op_page3=0x11

    , op_com_d=0x03
    , op_com_n=0x63
    , op_com_x=0x73

    , op_coma=0x43
    , op_comb=0x53

    , op_bita_i=0x85
    , op_bita_d=0x95
    , op_bita_n=0xa5
    , op_bita_x=0xb5

    , op_bitb_i=0xc5
    , op_bitb_d=0xd5
    , op_bitb_n=0xe5
    , op_bitb_x=0xf5

    , op_abx=0x3a

    , op_andcc_i=0x1c

    , op_anda_i=0x84
    , op_anda_d=0x94
    , op_anda_n=0xa4
    , op_anda_x=0xb4

    , op_andb_i=0xc4
    , op_andb_d=0xd4
    , op_andb_n=0xe4
    , op_andb_x=0xf4

    , op_orcc_i=0x1a

    , op_ora_i=0x8a
    , op_ora_d=0x9a
    , op_ora_n=0xaa
    , op_ora_x=0xba

    , op_orb_i=0xca
    , op_orb_d=0xda
    , op_orb_n=0xea
    , op_orb_x=0xfa

    , op_eora_i=0x88
    , op_eora_d=0x98
    , op_eora_n=0xa8
    , op_eora_x=0xb8

    , op_eorb_i=0xc8
    , op_eorb_d=0xd8
    , op_eorb_n=0xe8
    , op_eorb_x=0xf8

    , op_tst_d=0x0d

    , op_inc_d=0x0c
    , op_inc_n=0x6c
    , op_inc_x=0x7c

    , op_inca=0x4c
    , op_incb=0x5c

    , op_dec_d=0x0a
    , op_dec_n=0x6a
    , op_dec_x=0x7a

    , op_deca=0x4a
    , op_decb=0x5a

    , op_jmp_d=0x0e
    , op_jmp_n=0x6e
    , op_jmp_x=0x7e

    , op_clra=0x4f
    , op_clrb=0x5f

    , op_clr_d=0x0f
    , op_clr_n=0x6f
    , op_clr_x=0x7f

    , op_nop=0x12

    , op_lsla=0x48
    , op_lslb=0x58

    , op_lsl_d=0x08
    , op_lsl_n=0x68
    , op_lsl_x=0x78

    , op_lsra=0x44
    , op_lsrb=0x54

    , op_lsr_d=0x04
    , op_lsr_n=0x64
    , op_lsr_x=0x74

    , op_asra=0x47
    , op_asrb=0x57

    , op_asr_d=0x07
    , op_asr_n=0x67
    , op_asr_x=0x77

    , op_rola=0x49
    , op_rolb=0x59

    , op_rol_d=0x09
    , op_rol_n=0x69
    , op_rol_x=0x79

    , op_rora=0x46
    , op_rorb=0x56

    , op_ror_d=0x06
    , op_ror_n=0x66
    , op_ror_x=0x76

    , op_lda_i=0x86
    , op_lda_d=0x96
    , op_lda_n=0xa6
    , op_lda_x=0xb6

    , op_ldb_i=0xc6
    , op_ldb_d=0xd6
    , op_ldb_n=0xe6
    , op_ldb_x=0xf6

    , op_ldd_i=0xcc
    , op_ldd_d=0xdc
    , op_ldd_n=0xec
    , op_ldd_x=0xfc

    , op_ldu_i=0xce
    , op_ldu_d=0xde
    , op_ldu_n=0xee
    , op_ldu_x=0xfe

    , op_ldx_i=0x8e
    , op_ldx_d=0x9e
    , op_ldx_n=0xae
    , op_ldx_x=0xbe

    , op_adda_i=0x8b
    , op_addb_i=0xcb
    , op_addd_i=0xc3

    , op_adda_d=0x9b
    , op_addb_d=0xdb
    , op_addd_d=0xd3

    , op_adda_n=0xab
    , op_addb_n=0xeb
    , op_addd_n=0xe3

    , op_adda_x=0xbb
    , op_addb_x=0xfb
    , op_addd_x=0xf3

    , op_adca_i=0x89
    , op_adcb_i=0xc9

    , op_adca_d=0x99
    , op_adcb_d=0xd9

    , op_adca_n=0xa9
    , op_adcb_n=0xe9

    , op_adca_x=0xb9
    , op_adcb_x=0xf9

    , op_sbca_i=0x82
    , op_sbcb_i=0xc2

    , op_sbca_d=0x92
    , op_sbcb_d=0xd2

    , op_sbca_n=0xa2
    , op_sbcb_n=0xe2

    , op_sbca_x=0xb2
    , op_sbcb_x=0xf2

    , op_suba_i=0x80
    , op_subb_i=0xc0
    , op_subd_i=0x83

    , op_suba_d=0x90
    , op_subb_d=0xd0
    , op_subd_d=0x93

    , op_suba_n=0xa0
    , op_subb_n=0xe0
    , op_subd_n=0xa3

    , op_suba_x=0xb0
    , op_subb_x=0xf0
    , op_subd_x=0xb3

    , op_cmpa_i=0x81
    , op_cmpa_d=0x91
    , op_cmpa_n=0xa1
    , op_cmpa_x=0xb1

    , op_cmpb_i=0xc1
    , op_cmpb_d=0xd1
    , op_cmpb_n=0xe1
    , op_cmpb_x=0xf1

    , op_cmpx_i=0x8c
    , op_cmpx_d=0x9c
    , op_cmpx_n=0xac
    , op_cmpx_x=0xbc

    , op_exg_i=0x1e
    , op_tfr_i=0x1f

    , op_leas_n=0x32
    , op_leau_n=0x33
    , op_leax_n=0x30
    , op_leay_n=0x31

    , op_mul=0x3d

    , op_pshs_i=0x34
    , op_pshu_i=0x36

    , op_puls_i=0x35
    , op_pulu_i=0x37

    , op_jsr_d=0x9d
    , op_jsr_n=0xad
    , op_jsr_x=0xbd

    , op_rts=0x39

    , op_sta_d=0x97
    , op_sta_n=0xa7
    , op_sta_x=0xb7

    , op_stb_d=0xd7
    , op_stb_n=0xe7
    , op_stb_x=0xf7

    , op_std_d=0xdd
    , op_std_n=0xed
    , op_std_x=0xfd

    , op_stu_d=0xdf
    , op_stu_n=0xef
    , op_stu_x=0xff

    , op_stx_d=0x9f
    , op_stx_n=0xaf
    , op_stx_x=0xbf

    , op_sex=0x1d

    , op_daa=0x19

    , op_bcc_r=0x24
    , op_bcs_r=0x25
    , op_beq_r=0x27
    , op_bge_r=0x2c
    , op_bgt_r=0x2e
    , op_bhi_r=0x22
    , op_bhs_r=0x24
    , op_ble_r=0x2f
    , op_blo_r=0x25
    , op_bls_r=0x23
    , op_blt_r=0x2d
    , op_bmi_r=0x2b
    , op_bne_r=0x26
    , op_bpl_r=0x2a
    , op_bra_r=0x20
    , op_lbra_r=0x16
    , op_brn_r=0x21
    , op_bsr_r=0x8d
    , op_lbsr_r=0x17
    , op_bvc_r=0x28
    , op_bvs_r=0x29

    , op_rti=0x3b

    , op_swi=0x3f
} cpu_opcode_6809;

typedef enum cpu_opcode_page2_6809 {
    op2_lds_i=0xce
    , op2_lds_d=0xde
    , op2_lds_n=0xee
    , op2_lds_x=0xfe

    , op2_ldy_i=0x8e
    , op2_ldy_d=0x9e
    , op2_ldy_n=0xae
    , op2_ldy_x=0xbe

    , op2_cmpd_i=0x83
    , op2_cmpd_d=0x93
    , op2_cmpd_n=0xa3
    , op2_cmpd_x=0xb3

    , op2_cmpy_i=0x8c
    , op2_cmpy_d=0x9c
    , op2_cmpy_n=0xac
    , op2_cmpy_x=0xbc

    , op2_sts_d=0xdf
    , op2_sts_n=0xef
    , op2_sts_x=0xff

    , op2_sty_d=0x9f
    , op2_sty_n=0xaf
    , op2_sty_x=0xbf

    , op2_lbcc_r=0x24
    , op2_lbcs_r=0x25
    , op2_lbeq_r=0x27
    , op2_lbge_r=0x2c
    , op2_lbgt_r=0x2e
    , op2_lbhi_r=0x22
    , op2_lbhs_r=0x24
    , op2_lble_r=0x2f
    , op2_lblo_r=0x25
    , op2_lbls_r=0x23
    , op2_lblt_r=0x2d
    , op2_lbmi_r=0x2b
    , op2_lbne_r=0x26
    , op2_lbpl_r=0x2a
    , op2_lbrn_r=0x21
    , op2_lbvc_r=0x28
    , op2_lbvs_r=0x29

    , op2_swi2=0x3f
} cpu_opcode_page2_6809;

typedef enum cpu_opcode_page3_6809 {
    op3_cmpu_i=0x83
    , op3_cmpu_d=0x93
    , op3_cmpu_n=0xa3
    , op3_cmpu_x=0xb3

    , op3_cmps_i=0x8c
    , op3_cmps_d=0x9c
    , op3_cmps_n=0xac
    , op3_cmps_x=0xbc

    , op3_swi3=0x3f
} cpu_opcode_page3_6809;

typedef enum regnumber_6809 {
    rn_pc=0x80, rn_us=0x40, rn_y=0x20, rn_x=0x10, rn_dp=0x08, rn_b=0x04,
    rn_a=0x02, rn_cc=0x01,
    rn_entire=rn_pc|rn_cc|rn_us|rn_y|rn_x|rn_dp|rn_b|rn_a,
    rn_fast=rn_pc|rn_cc
} regnumber_6809;

typedef enum logicop {
    lo_and=0, lo_or, lo_xor, lo_com
} logicop;

typedef enum shiftmode {
    sh_left=0, sh_right=1, sh_aright=2
} shiftmode;

#define ERROREXIT(code, text) _errorexit((code), (text), __FILE__, __LINE__, (char*)__func__)

void _errorexit(int code, char *text, char *filename, int linenumber, char *func) {
    printf("\n*** Error: %s\n", text);
    if (code > sc_program_error) {
        printf("Location: %s(%d):%s\n\n", filename, linenumber, func);
    }
    cpu_status.code = code;
    cpu_regs._trap = tc_trap;
}

void _assert_a_b_d_union(void) {
    cpu_regs.a = 0xa5;
    cpu_regs.b = 0x5a;
    cpu_regs.d = 0x1234;
    if ((cpu_regs.a<<8 | cpu_regs.b) != cpu_regs.d) {
        printf("Registers A, B and D are not aligned. Check cpu_registers_6809.\n");
        printf("  A:%02x, B:%02x, D:%04x\n", cpu_regs.a, cpu_regs.b, cpu_regs.d);
        exit(99);
    }
}

void cpu_init6809(void) {
    _assert_a_b_d_union();
    memset(&cpu_regs, 0, sizeof(cpu_regs));
    cpu_status.code = sc_ok;
}

/* Sign-extend from 5 to 8 bits. */
static uint8_t _sext58(uint8_t bits5) {
    return (bits5&0x10) ? (0xe0|(bits5&0x1f)) : (bits5&0x1f);
}

/* Sign-extend from 8 to 16 bits. */
static uint16_t _sext816(uint8_t bits8) {
    return (bits8&0x80) ? (0xff00|bits8) : bits8;
}

/* Set-reset flags. Clear before set. */
static void _setclrf(uint8_t *reg, uint8_t set, uint8_t clr) {
    *reg &= ~clr;
    *reg |= set;
}

static void _setclrccf(uint8_t set, uint8_t clr) {
    _setclrf(&cpu_regs.cc, set, clr);
}

static void _setclrirqf(uint8_t set, uint8_t clr) {
    _setclrf(&cpu_regs._irq, set, clr);
}

/* Return the 8bit negative sign. 0 is positive. */
static uint8_t _nsign8(uint8_t a) {
    return (a&0x80)>>7;
}

/* Return the 16bit negative sign. 0 is positive. */
static uint8_t _nsign16(uint16_t a) {
    return (a&0x8000)>>15;
}

/* Return the 8bit carry bit. */
static uint8_t _carry8(uint16_t a) {
    return (a&0x0100)>>8;
}

/* Return the 16bit carry bit. */
static uint8_t _carry16(uint32_t a) {
    return (a&0x010000)>>16;
}

static uint8_t _nzero8(uint8_t a) {
    return a;
}

static uint16_t _nzero16(uint16_t a) {
    return a;
}

/* Set the sign and zero flags, clear overflow. */
static void _flags_nz_v8(uint8_t a) {
    uint8_t set=0, clr=cc_v;
    *(_nsign8(a) ? &set : &clr) |= cc_n;
    *((a) ? &clr : &set) |= cc_z;
    _setclrccf(set, clr);
}

/* Set the sign and zero flags, clear overflow. */
static void _flags_nz_v16(uint16_t a) {
    uint8_t set=0, clr=cc_v;
    *(_nsign16(a) ? &set : &clr) |= cc_n;
    *(a ? &clr : &set) |= cc_z;
    _setclrccf(set, clr);
}

/* Set the zero flag according to the value of a. */
static void _flags_z16(uint16_t a) {
    uint8_t set=0, clr=0;
    *(a ? &clr : &set) |= cc_z;
    _setclrccf(set, clr);
}

static uint8_t _sameccf(uint8_t flags) {
    return ((cpu_regs.cc&flags) == flags) || !(cpu_regs.cc&flags);
}

static uint8_t _orccf(uint8_t flags) {
    return cpu_regs.cc&flags;
}

/*
static uint8_t _andccf(uint8_t flags) {
    return ((regs.cc&flags) == flags);
}
*/

static void _decadja(void) {
    uint16_t a74 = cpu_regs.a&0xf0;
    uint8_t a30 = cpu_regs.a&0x0f;
    if ((cpu_regs.cc&cc_c)
            || (a74 > 0x90)
            || ((a74 > 0x80) && (a30 > 9)))
    {
        a74 += 0x60;
    }
    if ((cpu_regs.cc&cc_h) || (a30 > 9)) {
        a30 += 6;
    }
    cpu_regs.a = a74 + a30;
    _flags_nz_v8(cpu_regs.a);
    _setclrccf((a74&0xff00)?cc_c:0, 0);
}

/* Add 8 bit with carry and set flags. */
static uint8_t __addcf8(uint8_t a, uint8_t b, uint8_t carry) {
    uint16_t res = a + b + carry;
    uint8_t set=0, clr=0;
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    *(_carry8(res) ? &set : &clr) |= cc_c;
    *((~(_nsign8(a) ^ _nsign8(b)) & (_nsign8(a) ^ _nsign8(res))) ?
            &set : &clr) |= cc_v;
    *((((a&0x0f) + (b&0x0f))&0x10) ? &set : &clr) |= cc_h;
    _setclrccf(set, clr);
    return res;
}

static uint8_t _addf8(uint8_t a, uint8_t b) {
    return __addcf8(a, b, 0);
}

static uint8_t _addcf8(uint8_t a, uint8_t b) {
    return __addcf8(a, b, (cpu_regs.cc&cc_c)?1:0);
}

/* Add 16 bit and set flags. */
static uint16_t _addf16(uint16_t a, uint16_t b) {
    uint32_t res = a + b;
    uint8_t set=0, clr=0;
    *(_nsign16(res) ? &set : &clr) |= cc_n;
    *(_nzero16(res) ? &clr : &set) |= cc_z;
    *(_carry16(res) ? &set : &clr) |= cc_c;
    *((~(_nsign16(a) ^ _nsign16(b)) & (_nsign16(a) ^ _nsign16(res))) ?
            &set : &clr) |= cc_v;
    _setclrccf(set, clr);
    return res;
}

/* Subtract 8 bit and set flags. */
static uint8_t __subcf8(uint8_t a, uint8_t b, uint8_t carry) {
    uint16_t res = a - b - carry;
    uint8_t set=0, clr=0;
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    *(_carry8(res) ? &set : &clr) |= cc_c;
    *(((_nsign8(a) ^ _nsign8(b)) & (_nsign8(a) ^ _nsign8(res))) ?
            &set : &clr) |= cc_v;
    /* H is undefined on SUB, so leave it as is.
    *((((a&0x0f) - (b&0x0f))&0x10) ? &set : &clr) |= cc_h;
    */
    _setclrccf(set, clr);
    return res;
}

static uint8_t _subf8(uint8_t a, uint8_t b) {
    return __subcf8(a, b, 0);
}

static uint8_t _subcf8(uint8_t a, uint8_t b) {
    return __subcf8(a, b, (cpu_regs.cc&cc_c)?1:0);
}

/* Subtract 8 bit and set flags. */
static uint16_t _subf16(uint16_t a, uint16_t b) {
    uint32_t res = a - b;
    uint8_t set=0, clr=0;
    *(_nsign16(res) ? &set : &clr) |= cc_n;
    *(_nzero16(res) ? &clr : &set) |= cc_z;
    *(_carry16(res) ? &set : &clr) |= cc_c;
    *(((_nsign16(a) ^ _nsign16(b)) & (_nsign16(a) ^ _nsign16(res))) ?
            &set : &clr) |= cc_v;
    _setclrccf(set, clr);
    return res;
}

static uint16_t _mulf8(uint8_t a, uint8_t b) {
    uint16_t res = a * b;
    uint8_t set=0, clr=0;
    *(res ? &clr : &set) |= cc_z;
    *(res&0x0080 ? &set : &clr) |= cc_c;
    return res;
}

static uint8_t _incf8(uint8_t a) {
    uint16_t res = a + 1;
    uint8_t set=0, clr=0;
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    *((~_nsign8(a) & (_nsign8(a) ^ _nsign8(res))) ? &set : &clr) |= cc_v;
    _setclrccf(set, clr);
    return res;
}

static uint8_t _decf8(uint8_t a) {
    uint16_t res = a - 1;
    uint8_t set=0, clr=0;
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    *((_nsign8(a) & (_nsign8(a) ^ _nsign8(res))) ? &set : &clr) |= cc_v;
    _setclrccf(set, clr);
    return res;
}

static uint8_t _logicf8(uint8_t a, uint8_t b, logicop op) {
    uint8_t res;
    uint8_t set=0, clr=cc_v;
    switch (op) {
        case lo_and:
            res = a & b;
            break;
        case lo_or:
            res = a | b;
            break;
        case lo_xor:
            res = a ^ b;
            break;
        case lo_com:
            res = ~a;
            *(_nzero8(res) ? &set : &clr) |= cc_c;
            break;
        default:
            ERROREXIT(sc_invalid_logicop, "Invalid Logic Operation");
    }
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    _setclrccf(set, clr);
    return res;
}

static uint8_t _shiftf8(uint8_t a, shiftmode shmode, uint8_t rot) {
    uint8_t res;
    uint8_t set=0, clr=0;
    uint8_t acarry = a & (shmode?0x01:0x80);
    uint8_t carry = cpu_regs.cc&cc_c;
    res = (shmode?a>>1:a<<1);
    if (shmode == sh_aright) {
        res = (res&0x7f) | (a&0x80);
    }
    else if (rot && carry) {
        res |= (shmode?0x01:0x80);
    }
    *(acarry ? &set : &clr) |= cc_c;
    *(_nsign8(res) ? &set : &clr) |= cc_n;
    *(_nzero8(res) ? &clr : &set) |= cc_z;
    if (!shmode) {
        *((_nsign8(res) ^ _nsign8(a)) ? &set : &clr) |= cc_v;
    }
    return res;
}

static void _pushregs(uint8_t a, uint16_t *st, uint16_t *stus) {
    if (a&rn_pc) {
        mem_wr16((*st)-=2, cpu_regs.pc);
    }
    if (a&rn_us) {
        mem_wr16((*st)-=2, *stus);
    }
    if (a&rn_y) {
        mem_wr16((*st)-=2, cpu_regs.y);
    }
    if (a&rn_x) {
        mem_wr16((*st)-=2, cpu_regs.x);
    }
    if (a&rn_dp) {
        mem_wr8(--(*st), cpu_regs.dp);
    }
    if (a&rn_b) {
        mem_wr8(--(*st), cpu_regs.b);
    }
    if (a&rn_a) {
        mem_wr8(--(*st), cpu_regs.a);
    }
    if (a&rn_cc) {
        mem_wr8(--(*st), cpu_regs.cc);
    }
}

static void _pullregs(uint8_t a, uint16_t *st, uint16_t *stus) {
    if (a&0x01) {
        cpu_regs.cc = mem_rd8((*st)++);
    }
    if (a&0x02) {
        cpu_regs.a = mem_rd8((*st)++);
    }
    if (a&0x04) {
        cpu_regs.b = mem_rd8((*st)++);
    }
    if (a&0x08) {
        cpu_regs.dp = mem_rd8((*st)++);
    }
    if (a&0x10) {
        cpu_regs.x = mem_rd16(*st);
        (*st) += 2;
    }
    if (a&0x20) {
        cpu_regs.y = mem_rd16(*st);
        (*st) += 2;
    }
    if (a&0x40) {
        *stus = mem_rd16(*st);
        (*st) += 2;
    }
    if (a&0x80) {
        cpu_regs.pc = mem_rd16(*st);
        (*st) += 2;
    }
}

static void *_register(uint8_t reg, uint8_t *regsz) {
    *regsz = 2;
    switch(reg) {
        case 0b0000:
            return &cpu_regs.d;
        case 0b0001:
            return &cpu_regs.x;
        case 0b0010:
            return &cpu_regs.y;
        case 0b0011:
            return &cpu_regs.u;
        case 0b0100:
            return &cpu_regs.s;
        case 0b0101:
            return &cpu_regs.pc;
        case 0b1000:
            *regsz = 1;
            return &cpu_regs.a;
        case 0b1001:
            *regsz = 1;
            return &cpu_regs.b;
        case 0b1010:
            *regsz = 1;
            return &cpu_regs.cc;
        case 0b1011:
            *regsz = 1;
            return &cpu_regs.dp;
        default:
            ERROREXIT(sc_invalid_register, "Invalid Register");
    }
    return 0;
}

static void _transfer_sd(void *src, uint8_t srcsz,
                         void *dst, uint8_t dstsz,
                         uint8_t swap)
{
    uint16_t srca;
    uint16_t dsta;

    if (srcsz > 1) srca = *((uint16_t *)src);
    else {
        srca = *((uint8_t *)src);
        srca |= (srca << 8);
    }

    if (dstsz > 1) dsta = *((uint16_t *)dst);
    else {
        dsta = *((uint8_t *)dst);
        dsta |= (dsta << 8);
    }

    if (swap) {
        srca ^= dsta;
        dsta ^= srca;
        srca ^= dsta;
        if (srcsz > 1) *((uint16_t *)src) = srca;
        else *((uint8_t *)src) = srca;
    }
    else dsta = srca;

    if (dstsz > 1) *((uint16_t *)dst) = dsta;
    else *((uint8_t *)dst) = dsta;
}

/* Transfer or exchange register contents */
static void _transfer(uint8_t post, uint8_t swap) {
    uint8_t ddsz;
    uint8_t sssz;
    void *dd = _register(post&0x0f, &ddsz);
    void *ss = _register(post>>4, &sssz);
    _transfer_sd(ss, sssz, dd, ddsz, swap);
}

static void _jump(uint16_t a) {
    cpu_regs.pc = a;
}

static uint16_t _eadirect(void) {
    return (cpu_regs.dp<<8) | mem_rd8(cpu_regs.pc++);
}

/* Offsets
    5-bit = -16 to +15
    8-bit = -128 to +127 (not +128 as in some documents)
   16-bit = -32768 to 32767

   All signed calculations are done in unsigned arithmetic using
   two's complement.
*/
static uint16_t _eaindexed(void) {
    uint16_t ea = EYECATCHER_16;
    uint8_t post = mem_rd8(cpu_regs.pc++);
    uint16_t *preg;
    /* Determine register from bits xRRxxxxx*/
    switch ((post>>5)&0x03) {
        case 0: preg = &cpu_regs.x; break;
        case 1: preg = &cpu_regs.y; break;
        case 2: preg = &cpu_regs.u; break;
        case 3: preg = &cpu_regs.s; break;
    }
    /* determine effective address from bits xxxIAAAA*/
    uint8_t ind = post&0x10;
    uint16_t pcofs;
    switch (post&0x8f) {
        case 0x00 ... 0x1f: /* ,R+5bit offset */
            ea = *preg + _sext816(_sext58(post));
            ind = 0;
            break;
        case 0x80: /* ,R+ */
            ea = *preg;
            *preg += 1;
            break;
        case 0x81: /* ,R++ */
            ea = *preg;
            *preg += 2;
            break;
        case 0x82: /* ,-R */
            *preg -= 1;
            ea = *preg;
            break;
        case 0x83: /* ,--R */
            *preg -= 2;
            ea = *preg;
            break;
        case 0x84: /* ,R+0 offset */
            ea = *preg;
            break;
        case 0x85: /* ,R+ACCB offset */
            ea = *preg + _sext816(cpu_regs.b);
            break;
        case 0x86: /* ,R+ACCA offset */
            ea = *preg + _sext816(cpu_regs.a);
            break;
        case 0x88: /* ,R+8bit offset */
            ea = *preg + _sext816(mem_rd8(cpu_regs.pc++));
            break;
        case 0x89: /* ,R+16bit offset */
            ea = *preg + mem_rd16(cpu_regs.pc++);
            break;
        case 0x8b: /* ,R+D offset */
            ea = *preg + cpu_regs.d;
            break;
        case 0x8c: /* ,PC+8bit offset */
            pcofs = _sext816(mem_rd8(cpu_regs.pc++));
            ea = cpu_regs.pc + pcofs;
            break;
        case 0x8d: /* ,PC+16bit offset */
            pcofs = mem_rd16(cpu_regs.pc);
            cpu_regs.pc += 2;
            ea = cpu_regs.pc + pcofs;
            break;
        case 0x8f: /* ,Address */
            ea = mem_rd16(cpu_regs.pc);
            cpu_regs.pc += 2;
            break;
        default:
            cpu_status.code = sc_invalid_postcode;
    }
    if (ind && cpu_status.code == sc_ok) {
        ea = mem_rd16(ea);
    }
    return ea;
}

static uint16_t _earelative(uint8_t nbyte) {
    uint16_t ofs = ((nbyte>1)?mem_rd16(cpu_regs.pc):_sext816(mem_rd8(cpu_regs.pc)));
    cpu_regs.pc += nbyte;
    return cpu_regs.pc + ofs;
}

static uint16_t _eaextended(void) {
    uint16_t ea = mem_rd16(cpu_regs.pc);
    cpu_regs.pc += 2;
    return ea;
}

static uint16_t _eaimmediate(uint8_t nbyte) {
    uint16_t ea = cpu_regs.pc;
    cpu_regs.pc += nbyte;
    return ea;
}

/* Determine effective address from the opcode. */
static uint16_t _eafromopc(uint8_t opc) {
    switch (opc>>4) {
        case 0x00:
            return _eadirect();
        case 0x01:
            switch (opc&0x0f) {
                case 0x02: /* inherent */
                case 0x03:
                case 0x09:
                case 0x0d:
                    return EYECATCHER_16;
                case 0x06:
                case 0x07:
                    return _earelative(2);
                default:
                    return _eaimmediate(1);
            }
        case 0x02:
            return _earelative(1);
        case 0x03:
            switch (opc&0x0f) {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                    return _eaindexed();
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    return _eaimmediate(1);
                default: /* Inherent */
                    return EYECATCHER_16;
            }
        case 0x04: /* inherent */
        case 0x05:
            return EYECATCHER_16;
        case 0x06:
        case 0x0a:
        case 0x0e:
            return _eaindexed();
        case 0x07:
        case 0x0b:
        case 0x0f:
            return _eaextended();
        case 0x08:
        case 0x0c:
            switch (opc&0x0f) {
                case 0x03:
                case 0x0c:
                case 0x0e:
                    return _eaimmediate(2);
                default:
                    return _eaimmediate(1);
            }
        case 0x09:
        case 0x0d:
            return _eadirect();
        default:
            ERROREXIT(sc_undefined_adressing_mode, "Undefined Adressing Mode");
            break;
    }
#ifdef obsolete
    switch (opc) {
        case 0x00 ... 0x0f:
        case 0x90 ... 0x9f:
        case 0xd0 ... 0xdf:
            ea = _eadirect();
            break;
        case 0x16 ... 0x17:
            ea = _earelative(2);
            break;
        case 0x20 ... 0x2f:
            ea = _earelative(1);
            break;
        case 0x30 ... 0x33:
        case 0x60 ... 0x6f:
        case 0xa0 ... 0xaf:
        case 0xe0 ... 0xef:
            ea = _eaindexed();
            break;
        case 0x70 ... 0x7f:
        case 0xb0 ... 0xbf:
        case 0xf0 ... 0xff:
            ea = _eaextended();
            break;
        case 0x1a:
        case 0x1c:
        case 0x1e:
        case 0x1f:
        case 0x34 ... 0x37:
        case 0x80 ... 0x82:
        case 0x84 ... 0x8b:
        case 0x8d:
        case 0xc0 ... 0xc2:
        case 0xc4 ... 0xc6:
        case 0xc8 ... 0xcb:
            ea = _eaimmediate(1);
            break;
        case 0x83:
        case 0x8c:
        case 0x8e:
        case 0xc3:
        case 0xcc:
        case 0xce:
            ea = _eaimmediate(2);
            break;

        case 0x12: /* inherent */
        case 0x13:
        case 0x19:
        case 0x1d:
        case 0x39 ... 0x3d:
        case 0x3f:
        case 0x40:
        case 0x43:
        case 0x44:
        case 0x46 ... 0x4a:
        case 0x4c:
        case 0x4d:
        case 0x4f:
        case 0x50:
        case 0x53:
        case 0x54:
        case 0x56 ... 0x5a:
        case 0x5c:
        case 0x5d:
        case 0x5f:
            break;

        default:
            ERROREXIT(sc_undefined_adressing_mode, "Undefined Adressing Mode");
            break;
    }
#endif
    return EYECATCHER_16;
}

static void _pushfast() {
    _setclrccf(0, cc_e);
    _pushregs(rn_fast, &cpu_regs.s, 0);
}

static void _pushstate() {
    _setclrccf(cc_e, 0);
    _pushregs(rn_entire, &cpu_regs.s, &cpu_regs.u);
}

static void _pullstate() {
    _pullregs(cpu_regs.cc&cc_e?rn_entire:rn_fast
              , &cpu_regs.s, &cpu_regs.u);
}

static void _execop_page2(void) {
    uint8_t opc = mem_rd8(cpu_regs.pc++);
    uint16_t ea = _eafromopc(opc);
    uint16_t *preg16 = NULL;
    uint16_t val16;
    uint8_t val8;
    switch (opc) {
        case op2_lds_d:
        case op2_lds_i:
        case op2_lds_n:
        case op2_lds_x:
            preg16 = &cpu_regs.s;
            goto p2_loadreg16;
        case op2_ldy_d:
        case op2_ldy_i:
        case op2_ldy_n:
        case op2_ldy_x:
            preg16 = &cpu_regs.y;
        p2_loadreg16:
            *preg16 = mem_rd16(ea);
            _flags_nz_v16(*preg16);
            break;

        case op2_cmpd_d:
        case op2_cmpd_i:
        case op2_cmpd_n:
        case op2_cmpd_x:
            preg16 = &cpu_regs.d;
            goto p2_cmpreg16;
        case op2_cmpy_d:
        case op2_cmpy_i:
        case op2_cmpy_n:
        case op2_cmpy_x:
            preg16 = &cpu_regs.y;
        p2_cmpreg16:
            _subf16(*preg16, mem_rd16(ea));
            break;

        case op2_sts_d:
        case op2_sts_n:
        case op2_sts_x:
            val16 = cpu_regs.s;
            goto p2_storereg16;
        case op2_sty_d:
        case op2_sty_n:
        case op2_sty_x:
            val16 = cpu_regs.y;
        p2_storereg16:
            mem_wr16(ea, val16);
            _flags_nz_v16(val16);
            break;

        case op2_lbcc_r:
            val8 = !(cpu_regs.cc&cc_c);
            goto p2_branch;
        case op2_lbcs_r:
            val8 = cpu_regs.cc&cc_c;
            goto p2_branch;
        case op2_lbeq_r:
            val8 = cpu_regs.cc&cc_z;
            goto p2_branch;
        case op2_lbge_r:
            val8 = _sameccf(cc_n|cc_v);
            goto p2_branch;
        case op2_lbgt_r:
            val8 = _sameccf(cc_n|cc_v) && !(cpu_regs.cc&cc_z);
            goto p2_branch;
        case op2_lbhi_r:
            val8 = !_orccf(cc_z|cc_c);
            goto p2_branch;
        /*case op2_lbhs_r: bcc*/
        case op2_lble_r:
            val8 = !_sameccf(cc_n|cc_v) || (cpu_regs.cc&cc_z);
            goto p2_branch;
        /*case op2_lblo_r: bcs*/
        case op2_lbls_r:
            val8 = _orccf(cc_c|cc_z);
            goto p2_branch;
        case op2_lblt_r:
            val8 = !_sameccf(cc_n|cc_v);
            goto p2_branch;
        case op2_lbmi_r:
            val8 = cpu_regs.cc&cc_n;
            goto p2_branch;
        case op2_lbne_r:
            val8 = !(cpu_regs.cc&cc_z);
            goto p2_branch;
        case op2_lbpl_r:
            val8 = !(cpu_regs.cc&cc_n);
            goto p2_branch;
        case op2_lbrn_r:
            break;
        case op2_lbvc_r:
            val8 = !(cpu_regs.cc&cc_v);
            goto p2_branch;
        case op2_lbvs_r:
            val8 = cpu_regs.cc&cc_v;
        p2_branch:
            if (val8) _jump(ea);
            break;

        case op2_swi2:
            cpu_regs._irq |= irq_swi2;
            break;

        default:
            ERROREXIT(sc_unsupported_page2_opcode, "Invalid Page 2 Opcode");
    }
}

static void _execop_page3(void) {
    uint8_t opc = mem_rd8(cpu_regs.pc++);
    uint16_t ea = _eafromopc(opc);
    uint16_t *preg16 = NULL;
    switch (opc) {
        case op3_cmps_d:
        case op3_cmps_i:
        case op3_cmps_n:
        case op3_cmps_x:
            preg16 = &cpu_regs.s;
            goto p3_cmpreg16;
        case op3_cmpu_d:
        case op3_cmpu_i:
        case op3_cmpu_n:
        case op3_cmpu_x:
            preg16 = &cpu_regs.u;
        p3_cmpreg16:
            _subf16(*preg16, mem_rd16(ea));
            break;

        case op3_swi3:
            cpu_regs._irq |= irq_swi3;
            break;

        default:
            ERROREXIT(sc_unsupported_page3_opcode, "Invalid Page 3 Opcode");
    }
}

void cpu_execop(void) {
    uint8_t opc = mem_rd8(cpu_regs.pc++);
    uint16_t ea = EYECATCHER_16;
    uint16_t *preg16 = NULL;
    uint8_t *preg8 = NULL;
    uint16_t val16;
    uint8_t val8;
    if (opc != op_page2 && opc != op_page3) {
        ea = _eafromopc(opc);
    }
    switch (opc) {
        case op_page2:
            _execop_page2();
            break;

        case op_page3:
            _execop_page3();
            break;

        case op_neg_d:
        case op_neg_n:
        case op_neg_x:
            mem_wr8(ea, _subf8(0, mem_rd8(ea)));
            break;

        case op_nega:
            cpu_regs.a = _subf8(0, cpu_regs.a);
            break;
        case op_negb:
            cpu_regs.b = _subf8(0, cpu_regs.b);
            break;

        case op_com_d:
        case op_com_n:
        case op_com_x:
            mem_wr8(ea, _logicf8(mem_rd8(ea), 0, lo_com));
            break;

        case op_coma:
            cpu_regs.a = _logicf8(cpu_regs.a, 0, lo_com);
            break;
        case op_comb:
            cpu_regs.b = _logicf8(cpu_regs.b, 0, lo_com);
            break;

        case op_bita_d:
        case op_bita_i:
        case op_bita_n:
        case op_bita_x:
            preg8 = &cpu_regs.a;
            goto bitreg8;
        case op_bitb_d:
        case op_bitb_i:
        case op_bitb_n:
        case op_bitb_x:
            preg8 = &cpu_regs.b;
        bitreg8:
            (void)_logicf8(*preg8, mem_rd8(ea), lo_and);
            break;

        case op_abx:
            cpu_regs.x += cpu_regs.a;
            break;

        case op_andcc_i:
            preg8 = &cpu_regs.cc;
            goto andreg8;
        case op_anda_i:
        case op_anda_d:
        case op_anda_n:
        case op_anda_x:
            preg8 = &cpu_regs.a;
            goto andreg8;
        case op_andb_i:
        case op_andb_d:
        case op_andb_n:
        case op_andb_x:
            preg8 = &cpu_regs.b;
        andreg8:
            *preg8 = _logicf8(*preg8, mem_rd8(ea), lo_and);
            break;

        case op_orcc_i:
            preg8 = &cpu_regs.cc;
            goto orreg8;
        case op_ora_i:
        case op_ora_d:
        case op_ora_n:
        case op_ora_x:
            preg8 = &cpu_regs.a;
            goto orreg8;
        case op_orb_i:
        case op_orb_d:
        case op_orb_n:
        case op_orb_x:
            preg8 = &cpu_regs.b;
        orreg8:
            *preg8 = _logicf8(*preg8, mem_rd8(ea), lo_or);
            break;

        case op_eora_i:
        case op_eora_d:
        case op_eora_n:
        case op_eora_x:
            preg8 = &cpu_regs.a;
            goto eorreg8;
        case op_eorb_i:
        case op_eorb_d:
        case op_eorb_n:
        case op_eorb_x:
            preg8 = &cpu_regs.b;
        eorreg8:
            *preg8 = _logicf8(*preg8, mem_rd8(ea), lo_xor);
            break;

        case op_tst_d:
            break;

        case op_inc_d:
        case op_inc_n:
        case op_inc_x:
            mem_wr8(ea, _incf8(mem_rd8(ea)));
            break;

        case op_inca:
            cpu_regs.a = _incf8(cpu_regs.a);
            break;
        case op_incb:
            cpu_regs.b = _incf8(cpu_regs.b);
            break;

        case op_dec_d:
        case op_dec_n:
        case op_dec_x:
            mem_wr8(ea, _decf8(mem_rd8(ea)));
            break;

        case op_deca:
            cpu_regs.a = _decf8(cpu_regs.a);
            break;
        case op_decb:
            cpu_regs.b = _decf8(cpu_regs.b);
            break;

        case op_jmp_d:
        case op_jmp_n:
        case op_jmp_x:
            _jump(ea);
            break;

        case op_clra:
            cpu_regs.a = 0;
            goto setclrccfclr;
        case op_clrb:
            cpu_regs.b = 0;
        setclrccfclr:
            _setclrccf(cc_z, cc_n|cc_v|cc_c);
            break;

        case op_clr_d:
        case op_clr_n:
        case op_clr_x:
            mem_wr8(ea, 0);
            _setclrccf(cc_z, cc_n|cc_v|cc_c);
            break;

        case op_nop:
            break;

        case op_lsla:
            preg8 = &cpu_regs.a;
            goto lshiftreg8;
        case op_lslb:
            preg8 = &cpu_regs.b;
        lshiftreg8:
            *preg8 = _shiftf8(*preg8, sh_left, 0);
            break;

        case op_lsra:
            preg8 = &cpu_regs.a;
            goto rshiftreg8;
        case op_lsrb:
            preg8 = &cpu_regs.b;
        rshiftreg8:
            *preg8 = _shiftf8(*preg8, sh_right, 0);
            break;

        case op_asra:
            preg8 = &cpu_regs.a;
            goto arshiftreg8;
        case op_asrb:
            preg8 = &cpu_regs.b;
        arshiftreg8:
            *preg8 = _shiftf8(*preg8, sh_aright, 0);
            break;

        case op_lsl_d:
        case op_lsl_n:
        case op_lsl_x:
            mem_wr8(ea, _shiftf8(mem_rd8(ea), sh_left, 0));
            break;

        case op_lsr_d:
        case op_lsr_n:
        case op_lsr_x:
            mem_wr8(ea, _shiftf8(mem_rd8(ea), sh_right, 0));
            break;

        case op_asr_d:
        case op_asr_n:
        case op_asr_x:
            mem_wr8(ea, _shiftf8(mem_rd8(ea), sh_aright, 0));
            break;

        case op_rola:
            preg8 = &cpu_regs.a;
            goto lrotreg8;
        case op_rolb:
            preg8 = &cpu_regs.b;
        lrotreg8:
            *preg8 = _shiftf8(*preg8, sh_left, 1);
            break;

        case op_rora:
            preg8 = &cpu_regs.a;
            goto rrotreg8;
        case op_rorb:
            preg8 = &cpu_regs.b;
        rrotreg8:
            *preg8 = _shiftf8(*preg8, sh_right, 1);
            break;

        case op_lda_i:
        case op_lda_d:
        case op_lda_n:
        case op_lda_x:
            preg8 = &cpu_regs.a;
            goto loadreg8;
        case op_ldb_i:
        case op_ldb_d:
        case op_ldb_n:
        case op_ldb_x:
            preg8 = &cpu_regs.b;
        loadreg8:
            *preg8 = mem_rd8(ea);
            _flags_nz_v8(*preg8);
            break;

        case op_ldd_i:
        case op_ldd_d:
        case op_ldd_n:
        case op_ldd_x:
            preg16 = &cpu_regs.d;
            goto loadreg16;
        case op_ldx_i:
        case op_ldx_d:
        case op_ldx_n:
        case op_ldx_x:
            preg16 = &cpu_regs.x;
            goto loadreg16;
        case op_ldu_i:
        case op_ldu_d:
        case op_ldu_n:
        case op_ldu_x:
            preg16 = &cpu_regs.u;
        loadreg16:
            *preg16 = mem_rd16(ea);
            _flags_nz_v16(*preg16);
            break;

        case op_adda_d:
        case op_adda_i:
        case op_adda_n:
        case op_adda_x:
            preg8 = &cpu_regs.a;
            goto addreg8;
        case op_addb_d:
        case op_addb_i:
        case op_addb_n:
        case op_addb_x:
            preg8 = &cpu_regs.b;
        addreg8:
            *preg8 = _addf8(*preg8, mem_rd8(ea));
            break;

        case op_adca_d:
        case op_adca_i:
        case op_adca_n:
        case op_adca_x:
            preg8 = &cpu_regs.a;
            goto adcreg8;
        case op_adcb_d:
        case op_adcb_i:
        case op_adcb_n:
        case op_adcb_x:
            preg8 = &cpu_regs.b;
        adcreg8:
            *preg8 = _addcf8(*preg8, mem_rd8(ea));
            break;

        case op_suba_d:
        case op_suba_i:
        case op_suba_n:
        case op_suba_x:
            preg8 = &cpu_regs.a;
            goto subreg8;
        case op_subb_d:
        case op_subb_i:
        case op_subb_n:
        case op_subb_x:
            preg8 = &cpu_regs.b;
        subreg8:
            *preg8 = _subf8(*preg8, mem_rd8(ea));
            break;

        case op_sbca_d:
        case op_sbca_i:
        case op_sbca_n:
        case op_sbca_x:
            preg8 = &cpu_regs.a;
            goto sbcreg8;
        case op_sbcb_d:
        case op_sbcb_i:
        case op_sbcb_n:
        case op_sbcb_x:
            preg8 = &cpu_regs.b;
        sbcreg8:
            *preg8 = _subcf8(*preg8, mem_rd8(ea));
            break;

        case op_addd_d:
        case op_addd_i:
        case op_addd_n:
        case op_addd_x:
            cpu_regs.d = _addf16(cpu_regs.d, mem_rd16(ea));
            break;

        case op_subd_d:
        case op_subd_i:
        case op_subd_n:
        case op_subd_x:
            cpu_regs.d = _subf16(cpu_regs.d, mem_rd16(ea));
            break;

        case op_cmpa_d:
        case op_cmpa_i:
        case op_cmpa_n:
        case op_cmpa_x:
            preg8 = &cpu_regs.a;
            goto cmpreg8;
        case op_cmpb_d:
        case op_cmpb_i:
        case op_cmpb_n:
        case op_cmpb_x:
            preg8 = &cpu_regs.b;
        cmpreg8:
            _subf8(*preg8, mem_rd8(ea));
            break;

        case op_cmpx_d:
        case op_cmpx_i:
        case op_cmpx_n:
        case op_cmpx_x:
            _subf16(cpu_regs.x, mem_rd16(ea));
            break;

        case op_exg_i:
            _transfer(mem_rd8(ea), 1);
            break;
        case op_tfr_i:
            _transfer(mem_rd8(ea), 0);
            break;

        case op_leas_n:
            preg16 = &cpu_regs.s;
            goto leareg16;
        case op_leau_n:
            preg16 = &cpu_regs.u;
        leareg16:
            *preg16 = ea;
            break;

        case op_leax_n:
            preg16 = &cpu_regs.x;
            goto learegf16;
        case op_leay_n:
            preg16 = &cpu_regs.y;
        learegf16:
            *preg16 = ea;
            _flags_z16(*preg16);
            break;

        case op_mul:
            cpu_regs.d = _mulf8(cpu_regs.a, cpu_regs.b);
            break;

        case op_pshs_i:
            _pushregs(mem_rd8(ea), &cpu_regs.s, &cpu_regs.u);
            break;
        case op_pshu_i:
            _pushregs(mem_rd8(ea), &cpu_regs.u, &cpu_regs.s);
            break;

        case op_puls_i:
            _pullregs(mem_rd8(ea), &cpu_regs.s, &cpu_regs.u);
            break;
        case op_pulu_i:
            _pullregs(mem_rd8(ea), &cpu_regs.u, &cpu_regs.s);
            break;

        case op_jsr_d:
        case op_jsr_n:
        case op_jsr_x:
            cpu_regs.s -= 2;
            mem_wr16(cpu_regs.s, cpu_regs.pc);
            _jump(ea);
            break;

        case op_rts:
            _jump(mem_rd16(cpu_regs.s));
            cpu_regs.s += 2;
            break;

        case op_sta_d:
        case op_sta_n:
        case op_sta_x:
            val8 = cpu_regs.a;
            goto storereg8;
        case op_stb_d:
        case op_stb_n:
        case op_stb_x:
            val8 = cpu_regs.b;
        storereg8:
            mem_wr8(ea, val8);
            _flags_nz_v8(val8);
            break;

        case op_std_d:
        case op_std_n:
        case op_std_x:
            val16 = cpu_regs.d;
            goto storereg16;
        case op_stu_d:
        case op_stu_n:
        case op_stu_x:
            val16 = cpu_regs.u;
            goto storereg16;
        case op_stx_d:
        case op_stx_n:
        case op_stx_x:
            val16 = cpu_regs.x;
        storereg16:
            mem_wr16(ea, val16);
            _flags_nz_v16(val16);
            break;

        case op_sex:
            cpu_regs.d = _sext816(cpu_regs.a);
            _flags_nz_v16(cpu_regs.d);
            break;

        case op_daa:
            _decadja();
            break;

        case op_bcc_r:
            val8 = !(cpu_regs.cc&cc_c);
            goto branch;
        case op_bcs_r:
            val8 = cpu_regs.cc&cc_c;
            goto branch;
        case op_beq_r:
            val8 = cpu_regs.cc&cc_z;
            goto branch;
        case op_bge_r:
            val8 = _sameccf(cc_n|cc_v);
            goto branch;
        case op_bgt_r:
            val8 = _sameccf(cc_n|cc_v) && !(cpu_regs.cc&cc_z);
            goto branch;
        case op_bhi_r:
            val8 = !_orccf(cc_z|cc_c);
            goto branch;
        /*case op_bhs_r: bcc*/
        case op_ble_r:
            val8 = !_sameccf(cc_n|cc_v) || (cpu_regs.cc&cc_z);
            goto branch;
        /* case op_blo_r: bcs */
        case op_bls_r:
            val8 = _orccf(cc_c|cc_z);
            goto branch;
        case op_blt_r:
            val8 = !_sameccf(cc_n|cc_v);
            goto branch;
        case op_bmi_r:
            val8 = cpu_regs.cc&cc_n;
            goto branch;
        case op_bne_r:
            val8 = !(cpu_regs.cc&cc_z);
            goto branch;
        case op_bpl_r:
            val8 = !(cpu_regs.cc&cc_n);
            goto branch;
        case op_bra_r:
        case op_lbra_r:
            val8 = 1;
            goto branch;
        case op_brn_r:
            break;
        case op_bsr_r:
        case op_lbsr_r:
            cpu_regs.s -= 2;
            mem_wr16(cpu_regs.s, cpu_regs.pc);
            _jump(ea);
            break;
        case op_bvc_r:
            val8 = !(cpu_regs.cc&cc_v);
            goto branch;
        case op_bvs_r:
            val8 = cpu_regs.cc&cc_v;
        branch:
            if (val8) _jump(ea);
            break;

        case op_rti:
            _pullstate();
            break;

        case op_swi:
            cpu_regs._irq |= irq_swi;
            break;

        case 0x01:
        case 0x02:
        case 0x05:
        case 0x0b:
        case 0x14:
        case 0x15:
        case 0x18:
        case 0x1b:
        case 0x38:
        case 0x3e:
        case 0x41:
        case 0x42:
        case 0x45:
        case 0x4b:
        case 0x4e:
        case 0x51:
        case 0x52:
        case 0x55:
        case 0x5b:
        case 0x5e:
        case 0x61:
        case 0x62:
        case 0x65:
        case 0x6b:
        case 0x71:
        case 0x72:
        case 0x75:
        case 0x7b:
        case 0x87:
        case 0x8f:
        case 0xc7:
        case 0xcd:
        case 0xcf:
            ERROREXIT(sc_invalid_opcode, "Invalid Opcode");
            break;

        default:
            ERROREXIT(sc_unsupported_opcode, "Unsupported Opcode");
    }
}

void cpu_execirq(void) {
    if (cpu_regs._irq&irq_rst) {
        cpu_regs.dp = 0;
        cpu_regs._irq = 0;
        cpu_regs.pc = mem_rd16(0xfffe);
        return;
    }
    if (cpu_regs._irq&irq_nmi) {
        _setclrirqf(0, irq_nmi);
        _pushstate();
        cpu_regs.pc = mem_rd16(0xfffc);
        return;
    }
    if ((cpu_regs._irq&irq_firq) && !(cpu_regs.cc&cc_f)) {
        _setclrirqf(0, irq_firq);
        _pushfast();
        cpu_regs.pc = mem_rd16(0xfff6);
        return;
    }
    if ((cpu_regs._irq&irq_irq) && !(cpu_regs.cc&cc_i)) {
        _setclrirqf(0, irq_irq);
        _pushstate();
        cpu_regs.pc = mem_rd16(0xfff8);
        return;
    }
    if (cpu_regs._irq&irq_swi) {
        _setclrirqf(0, irq_swi);
        _pushstate();
        cpu_regs.pc = mem_rd16(0xfffa);
        return;
    }
    if (cpu_regs._irq&irq_swi2) {
        _setclrirqf(0, irq_swi2);
        _pushstate();
        cpu_regs.pc = mem_rd16(0xfff4);
        return;
    }
    if (cpu_regs._irq&irq_swi3) {
        _setclrirqf(0, irq_swi3);
        _pushstate();
        cpu_regs.pc = mem_rd16(0xfff2);
        return;
    }
}

void _irq(uint8_t cpu_irq, uint8_t set) {
    _setclrirqf(set?cpu_irq:0, set?0:cpu_irq);
}

void cpu_reset(uint8_t set) {
    _irq(irq_rst, set);
}

void cpu_irq(uint8_t set) {
    _irq(irq_irq, set);
}

void cpu_firq(uint8_t set) {
    _irq(irq_firq, set);
}

void cpu_nmi(uint8_t set) {
    _irq(irq_nmi, set);
}
