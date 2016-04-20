# Copyright (C) 2016 Franklin "Snaipe" Mathieu.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the LICENSE file distributed with Tizzy.

set (SOURCE_FILES
    src/addr2line.c
    src/addr2line.h
    src/tizzy.c
    src/backtrace.c
    src/backtrace.h
    src/basename.c
    src/basename.h
    src/dwarf.c
    src/dwarf.h
    src/section-${TZY_EXE_FORMAT}.c
    src/section.h
)

if (WIN32)
    set (SOURCE_FILES ${SOURCE_FILES}
        src/pdb.c
        src/pdb.h
    )
endif ()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
)

set (SOURCE_FILES ${SOURCE_FILES} PARENT_SCOPE)
