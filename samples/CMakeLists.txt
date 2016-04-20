# Copyright (C) 2016 Franklin "Snaipe" Mathieu.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the LICENSE file distributed with Tizzy.

set (CMAKE_BUILD_TYPE "Debug")

add_executable(panic panic.c)
target_link_libraries(panic tizzy)
