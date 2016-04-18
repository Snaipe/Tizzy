#ifndef ADDR2LINE_H_
# define ADDR2LINE_H_

# include <stddef.h>
# include "section.h"

struct addr2line_entry {
    const void *addr;
    const void *max_addr;
    size_t line;
    size_t column;
    const char *filename;
    const char *dirname;
};

int addr2line(void *addr, const char **file_out, size_t *lineno_out);

#endif /* !ADDR2LINE_H_ */
