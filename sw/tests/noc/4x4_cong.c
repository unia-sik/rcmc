#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = fgmp_get_cid();
  cid_t target = (cid + 1) % 16;
  int cnt = 0;

  while (!fgmp_cong()) {
    pimp2_send_flit(target, (cid << 32 | target << 16 | cnt++));
  }

  test_result(true);

  return 0;
}
