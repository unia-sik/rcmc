#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = pimp2_get_cid();

  if (cid == 0) {
    pimp2_send_flit(1, 456);
  } else if (cid == 1) {
    flit_t data = pimp2_recv_flit(0);
    if (data != 456) {
      test_result(false);
    }
  }

  test_result(true);

  return 0;
}
