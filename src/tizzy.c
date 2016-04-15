#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tizzy.h"
#include "backtrace.h"

extern char *__progname;

static tzy_abort_fn abort_fn;
static tzy_println_fn println_fn;

void tzy_set_abort(tzy_abort_fn fn)
{
    abort_fn = fn;
}

TZY_NORETURN void tzy_abort(void)
{
    (abort_fn ? abort_fn : abort)();
}

static void println_stderr(const char *str, va_list vl)
{
    vfprintf(stderr, str, vl);
    fputs("\n", stderr);
}

void tzy_set_println(tzy_println_fn fn)
{
    println_fn = fn;
}

void tzy_println(const char *str, ...)
{
    va_list vl;
    va_start(vl, str);
    (println_fn ? println_fn : println_stderr)(str, vl);
    va_end(vl);
}

void tzy_vprintln(const char *str, va_list vl)
{
    (println_fn ? println_fn : println_stderr)(str, vl);
}

TZY_NORETURN void tzy_panic(const char *str, ...)
{
    va_list vl;
    va_start(vl, str);

    char *line = alloca(strlen(str) + strlen(__progname) + 3);
    *line = '\0';

    strcat(line, __progname);
    strcat(line, ": ");
    strcat(line, str);

    tzy_println(line, vl);
    va_end(vl);

    tzy_print_backtrace();

    tzy_abort();
}
