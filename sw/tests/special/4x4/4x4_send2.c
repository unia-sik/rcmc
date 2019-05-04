#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = pimp2_get_cid();
  cid_t next_cid = (cid + 1) % 16;
  cid_t prev_cid = cid - 1;

  if (prev_cid < 0) {
    prev_cid = 15;
  }

  if (cid == 0) {
    pimp2_send_flit(prev_cid, 123);
    flit_t data = pimp2_recv_flit(next_cid);
    if (data != 123) {
      Debug('F');
      Debug(10);
      return 1;
    }
  } else {
    flit_t data = pimp2_recv_flit(next_cid);
    pimp2_send_flit(prev_cid, data);
  }

  Debug('k');
  Debug(10);
  return 0;
}
