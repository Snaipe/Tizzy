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
#ifndef SECTION_H_
# define SECTION_H_

# include "config.h"

# if defined TZY_EXE_FMT_ELF
#  define MODULE_INVALID NULL
#  include <link.h>

typedef struct mod_handle {
    int fd;
    const ElfW(Ehdr) *map;
    size_t len;
} mod_handle;
# elif defined TZY_EXE_FMT_MACH_O
#  define MODULE_INVALID -1
typedef int mod_handle;
# endif

struct section_mapping {
    const void *map;
    size_t len;
    size_t sec_len;
};

int module_from_address(const void *addr, mod_handle *mod);
void close_module(mod_handle *mod);
const void *map_section_data(mod_handle *mod, const char *name,
        struct section_mapping *map);
void unmap_section_data(struct section_mapping *map);

#endif /* !SECTION_H_ */
