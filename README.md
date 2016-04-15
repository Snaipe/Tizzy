# Tizzy

A C library dedicated to error handling and panicking.

**Warning:** This library is experimental. Use at your own risk.

Tizzy provides convenience functions to help application-writers panic
when everything goes wrong, and provide them useful diagnostics for debugging
purposes.

## Example

```
$> cat -n panic.c

     1	#include <tizzy.h>
     2	
     3	static void baz(void) {
     4	    tzy_panic("Something, somewhere, went horribly wrong.");
     5	}
     6	
     7	static void bar(void) {
     8	    baz();
     9	}
    10	
    11	static void foo(void) {
    12	    bar();
    13	}
    14	
    15	int main(void)
    16	{
    17	    foo();
    18	    return 0;
    19	}

$> gcc -g -o panic panic.c -ltizzy

$> ./panic

panic: Something, somewhere, went horribly wrong.
  - at baz(panic.c:4)
  - at bar(panic.c:8)
  - at foo(panic.c:12)
  - at main(panic.c:17)
zsh: abort (core dumped)  ./panic
```
