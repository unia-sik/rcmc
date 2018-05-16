/*
 * Send to every node a message of a random length.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fgmp.h"


#define MAX_LEN 100

uint32_t seed;

uint32_t my_random(uint32_t max)
{
    seed = (1103515245*seed + 12345) & 0x7fffffff;
    return seed % max;
}

int main(int argc, char *argv[])
{
    int i;
    cid_t cid = fgmp_get_cid();
    cid_t max_cid = fgmp_get_max_cid();

    bool finished[max_cid];
    unsigned received[max_cid];
    unsigned sent[max_cid];
    unsigned recv_node_remain = max_cid-1;
    unsigned send_node_remain = max_cid-1;
    unsigned send_msg_remain = my_random(MAX_LEN)+1;
    unsigned send_no = 1;
    unsigned send_cid = my_random(max_cid-1);
    if (send_cid>=cid) send_cid++; // don't send to myself

    seed = cid;
    for (i=0; i<max_cid; i++) {
        finished[i] = false;
        received[i] = 0;
        sent[i] = 0;
    }
    sent[send_cid] = send_msg_remain;
    sent[cid] = 1;
    finished[cid] = true;

    do {
        // receive
        cid_t recv_cid = fgmp_any();
        while (recv_cid>=0) {
            flit_t flit = fgmp_recv_flit(recv_cid);
            cid_t cid_in_flit = flit >> 12;
            unsigned msgtype = (flit >> 10) & 1;
            unsigned no = flit & 0x3ff;

            if (cid_in_flit != recv_cid) {
                printf("source cid in flit (%lu) is wrong, should be %lu\n", 
                    cid_in_flit, recv_cid);
                exit(1);
            }
            if (received[recv_cid]+1 != no) {
                printf("node %lu: body flits out of order: "
                    "flit %u from %lu has number %u\n", 
                    cid, received[recv_cid]+1, recv_cid, no);
                exit(1);
            }
            received[recv_cid] = no;
            if (finished[recv_cid]) {
                printf("additional flits after tail flit\n");
                exit(1);
            }
            if (msgtype==1) {
                //printf("%lu<%u>%lu\n", recv_cid, no, cid);
                finished[recv_cid] = true; // tail flit
                recv_node_remain--;
            }
            recv_cid = fgmp_any();
        }

        // send
        if (!fgmp_cong() && send_node_remain!=0) {
            if (send_msg_remain>1) {
                fgmp_send_flit(send_cid, (cid<<12) | send_no);
                send_msg_remain--;
                send_no++;
            } else {
                fgmp_send_flit(send_cid, (cid<<12) | 0x400 | send_no);
                send_node_remain--;
                if (send_node_remain!=0) {
                    // next node
                    send_cid = my_random(max_cid);
                    while (sent[send_cid]!=0) {
                        send_cid++;
                        if (send_cid==max_cid) send_cid = 0;
                    }
                    send_msg_remain = my_random(MAX_LEN)+1;
                    sent[send_cid] = send_msg_remain;
                    send_no = 1;
                }
            }
        }
    } while (send_node_remain!=0 || recv_node_remain!=0);

    putchar('k');
    putchar('\n');
    return 0;
}
