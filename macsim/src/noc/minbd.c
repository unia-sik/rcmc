/*
 * minbd.c
 * Minimally Buffered Deflection
 *
 * RC/MC project
 */

#include "minbd.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


#define DEBUG(...)
//#define DEBUG printf



/*
#include "netrace.h"
// check if flit is the last flit of packet id from netrace
static bool checkid(flit_t flit, unsigned packet_id)
{
    if (flit!=0) {
        nt_packet_t *packet = (nt_packet_t *)flit;
        if (packet->id==packet_id) return true;
    }
    return false;
}

*/


extern const char letter[64];


unsigned long stat_recvbuf_max = 0;


bool minbd_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    flit_container2_t fc;
    unsigned long *no = ((minbd_context_t *)node->noc_context)->send_no;
    fc.src = no[dest]*conf_max_rank + node->rank;
//    no[dest]++;
    fc.dest =  dest;
    fc.flit = flit;
    bool r = flitfifo_insert(&((minbd_context_t *)node->noc_context)->send_fifo, &fc);
    if (r) {
        no[dest]++;
        DEBUG("S %lu->%lu(%lx)\n", node->rank, dest, flit);
    }
    return r;
}

bool minbd_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container2_t fc;
    unsigned long *no = ((minbd_context_t *)node->noc_context)->recv_no;
    if (fc_dequeue_src(&((minbd_context_t *)node->noc_context)->recv_fifo, 
        no[src]*conf_max_rank+src, &fc)) 
    {
        DEBUG("R %lu->%lu(%lx)\n", src, node->rank, fc.flit);
        *flit = fc.flit;
        no[src]++;
        return true;
    }
    return false;
}

bool minbd_sender_ready(node_t *node)
{
    return !flitfifo_full(&((minbd_context_t *)node->noc_context)->send_fifo);
}

bool minbd_probe_rank(node_t *node, rank_t src)
{
    minbd_context_t  *r  = node->noc_context;
    return fc_find_src(&r->recv_fifo, r->recv_no[src]*conf_max_rank+src) >= 0;
}

rank_t minbd_probe_any(node_t *node)
{
    minbd_context_t  *r  = node->noc_context;
    unsigned long    *no = r->recv_no;
    fc_queue_entry_t *h  = r->recv_fifo.head;

    while (h) {
        rank_t src = h->fc.src % conf_max_rank;
        if (h->fc.src == (no[src]*conf_max_rank+src)) return src;
        h = h->next;
    }
    return -1;
}


static uint32_t seed = 1;

static uint32_t rand4()
{
    seed = (1103515245*seed + 12345) & 0x7fffffff;
    return seed & 3;
}


#define conf_redir_threshold 2
#define conf_send_fifo_size (conf_noc_width)
#define conf_side_fifo_size 4

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

