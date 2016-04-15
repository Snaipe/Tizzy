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
