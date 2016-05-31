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
#include "section.h"
#include "dwarf.h"

#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "addr2line.h"

# define MAX_LINE_SIZE 128

dwarf_uword dwarf_decode_uleb128(const uint8_t **b) {
    dwarf_uword result = 0;
    size_t shift = 0;
    for (;;) {
        uint8_t byte = **b;
        ++*b;
        result |= (byte & 0x7f) << shift;
        if (!(byte & 0x80))
            break;
        shift += 7;
    }
    return result;
}

dwarf_sword dwarf_decode_sleb128(const uint8_t **b) {
    dwarf_sword result = 0;
    size_t shift = 0;
    dwarf_bool sign;
    for (;;) {
        uint8_t byte = **b;
        ++*b;
        sign = byte & 0x40;
        result |= (byte & 0x7f) << shift;
        shift += 7;
        if (!(byte & 0x80))
            break;
    }

    if (shift < sizeof (dwarf_sword) * 8 && sign)
        result |= -((dwarf_sword) 1 << shift);

    return result;
}

static inline void *dwarf_skip_entry(const void *ptr, size_t level)
{
    const char *b;
    if (level == 0) {
        for (b = ptr; *b; ++b);
    } else {
        for (b = ptr; *b; b = dwarf_skip_entry(b, level - 1));
    }
    return (void *) (b + 1);
}

static char *dwarf_get_nth_dirname(dwarf_uword dir_idx,
        const uint8_t *include_dirs)
{
    static char empty[] = "";
    if (!dir_idx)
        return empty;
    for (size_t i = dir_idx - 1; i; --i) {
        include_dirs = dwarf_skip_entry(include_dirs, 0);
        if (!*include_dirs)
            return empty;
    }
    return (char *) include_dirs;
}

static int dwarf_fill_filename(int file_idx,
        const uint8_t *include_directories, const uint8_t *filenames,
        const char **out_dir, const char **out_filename)
{
    for (int i = 1; i < file_idx; ++i) {
        if (!*filenames)
            return 0;

        filenames = dwarf_skip_entry(filenames, 0);
        (void) dwarf_decode_uleb128(&filenames);
        (void) dwarf_decode_uleb128(&filenames);
        (void) dwarf_decode_uleb128(&filenames);
    }

    char *filename = (char *) filenames;
    if (!*filename)
        return 0;

    filenames = dwarf_skip_entry(filenames, 0);
    dwarf_uword dir_idx = dwarf_decode_uleb128(&filenames);

    if (out_filename)
        *out_filename = filename;
    if (out_dir)
        *out_dir = dwarf_get_nth_dirname(dir_idx, include_directories);

    return 1;
}

static void dwarf_match_addr(dwarf_addr src_addr,
        const uint8_t *include_directories, const uint8_t *filenames,
        struct addr2line_entry *ln, struct dwarf_regs *regs)
{
    regs->basic_block = 0;
    regs->prologue_end = 0;
    regs->epilogue_begin = 0;
    if (src_addr > regs->addr && src_addr < regs->addr + MAX_LINE_SIZE)
    {
        const char *filename = NULL, *dirname = NULL;
        dwarf_fill_filename(regs->file, include_directories, filenames,
                &dirname, &filename);

        ln->line = regs->line;
        ln->column = regs->column;
        ln->filename = strdup(filename);
        ln->dirname = strdup(dirname);
    }
}

# define offset_after(Field) ((uint8_t *) &(Field) + sizeof (Field) + (Field))

