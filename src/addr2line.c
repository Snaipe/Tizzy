#include "addr2line.h"

#include <fcntl.h>
#include <unistd.h>

static inline int next_cu(Dwarf_Debug dbg)
{
    Dwarf_Error err;
    Dwarf_Unsigned cu;
    return dwarf_next_cu_header(dbg, NULL, NULL, NULL, NULL, &cu, &err);
}

static void reset_dwarf_cu(Dwarf_Debug dbg, int rc)
{
    Dwarf_Error err;
    while (rc == DW_DLV_OK)
        rc = next_cu(dbg);
}

static Dwarf_Die find_die_by_tag(Dwarf_Debug dbg, Dwarf_Half tag)
{
    Dwarf_Error err;
    Dwarf_Die die = NULL;
    while (dwarf_siblingof(dbg, die, &die, &err) == DW_DLV_OK) {
        Dwarf_Half die_tag;
        if (dwarf_tag(die, &die_tag, &err) != DW_DLV_OK)
            break;
        if (die_tag == tag)
            return die;
    }
    return NULL;
}

int addr2line_init(struct addr2line_ctx *ctx)
{
    Dwarf_Error err;
    ctx->fd = open("/proc/self/exe", O_RDONLY);
    if (ctx->fd == -1)
        return 0;

    int rc = dwarf_init(ctx->fd, DW_DLC_READ, NULL, NULL, &ctx->dbg, &err);
    if (rc != DW_DLV_OK) {
        close(ctx->fd);
        return 0;
    }
    return 1;
}

void addr2line_term(struct addr2line_ctx *ctx)
{
    Dwarf_Error err;
    dwarf_finish(ctx->dbg, &err);
    close(ctx->fd);
}

int addr2line_translate(struct addr2line_ctx *ctx, void *_addr,
        char **file_out, size_t *lineno_out)
{
    Dwarf_Debug dbg = ctx->dbg;
    Dwarf_Addr addr = (Dwarf_Addr) _addr;

    int rc = 0;
    Dwarf_Error err;

    int ret;
    for (ret = next_cu(dbg); ret == DW_DLV_OK; ret = next_cu(dbg)) {
        Dwarf_Die die = find_die_by_tag(dbg, DW_TAG_compile_unit);
        if (die == NULL)
            goto reset;

        Dwarf_Addr lopc, hipc;
        Dwarf_Half hipc_o;
        enum Dwarf_Form_Class kind;

        int has_attrs = dwarf_lowpc(die, &lopc, &err) == DW_DLV_OK
            && dwarf_highpc_b(die, &hipc, &hipc_o, &kind, &err) == DW_DLV_OK;
        if (has_attrs) {
            if (kind == DW_FORM_CLASS_CONSTANT)
                hipc += lopc;
            if (addr < lopc || addr >= hipc)
                continue;
        }

        Dwarf_Line *lines;
        Dwarf_Signed lcount;
        if (dwarf_srclines(die, &lines, &lcount, &err) != DW_DLV_OK)
            goto reset;

        Dwarf_Addr raddr = -1;
        *lineno_out = 0;
        *file_out = NULL;

        for (int i = 0; i < lcount; i++) {
            Dwarf_Addr lineaddr;
            if (dwarf_lineaddr(lines[i], &lineaddr, &err) != DW_DLV_OK)
                goto reset;

            Dwarf_Unsigned lineno;
            if (dwarf_lineno(lines[i], &lineno, &err) != DW_DLV_OK)
                goto reset;

            char *file = NULL;
            char *linesrc;
            if (dwarf_linesrc(lines[i], &linesrc, &err) == DW_DLV_OK)
                file = linesrc;

            if (addr == lineaddr || (addr < lineaddr && addr > raddr))
                goto success;

            raddr = lineaddr;
            *lineno_out = lineno;
            *file_out = file;
        }
    }

success:
    rc = 1;

reset:
    reset_dwarf_cu(dbg, ret);
    return rc;
}
