#include <tizzy.h>

static void baz(void) {
    tzy_panic("Something, somewhere, went horribly wrong.");
}

static void bar(void) {
    baz();
}

static void foo(void) {
    bar();
}

int main(void)
{
    foo();
    return 0;
}