static void router_one_cycle(minbd_context_t *r)
{
    unsigned i;
    unsigned eject_count=0;
    bool valid[4];
    flit_container2_t fc[4];
    unsigned productive[4];


    // 2nd stage
    // ----------



    // deflection
    for (i=0; i<4; i++) {
        valid[i] = r->s2_valid[i];
        if (!valid[i]) {
            productive[i] = 0;
        } else {
            fc[i] = r->s2_fc[i];
            int x = X_FROM_RANK(fc[i].dest) - r->x;
            int y = Y_FROM_RANK(fc[i].dest) - r->y;
            if (x>=0) {
                if (y>=0) {
                    productive[i] = (x > y) ? EAST : SOUTH;
                } else {
                    productive[i] = (x > (-y)) ? EAST : NORTH;
                }
            } else {
                if (y>=0) {
                    productive[i] = ((-x) > y) ? WEST : SOUTH;
                } else {
                    productive[i] = ((-x) > (-y)) ? WEST : NORTH;
                }
            }
        }
    }

    // 0 N -- 0 -> P0 -- P2 -> 0 N
    // 1 S \/ 2 -> P0 \/ P2 -> 1 S
    // 2 E /\ 1 -> P1 /\ P3 -> 2 E
    // 3 W -- 3 -> P1 -- P3 -> 3 W
    unsigned silver = rand4(); // silver flit
    if (!valid[silver]) silver=4;
    unsigned p[4];
    unsigned q[4];
    // P0 (0,2)
    if (silver==0 || !valid[2]) {
        // 0 prefered
        if (productive[0]<2) { p[0]=0; p[1]=2; }
                        else { p[0]=2; p[1]=0; }
    } else {
        // 2 prefered
        if (productive[2]>=2) { p[0]=0; p[1]=2; }
                         else { p[0]=2; p[1]=0; }
    }

    // P1 (1,3)
    if (silver==1 || !valid[3]) {
        // 1 prefered
        if (productive[1]<2) { p[2]=1; p[3]=3; }
                        else { p[2]=3; p[3]=1; }
    } else {
        // 3 prefered
        if (productive[3]>=2) { p[2]=1; p[3]=3; }
                         else { p[2]=3; p[3]=1; }
    }

    // P2 (p[0],p[2])
    if (silver==p[0] || !valid[p[2]]) {
        // p[0] prefered
        if (productive[p[0]]==0) { q[0]=p[0]; q[1]=p[2]; }
                            else { q[0]=p[2]; q[1]=p[0]; }
    } else {
        // p[2] prefered
        if (productive[p[2]]==1) { q[0]=p[0]; q[1]=p[2]; }
                            else { q[0]=p[2]; q[1]=p[0]; }
    }

    // P3 (p[1],p[3])
    if (silver==p[1] || !valid[p[3]]) {
        // p[1] prefered
        if (productive[p[1]]==2) { q[2]=p[1]; q[3]=p[3]; }
                            else { q[2]=p[3]; q[3]=p[1]; }
    } else {
        // p[3] prefered
// wrong:
//      if (productive[p[2]]==3) { q[2]=p[1]; q[3]=p[3]; }
//                          else { q[2]=p[3]; q[3]=p[1]; }
        if (productive[p[3]]==3) { q[2]=p[1]; q[3]=p[3]; }
                            else { q[2]=p[3]; q[3]=p[1]; }
    }

    for (i=0; i<4; i++) {
        r->out_valid[i] = valid[q[i]];
        r->out_fc[i]    =    fc[q[i]];
    }


    // buffer eject
    if (!flitfifo_full(&r->side_fifo)) {
        for (i=0; i<4; i++) {
            if (r->out_valid[i] && productive[q[i]]!=i) {

                flitfifo_insert(&r->side_fifo, &r->out_fc[i]);
                r->out_valid[i] = false;
                break;
            }
        }
    }

/*
if (r->rank==17) {
  unsigned a[4];
  for (i=0; i<4; i++) a[q[i]] = i;
  printf("productive=%d%d%d%d P=%d%d%d%d q=%d%d%d%d a=%d%d%d%d silver=%d\n",
    productive[0], productive[1], productive[2], productive[3],
    p[0], p[1], p[2], p[3], q[0], q[1], q[2], q[3], a[0], a[1], a[2], a[3],
    silver);
}
*/
#define FID 1015

    // 1st stage
    // ----------


    unsigned free_slots=0;


    // double eject
    for (i=0; i<4; i++) {
        valid[i] = r->in_valid[i];
        fc[i] = r->in_fc[i];
        if (r->in_valid[i] && r->in_fc[i].dest==r->rank && eject_count<2) {
            if (!fc_enqueue(&r->recv_fifo, fc[i])) fatal("Out of memory (reassembly buffer)");
//if (checkid(fc[i].flit, FID)) {
//    printf("FLIT %d: received at node %ld from node %lx\n", FID, r->rank, fc[i].src);
//    fc_print_queue(&r->recv_fifo);
//    printf("\n");
//}
            if (r->recv_fifo.count > stat_recvbuf_max) 
                stat_recvbuf_max = r->recv_fifo.count;
            eject_count++;
            valid[i] = false;
        }
        if (!valid[i]) free_slots++;
    }

    // redirection
    if (free_slots==0) {
        if (!flitfifo_empty(&r->side_fifo)) {
            r->redir_counter++;
            if (r->redir_counter > conf_redir_threshold) {
                i = rand4();
                flit_container2_t redir_fc = fc[i];
                flitfifo_remove(&r->side_fifo, &fc[i]);
                flitfifo_insert(&r->side_fifo, &redir_fc);
                r->redir_counter = 0;
            }
        }
    } else {
        r->redir_counter = 0;

        // buffer inject
        if (!flitfifo_empty(&r->side_fifo)) {
            i=0; while (valid[i]) i++; // search free slot
            flitfifo_remove(&r->side_fifo, &fc[i]);
            valid[i] = true;
            free_slots--;
        }

        // inject
        if (free_slots!=0 && !flitfifo_empty(&r->send_fifo)) {
            i=0; while (valid[i]) i++; // search free slot
            flitfifo_remove(&r->send_fifo, &fc[i]);
            valid[i] = true;
            free_slots--;
        }
    }

    for (i=0; i<4; i++) {
        r->s2_valid[i] = valid[i];
        r->s2_fc[i] = fc[i];
    }

/*
for (i=0; i<4; i++) {
  if (r->in_valid[i] && checkid(r->in_fc[i].flit, FID))
    printf("FLIT %d: node %ld input %d\n", FID, r->rank, i);
  if (r->out_valid[i] && checkid(r->out_fc[i].flit, FID))
    printf("FLIT %d: node %ld output %d\n", FID, r->rank, i);
  if (r->s2_valid[i] && checkid(r->s2_fc[i].flit, FID))
    printf("FLIT %d: node %ld s2 %d\n", FID, r->rank, i);
}

if (r->rank==9) {
  fc_print_queue(&r->recv_fifo);
  printf(" recv_no[28]=%lx node 9\n", r->recv_no[28]*conf_max_rank+28); 
}


if (r->rank==28) {
  flitfifo_print(&r->send_fifo);
  printf(" send fifo node 28\n");
}
*/



}





