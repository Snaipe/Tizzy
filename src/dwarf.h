/*
 * The MIT License (MIT)
 *
 * Copyright © 2016 Franklin "Snaipe" Mathieu <http://snai.pe/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef DWARF_H_
# define DWARF_H_

# include "addr2line.h"

# ifdef __GNUC__
#  define PACK(Struct) Struct __attribute__((__packed__))
# elif _MSC_VER
#  define PACK(Struct) __pragma(pack(push, 1)) Struct __pragma(pack(pop))
# else
#  error Compiler not supported: no packing attribute
# endif

typedef uint8_t     dwarf_ubyte;
typedef int8_t      dwarf_sbyte;
typedef uint16_t    dwarf_uhalf;
typedef uint32_t    dwarf_uword;
typedef int32_t     dwarf_sword;

typedef int         dwarf_bool;
typedef uintptr_t   dwarf_addr;

struct dwarf_regs {
    dwarf_addr addr;
    dwarf_uword file;
    dwarf_uword line;
    dwarf_uword column;
    dwarf_uword isa;
    dwarf_bool is_stmt          : 1;
    dwarf_bool basic_block      : 1;
    dwarf_bool end_sequence     : 1;
    dwarf_bool prologue_end     : 1;
    dwarf_bool epilogue_begin   : 1;
};

PACK(struct dwarf_debug_line_hdr {
    dwarf_uword length;
    dwarf_uhalf version;
    dwarf_uword header_length;
    dwarf_ubyte min_instruction_length;
    dwarf_ubyte default_is_stmt;
    dwarf_sbyte line_base;
    dwarf_ubyte line_range;
    dwarf_ubyte opcode_base;
    dwarf_ubyte opcode_lengths[];
});

enum dwarf_lns_opcodes {
    DW_LNS_copy = 1,
    DW_LNS_advance_pc = 2,
    DW_LNS_advance_line = 3,
    DW_LNS_set_file = 4,
    DW_LNS_set_column = 5,
    DW_LNS_negate_stmt = 6,
    DW_LNS_set_basic_block = 7,
    DW_LNS_const_add_pc = 8,
    DW_LNS_fixed_advance_pc = 9,
    DW_LNS_set_prologue_end = 10,
    DW_LNS_set_epilogue_begin = 11,
    DW_LNS_set_isa = 12
};

enum dwarf_lne_opcodes {
    DW_LNE_end_sequence = 1,
    DW_LNE_set_address = 2,
    DW_LNE_define_file = 3,
    DW_LNE_set_discriminator = 4,
};

int dwarf_translate(struct addr2line_entry *ln, const void *section,
        size_t section_len);

#endif /* !DWARF_H_ */
