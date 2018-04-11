#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = fgmp_get_cid();

  if (cid == 3) {
    fgmp_send_flit(1, 456);
  } else if (cid == 1) {
    cid_t any = -1;
    while ((any = fgmp_any()) == -1) {

    }
    if (any != 3) {
      test_result(false);
    }
  }

  test_result(true);

  return 0;
}
