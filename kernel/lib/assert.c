/**
 * @file lib/assert.c
 */
#include <assert.h>
#include <kernel/debug.h>
#include <errno.h>

#define ASSERT_FUNCTION_GENERATOR(_name, _op)                    \
    inline ASSERT_FUNCTION_PROTOTYPE(_name) {                    \
      if (!(left _op right)) {                                   \
        panic("%s:%u:%s: Failed assert!\n"                       \
              "   EXPECTED: %s "#_op" %s\n"                      \
              "        GOT: 0x%x <> 0x%x",                       \
              file, line, func, leftstr, rightstr, left, right); \
      }                                                          \
    }

ASSERT_FUNCTION_GENERATOR( __assert_equals,    == )
ASSERT_FUNCTION_GENERATOR( __assert_notequals, != )
ASSERT_FUNCTION_GENERATOR( __assert_less,      <  )
ASSERT_FUNCTION_GENERATOR( __assert_lesseq,    <= )
ASSERT_FUNCTION_GENERATOR( __assert_greater,   >  )
ASSERT_FUNCTION_GENERATOR( __assert_greatereq, >= )

#define SOX_FMT "%s:%d, in %s(), %s returned %d (%s)"

void __succeed_or_die(int expr, const char * exprstr,
                      const char *file, const char *func, int line)
{
	if (0 != expr) {
		ERROR(SOX_FMT, file, line, func, exprstr, expr, strerr(expr));
		panic("PANIC "SOX_FMT, file, line, func, exprstr, expr, strerr(expr));
	}
}

void __succeed_or_warn(int expr, const char * exprstr,
                       const char *file, const char *func, int line)
{
	if (0 != expr) {
		WARN(SOX_FMT, file, line, func, exprstr, expr, strerr(expr));
	}
}
