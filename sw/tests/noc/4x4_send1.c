#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = fgmp_get_cid();
  cid_t next_cid = (cid + 1) % 16;
  cid_t prev_cid = cid - 1;

  if (prev_cid < 0) {
    prev_cid = 15;
  }

  if (cid == 0) {
    fgmp_send_flit(next_cid, 123);
    flit_t data = fgmp_recv_flit(prev_cid);
    if (data == 123) {
      test_result(true);
    }
  } else {
    flit_t data = fgmp_recv_flit(prev_cid);
    fgmp_send_flit(next_cid, data);
    test_result(true);
  }

  test_result(true);

  return 0;
}
