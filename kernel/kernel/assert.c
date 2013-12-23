/**
 * @file lib/assert.c
 */
#include <assert.h>

inline void __assert_equals(const char *file, const char *func, int line,
                            int left, const char *leftstr, 
                            int right, const char *rightstr) {
  if (left != right) {
      panic("%s:%s:%u: Failed ASSERT_EQUALS!\n"
            "   EXPECTED: %s == %s\n"
            "        GOT: 0x%x != 0x%x",
            file, func, line, leftstr, rightstr, left, right);
  }
}
