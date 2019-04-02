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

  for (int i = 0; i < 12; i++) {
    pimp2_send_flit(prev_cid, 123);
  }

  for (int i = 0; i < 12; i++) {
    pimp2_recv_flit(next_cid);
  }

  Debug('k');
  Debug(10);
  return 0;
}
