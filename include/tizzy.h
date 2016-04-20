/*
 * The MIT License (MIT)
 *
 * Copyright © 2016 Franklin "Snaipe" Mathieu <http://snai.pe/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef TIZZY_H_
# define TIZZY_H_

# include <errno.h>
# include <stdarg.h>

# if defined __GNUC__
#  define TZY_GNU_FORMAT(Archetype, Idx, Ftc) \
    __attribute__((format(Archetype, Idx, Ftc)))
# else
#  define TZY_GNU_FORMAT
# endif

# if defined _MSC_VER
#  define TZY_MSC_FORMAT _Printf_format_string_
# else
#  define TZY_MSC_FORMAT
# endif

# if defined __GNUC__
#  define TZY_NORETURN __attribute__((noreturn))
# elif defined _MSC_VER
#  define TZY_NORETURN __declspec(noreturn)
# else
#  define TZY_NORETURN
# endif

typedef TZY_NORETURN void (*tzy_abort_fn)(void);
typedef void (*tzy_println_fn)(const char *, va_list);

TZY_NORETURN
TZY_GNU_FORMAT(printf, 1, 2)
void tzy_panic(TZY_MSC_FORMAT const char *str, ...);

TZY_NORETURN void tzy_abort(void);

void tzy_set_abort(tzy_abort_fn fn);
void tzy_set_println(tzy_println_fn fn);

void tzy_println(const char *str, ...);
void tzy_vprintln(const char *str, va_list vl);

# define tzy_errno_ignore(Stmt) do {    \
        int tzy_err__ = errno;          \
        Stmt;                           \
        errno = tzy_err__;              \
    } while (0)

# define tzy_errno_return(Err, Val) do {    \
        errno = (Err);                      \
        return (Val);                       \
    } while (0)

# define tzy_assert(Cnd) do {                           \
        if (!(Cnd)) {                                   \
            tzy_panic("Assertion failed: %s", #Cnd);    \
        }                                               \
    } while (0)

# ifdef NDEBUG
#  define tzy_debug_assert(Cnd)
# else
#  define tzy_debug_assert(Cnd) tzy_assert(Cnd)
# endif

#endif /* !TIZZY_H_ */
