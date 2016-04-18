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
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <string.h>

#include "addr2line.h"
#include "basename.h"
#include "tizzy.h"

void tzy_print_backtrace(void)
{
    char buf[1024];

    unw_context_t uc;
    unw_getcontext(&uc);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0) {
        unw_word_t ip;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        unw_word_t offp;
        int rc = unw_get_proc_name(&cursor, buf, sizeof (buf), &offp);
        const char *name = rc < 0 ? "<unknown fn>" : buf;

        if (!strncmp(name, "tzy_", 4))
            continue;

        const char *file;
        size_t line;
        if (addr2line((void*) ip, &file, &line)) {
            tzy_println("  - at %s(%s:%llu)",
                    name, file, (unsigned long long) line);
        } else {
            tzy_println("  - at %s(unknown @ %p)", name, (void*) ip);
        }

        if (!strcmp(name, "main"))
            break;
    }
}