static int dwarf_translate_cu(dwarf_addr src_addr,
        const uint8_t **section, struct addr2line_entry *ln, jmp_buf abort)
{
    struct dwarf_debug_line_hdr *hdr = (void *) *section;

    const uint8_t *start = offset_after(hdr->header_length);
    const uint8_t *end = offset_after(hdr->length);

    const uint8_t *buf = (uint8_t*) (hdr + 1) + hdr->opcode_base - 1;

    const uint8_t *include_directories = buf;

    /* skip include directories */
    buf = dwarf_skip_entry(buf, 1);

    const uint8_t *filenames = buf;

    buf = dwarf_skip_entry(buf, 1);

    if (hdr->version != 2)
        return -EINVAL;

    buf = start;
    *section = end;

    /* Dwarf registers */
    struct dwarf_regs regs = {
        .file = 1,
        .line = 1,
        .is_stmt = hdr->default_is_stmt,
    };

    int extended = 0;

    while (buf < end) {
        if (extended) {
            dwarf_uword op_len = dwarf_decode_uleb128(&buf);
            const uint8_t *ext_buf = buf;

            /* Ignore empty extended opcodes */
            if (op_len == 0) {
                extended = 0;
                continue;
            }
            uint8_t op = *ext_buf++;
            switch (op) {
                case DW_LNE_end_sequence:
                    regs.end_sequence = 1;
                    dwarf_match_addr(src_addr, include_directories,
                                filenames, ln, &regs);
                    regs = (struct dwarf_regs) {
                        .file = 1,
                        .line = 1,
                        .is_stmt = hdr->default_is_stmt,
                    };
                    break;
                case DW_LNE_set_address:
                    regs.addr = *(unsigned long *)ext_buf;
                    break;
                case DW_LNE_define_file:
                    /* Unimplemented */
                    for (; *ext_buf; ++ext_buf); // consume filename
                    (void) dwarf_decode_uleb128(&ext_buf);
                    (void) dwarf_decode_uleb128(&ext_buf);
                    (void) dwarf_decode_uleb128(&ext_buf);
                    break;
                case DW_LNE_set_discriminator: /* Ignore */
                    (void) dwarf_decode_uleb128(&ext_buf);
                    break;
                default: /* Unknown opcode; can't deal with it */
                    longjmp(abort, 1);
                    break;
            }
            extended = 0;
            buf += op_len;
        } else {
            uint8_t op = *buf++;
            switch (op) {
                case DW_LNS_copy:
                    dwarf_match_addr(src_addr, include_directories,
                                filenames, ln, &regs);
                    break;
                case DW_LNS_advance_pc:
                    regs.addr += dwarf_decode_uleb128(&buf)
                        * hdr->min_instruction_length;
                    break;
                case DW_LNS_advance_line:
                    regs.line += dwarf_decode_sleb128(&buf);
                    break;
                case DW_LNS_set_file:
                    regs.file = dwarf_decode_uleb128(&buf);
                    break;
                case DW_LNS_set_column:
                    regs.column = dwarf_decode_uleb128(&buf);
                    break;
                case DW_LNS_negate_stmt:
                    regs.is_stmt = !regs.is_stmt;
                    break;
                case DW_LNS_set_basic_block:
                    regs.basic_block = 1;
                    break;
                case DW_LNS_const_add_pc:
                    regs.addr += ((0xff - hdr->opcode_base) / hdr->line_range)
                        * hdr->min_instruction_length;
                    break;
                case DW_LNS_fixed_advance_pc:
                    regs.addr += *(uint16_t *) buf;
                    buf += sizeof (uint16_t);
                    break;
                case DW_LNS_set_prologue_end:
                    regs.prologue_end = 1;
                    break;
                case DW_LNS_set_epilogue_begin:
                    regs.epilogue_begin = 1;
                    break;
                case DW_LNS_set_isa:
                    regs.isa = (unsigned int) dwarf_decode_uleb128(&buf);
                    break;
                case 0:
                    extended = 1;
                    break;
                /* Special opcodes */
                default: {
                    dwarf_uword incr = op - hdr->opcode_base;
                    dwarf_uword addr_incr = (incr / hdr->line_range)
                        * hdr->min_instruction_length;
                    dwarf_uword line_incr = hdr->line_base
                        + (incr % hdr->line_range);
                    regs.addr += addr_incr;
                    regs.line += line_incr;

                    dwarf_match_addr(src_addr, include_directories,
                                filenames, ln, &regs);
                    break;
                }
            }
        }
    }
    return 0;
}

int dwarf_translate(struct addr2line_entry *ln, const void *section, size_t section_len)
{
    jmp_buf abort;
    if (!setjmp(abort)) {
        const uint8_t *section_start = section;
        const uint8_t *section_end = section + section_len;
        const uint8_t *entry = section_start;
        while (entry >= section_start && entry < section_end) {
            int rc = dwarf_translate_cu((dwarf_addr) ln->addr, &entry, ln, abort);
            if (rc < 0)
                return rc;
        }
        return 0;
    } else {
        return 0;
    }
}
