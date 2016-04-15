#ifndef ADDR2LINE_H_
# define ADDR2LINE_H_

# include <stddef.h>

# ifndef _WIN32
#  include <libdwarf/dwarf.h>
#  include <libdwarf/libdwarf.h>
struct addr2line_ctx {
    int fd;
    Dwarf_Debug dbg;
};
# else
struct addr2line_ctx {
    void *unused;
};
# endif

int addr2line_init(struct addr2line_ctx *ctx);
void addr2line_term(struct addr2line_ctx *ctx);

int addr2line_translate(struct addr2line_ctx *ctx, void *addr,
        char **file_out, size_t *lineno_out);

#endif /* !ADDR2LINE_H_ */
