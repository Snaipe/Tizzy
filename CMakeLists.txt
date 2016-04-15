# Copyright (C) 2016 Franklin "Snaipe" Mathieu.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the LICENSE file distributed with Tizzy.

cmake_minimum_required (VERSION 2.8)

project (Tizzy C)

set (MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.cmake/Modules")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MODULE_DIR})

include (LibUtils)
include (Subprojects)

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
         --without-libunwind-${CMAKE_SYSTEM_PROCESSOR}
    AUTOTOOLS AUTOGEN_CONFIGURE)

add_convenience_library (tizzy_core ${SOURCE_FILES})
link_subproject (tizzy_core unwind)

message("unwind lib = ${unwind_STATIC_LIB}")
merge_static_libs (tizzy tizzy_core ${unwind_STATIC_LIB})
