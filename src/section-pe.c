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
int module_from_address(const void *addr, mod_handle *mod)
{
    return !!GetModuleHandleEx(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        addr, mod);
}

void close_module(mod_handle *mod)
{
    (void) mod;
}

static const char *get_strtab(PIMAGE_DOS_HEADER dos_hdr)
{
    PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER) *mod;
    PIMAGE_NT_HEADERS nt_hdr = (PIMAGE_NT_HEADERS) ((uintptr_t)dos_hdr
            + dos_hdr->e_lfanew);
    PIMAGE_FILE_HEADER file_hdr = &nt_hdr->FileHeader;

    size_t off = ifh->PointerToSymbolTable;
    if (!off)
        return NULL;

    PIMAGE_SYMBOL symtab = NULL;
    PIMAGE_SECTION_HEADER sec_hdr = IMAGE_FIRST_SECTION(nt_hdr);
    for(size_t i = 0; i < file_hdr->NumberOfSections; i++, sec_hdr++) {
        size_t soff = sec_hdr->PointerToRawData;
        if (off >= soff && off < soff + sec_hdr->SizeOfRawData) {
            uintptr_t addr = soff - off + sec_hdr->VirtualAddress;
            symtab = (PIMAGE_SYMBOL) ((const char*) dos_hdr + addr);
        }
    }

    if (!symtab)
        return NULL;
    return &symtab[file_hdr->NumberOfSymbols];
}

static int sec_name_eq(PIMAGE_SECTION_HEADER sec, const char *name
        const char *strtab)
{
    const char *sec_name = (char *) sec->Name;
    if (*sec_name == '/') {
        char buf[IMAGE_SIZEOF_SHORT_NAME];
        strncpy(buf, sec_name + 1, sizeof (buf) - 1);
        buf[sizeof (buf) - 1] = '\0';

        int idx = atoi(buf);
        return !strcmp(strtab[idx], name);
    }
    return !strncmp(sec_name, name, IMAGE_SIZEOF_SHORT_NAME);
}

const void *map_section_data(mod_handle *mod, const char *name,
        struct section_mapping *map)
{
    PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER) *mod;
    PIMAGE_NT_HEADERS nt_hdr = (PIMAGE_NT_HEADERS) ((uintptr_t) dos_hdr
            + dos_hdr->e_lfanew);

    const char *strtab = get_strtab(dos_hdr);
    if (!strtab && strlen(name) > IMAGE_SIZEOF_SHORT_NAME)
        return NULL;

    PIMAGE_SECTION_HEADER sec_hdr = IMAGE_FIRST_SECTION(nt_hdr);
    for(int i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++, sec_hdr++) {
        if (sec_name_eq(sec_hdr, section, strtab)) {
            return (char *) dos_hdr + sec_hdr->VirtualAddress;
        }
    }
}

void unmap_section_data(struct section_mapping *map)
{
    (void) map;
}
