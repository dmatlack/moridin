/**
 * @file lib/assert.c
 */
#include <assert.h>

#define ASSERT_FUNCTION_GENERATOR(_name, _op)                    \
    inline ASSERT_FUNCTION_PROTOTYPE(_name) {                    \
      if (!(left _op right)) {                                   \
        panic("%s:%s:%u: Failed assert!\n"                       \
              "   EXPECTED: %s "#_op" %s\n"                      \
              "        GOT: 0x%x <> 0x%x",                       \
              file, func, line, leftstr, rightstr, left, right); \
      }                                                          \
    }

ASSERT_FUNCTION_GENERATOR( __assert_equals,    == )
ASSERT_FUNCTION_GENERATOR( __assert_notequals, != )
ASSERT_FUNCTION_GENERATOR( __assert_less,      <  )
ASSERT_FUNCTION_GENERATOR( __assert_lesseq,    <= )
ASSERT_FUNCTION_GENERATOR( __assert_greater,   >  )
ASSERT_FUNCTION_GENERATOR( __assert_greatereq, >= )
