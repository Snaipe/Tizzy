# Copyright (C) 2016 Franklin "Snaipe" Mathieu.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the LICENSE file distributed with Tizzy.

cmake_minimum_required (VERSION 2.8)

project (Tizzy C)

if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif ()

set (MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.cmake/Modules")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MODULE_DIR})

include (LibUtils)
include (Subprojects)
include (CheckSymbolExists)

if (NOT MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set (TZY_EXE_FORMAT elf)
    set (TZY_EXE_FMT_ELF 1)
    add_definitions(-D_GNU_SOURCE)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set (TZY_EXE_FORMAT mach-o)
    set (TZY_EXE_FMT_MACH_O 1)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set (TZY_EXE_FORMAT pe)
    set (TZY_EXE_FMT_PE 1)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    add_definitions (-DWIN32_LEAN_AND_MEAN)
elseif (CMAKE_SYSTEM_NAME MATCHES "((Free|Net|Open)BSD|Solaris|SunOS)")
    set (TZY_EXE_FORMAT elf)
    set (TZY_EXE_FMT_ELF 1)
else ()
    option (EXE_FORMAT "The executable format" "")
    if (NOT EXE_FORMAT)
        message (WARNING "No executable format specified for platform '${CMAKE_SYSTEM_NAME}', assuming elf")
        set (TZY_EXE_FORMAT elf)
        set (TZY_EXE_FMT_ELF 1)
    else ()
        set (TZY_EXE_FORMAT "${EXE_FORMAT}")
        string (TOUPPER "${EXE_FORMAT}" _EXE_SUFFIX)
        string (REGEX REPLACE "[^A-Z0-9_]" "_" _EXE_SUFFIX "${_EXE_SUFFIX}")
        set ("TZY_EXE_FMT_${_EXE_SUFFIX}" 1)
    endif ()
endif ()

list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

check_symbol_exists(mmap sys/mman.h HAVE_MMAP)
check_symbol_exists(mremap sys/mman.h HAVE_MREMAP)

list(REMOVE_ITEM CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

add_subdirectory (src)
include_directories (src include)

add_subproject (unwind
    GIT git://git.sv.gnu.org/libunwind.git
    OPTS --disable-coredump
         --disable-ptrace
         --disable-setjmp
         --disable-debug
         --disable-cxx-exceptions
         --disable-block-signals
         --disable-documentation
         --disable-shared
         --disable-minidebuginfo
         --with-pic
    AUTOTOOLS AUTOGEN_CONFIGURE)

add_convenience_library (tizzy_core ${SOURCE_FILES})
link_subproject (tizzy_core unwind)

merge_static_libs (tizzy tizzy_core ${unwind_STATIC_LIB})

add_subdirectory (samples)