// initialize the best effort paternoster interconnection simulator
void minbd_init(node_t *n)
{
    // set function pointers
    n->noc_send_flit         = minbd_send_flit;
    n->noc_recv_flit         = minbd_recv_flit;
    n->noc_sender_ready      = minbd_sender_ready;
    n->noc_probe_rank        = minbd_probe_rank;
    n->noc_probe_any         = minbd_probe_any;
//    n->noc_route_one_cycle   = minbd_route_one_cycle;

    minbd_context_t *r = fatal_malloc(sizeof(minbd_context_t));
    n->noc_context = r;
    r->rank = n->rank;

    unsigned i;
    for (i=0; i<4; i++) {
        r->in_valid[i] = false;
        r->s2_valid[i] = false;
    }
    r->redir_counter = 0;

    r->send_no = fatal_malloc(sizeof(unsigned long)*conf_max_rank);
    r->recv_no = fatal_malloc(sizeof(unsigned long)*conf_max_rank);
    for (i=0; i<conf_max_rank; i++) {
        r->send_no[i] = 0;
        r->recv_no[i] = 0;
    }
    flitfifo_init(&r->send_fifo, conf_send_fifo_size);
    fc_init_queue(&r->recv_fifo);
    flitfifo_init(&r->side_fifo, conf_side_fifo_size);
}


void minbd_destroy(node_t *node)
{
    minbd_context_t *r = node->noc_context;
    flitfifo_destroy(&r->send_fifo);
    fc_destroy_queue(&r->recv_fifo);
    flitfifo_destroy(&r->side_fifo);
    free(r->send_no);
    free(r->recv_no);
    free(node->noc_context);
}


// Connect to the neigbour routers.
// All routers must be initalised before.
void minbd_connect(node_t *me, rank_t x, rank_t y,
    node_t *north, node_t *south, node_t *west, node_t *east)
{
    minbd_context_t *r = me->noc_context;
    r->x = x;
    r->y = y;
    r->rank = y*conf_noc_width+x;
    r->north = north->noc_context;
    r->south = south->noc_context;
    r->west = west->noc_context;
    r->east = east->noc_context;
}


