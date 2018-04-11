/* fgmp_test.h
 * FGMP test helper functions
 */
#ifndef _FGMP_TEST_H
#define _FGMP_TEST_H

#include <stdbool.h>
#include "debug.h"

void _exit(int e);

static inline __attribute__((noreturn)) __attribute__((always_inline)) void test_result(bool pass) {
  // Pass test result.
  Debug(pass ? 'k' : 'F');
  Debug(10);
  _exit(1);
}


#endif // _FGMP_TEST_H
