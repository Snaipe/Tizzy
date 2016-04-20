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
#include <getsect.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>

static int open_self(void)
{
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (!_NSGetExecutablePath(path, &size) == 0)
        return -1;
    return open(path, O_RDONLY);
}

static inline void *get_real_address(int lib, void *addr) {
    if (!addr)
        return NULL;

    // We need to slide the section address to get a valid pointer
    // because ASLR will shift the image by a random offset
    return addr + _dyld_get_image_vmaddr_slide(lib);
}

static inline void *ptr_add(const void *ptr, size_t off)
{
    return (char *) ptr + off;
}

int module_from_address(const void *addr, mod_handle *mod)
{
    size_t nb_images = _dyld_image_count();
    for (size_t i = 1; i < nb_images; ++i) {
        const struct mach_header *hdr = _dyld_get_image_header(lib);
        const struct load_command *lc = ptr_add(hdr, sizeof (*hdr));
        for (size_t i = 0; i < hdr->ncmds; ++i, lc = ptr_add(lc, lc->cmdsize)) {
            const char *vmaddr = NULL;
            size_t vmsize = 0;
            if (lc->cmd == LC_SEGMENT) {
                const struct segment_command *sc = (void *) lc;
                vmaddr = get_real_address(lib, sc->vmaddr)
                vmsize = sc->vmsize;
            } else if (lc->cmd == LC_SEGMENT_64) {
                const struct segment_command_64 *sc = (void *) lc;
                vmaddr = get_real_address(lib, sc->vmaddr)
                vmsize = sc->vmsize;
            }
            if (addr >= vmaddr && addr < vmaddr + vmsize) {
                *mod = lib;
                return 1;
            }
        }
    }
    return 0;
}

void close_module(mod_handle *mod)
{
    (void) mod;
}

const void *map_section_data(mod_handle *mod, const char *name,
        struct section_mapping *map)
{
    (void) map;

    const struct load_command *lc = ptr_add(hdr, sizeof (mach_hdr));
    for (size_t i = 0; i < hdr->ncmds; ++i, lc = ptr_add(lc, lc->cmdsize)) {
        if (lc->cmd == LC_SEGMENT) {
            const struct segment_command *sc = (void *) lc;
            if (strncmp("__DATA", sc->segname, 16))
                continue;
            const struct section *s = ptr_add(sc, sizeof (*sc));
            for (size_t j = 0; j < sc->nsects; ++j, ++s) {
                if (strncmp(name, s->sectname, 16))
                    continue;

                return get_real_address(*mod, (void *) s->addr);
            }
        } else if (lc->cmd == LC_SEGMENT_64) {
            const struct segment_command_64 *sc = (void *) lc;
            if (strncmp("__DATA", sc->segname, 16))
                continue;
            const struct section_64 *s = ptr_add(sc, sizeof (*sc));
            for (size_t j = 0; j < sc->nsects; ++j, ++s) {
                if (strncmp(name, s->sectname, 16))
                    continue;

                return get_real_address(*mod, (void *) s->addr);
            }
        }
    }
    return NULL;
}

void unmap_section_data(struct section_mapping *map)
{
    (void) map;
}