void print_minbd_ctx(node_t *node)
{
    unsigned i;
    minbd_context_t *r = node->noc_context;

    printf("in  s2  out\n");
    for (i=0; i<4; i++) {
        print_flit(r->in_valid[i], r->in_fc[i]); putchar(' ');
        print_flit(r->s2_valid[i], r->s2_fc[i]); putchar(' ');
        print_flit(r->out_valid[i], r->out_fc[i]);
        printf(" redir=%d\n", r->redir_counter);
    }
}



void minbd_route_all(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    for (r=0; r<max_rank; r++) {
        router_one_cycle(nodes[r]->noc_context);
    }

    for (r=0; r<max_rank; r++) {
        minbd_context_t *self    = nodes[r]->noc_context;
        self->in_valid[NORTH]    = self->north->out_valid[SOUTH];
        self->in_fc[NORTH]       = self->north->out_fc[SOUTH];
        self->in_valid[SOUTH]    = self->south->out_valid[NORTH];
        self->in_fc[SOUTH]       = self->south->out_fc[NORTH];
        self->in_valid[EAST]     = self->east->out_valid[WEST];
        self->in_fc[EAST]        = self->east->out_fc[WEST];
        self->in_valid[WEST]     = self->west->out_valid[EAST];
        self->in_fc[WEST]        = self->west->out_fc[EAST];
    }

/*
    rank_t i, j;
    unsigned k, l;
    for (i=0; i<max_rank; i++) {
        minbd_context_t *a = nodes[i]->noc_context;
        for (j=i+1; j<max_rank; j++) {
            minbd_context_t *b = nodes[j]->noc_context;
            for (k=0; k<4; k++) {
                for (l=0; l<4; l++) {
                    if (a->out_valid[k] && b->out_valid[l] 
                        && a->out_fc[k].flit==b->out_fc[l].flit
                        && a->out_fc[k].src==b->out_fc[l].src
                        && a->out_fc[k].dest==b->out_fc[l].dest)
                    {
                        printf("#%lu SAME FLIT in %lu[%u] and %lu[%u]: ",
                            nodes[0]->cycle, i, k, j, l);
                        print_flit(true, a->out_fc[k]);
                        printf("\n");
//                        print_minbd_ctx(nodes[i]);
//                        print_minbd_ctx(nodes[j]);
                    }
                }
            }
        }
    }
*/
}




void minbd_print_context(node_t *nodes[], rank_t max_rank)
{
    rank_t x, y;

/*
    rank_t i;
    for (i=0; i<conf_max_rank; i++) {
        minbd_context_t *r = nodes[i]->noc_context;
        printf("°%lx ", i);
        flitfifo_print(&r->send_fifo);
        printf(" || side: ");
        flitfifo_print(&r->side_fifo);
        printf("\n");
    }
*/
    print_minbd_ctx(nodes[9]);
    print_minbd_ctx(nodes[10]);
    print_minbd_ctx(nodes[28]);

    for (y=0; y<conf_noc_height; y++) {
        printf("   ");
        for (x=0; x<conf_noc_height; x++) {
            minbd_context_t *r = nodes[y*conf_noc_width+x]->noc_context;
            print_flit(r->out_valid[NORTH], r->out_fc[NORTH]);
            printf("       ");
        }
        printf("\n");
        for (x=0; x<conf_noc_height; x++) {
            minbd_context_t *r = nodes[y*conf_noc_width+x]->noc_context;
            print_flit(r->out_valid[WEST], r->out_fc[WEST]);
            printf("(%c)", letter[y*conf_noc_width+x]);
            print_flit(r->out_valid[EAST], r->out_fc[EAST]);
            printf(" ");
        }
        printf("\n   ");
        for (x=0; x<conf_noc_height; x++) {
            minbd_context_t *r = nodes[y*conf_noc_width+x]->noc_context;
            print_flit(r->out_valid[SOUTH], r->out_fc[SOUTH]);
            printf("       ");
        }
        printf("\n");
    }
}




void minbd_print_stat()
{
    user_printf(
        "Maximum entries in reassembly buffer: %lu\n",
        stat_recvbuf_max);
}
