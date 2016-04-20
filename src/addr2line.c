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
#include <stdio.h>
#include "addr2line.h"
#include "dwarf.h"

int addr2line(void *_addr, const char **file_out, size_t *lineno_out)
{
    int rc = 0;
    mod_handle mod;

    if (!module_from_address(_addr, &mod))
        return 0;

    struct section_mapping map;
    const void *debug_line = map_section_data(&mod, ".debug_line", &map);

    if (!debug_line)
        goto finish;

    struct addr2line_entry e = { .addr = _addr };

    if (dwarf_translate(&e, debug_line, map.sec_len) < 0)
        goto finish;

    if (e.filename == NULL)
        goto finish;

    *file_out = e.filename;
    *lineno_out = e.line;

    rc = 1;

finish:
    if (debug_line)
        unmap_section_data(&map);
    close_module(&mod);

    return rc;
}
