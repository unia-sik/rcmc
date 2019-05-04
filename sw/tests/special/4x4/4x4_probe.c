#include <stdio.h>
#include <stdbool.h>

#include <fgmp.h>

#include "fgmp_test.h"

int main(int argc, char *argv[]) {

  cid_t cid = pimp2_get_cid();

  if (cid == 0) {
    pimp2_send_flit(1, 456);
  } else if (cid == 1) {
    fgmp_send_ready(0); 
        // one-to-one requires ready before sending,
        // for other routers it is a nop

    // Probe until data is available.
    while (pimp2_probe(0) == 0) {

    }

    flit_t data = pimp2_recv_flit(0);
    Debug(data == 456 ? 'k' : 'F');
  }
  return 0;
}