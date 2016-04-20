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
