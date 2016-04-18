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
