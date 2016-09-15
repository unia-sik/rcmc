/**
 * riscv.c
 * Core that sends and receives synthetic traffic
 *
 * MacSim project
 *
 */

#include "traffic.h"
#include <stdlib.h>

//#define conf_flits_per_packet 4


rank_t bitrev(rank_t v, rank_t bits)
{
    rank_t r=0;
    while (bits>0) {
        r = (r << 1) | (v & 1);
        v = v >> 1;
        bits--;
    }
    return r;
}



// Simulate one cycle
instruction_class_t traffic_one_cycle(node_t *node)
{
    rank_t i = node->rank;

    // check for injection once per cycle
//    if (node_rand64(node) < conf_inj_prob) {
    uint64_t rand = node_rand64(node);
    if (rand < conf_inj_prob) {
        unsigned dest;
        switch (node->core.traffic.type) {

            case TRAFFIC_UNIFORM: // U
                do {
                    dest = node_randmax(node, conf_max_rank);
                } while (dest==i);
                break;

            case TRAFFIC_BITCOMP:
		dest = (~i) % conf_max_rank;
		break;
		
	    case TRAFFIC_TRANSPOSE:
		dest = (i%conf_noc_width)*conf_noc_width + (i/conf_noc_width);
		break;
		
	    case TRAFFIC_BITREV: // R
		dest = bitrev(i, node->core.traffic.nodebits);
		break;
	
	    case TRAFFIC_SHUFFLE: // S
		//dest_i = ((i<<1) % conf_max_rank) | ((i>>(node->traffic.nodebits-1)) & 1);
		dest = (i<<1) - (i >= conf_max_rank/2 ? conf_max_rank-1 : 0);
		break;

	    case TRAFFIC_TORNADO: // O
		dest = (((i%conf_noc_width) + ((conf_noc_width/2)-1)) % conf_noc_width)
		    + ((((i/conf_noc_width) + ((conf_noc_height/2)-1)) % conf_noc_height)*conf_noc_width);
		break;

	    case TRAFFIC_NEIGHBOR: // N
		dest = (((i%conf_noc_width) + 1) % conf_noc_width)
		    + ((((i/conf_noc_width) + 1) % conf_noc_height)*conf_noc_width);
		break;
	
	    case TRAFFIC_UPPERLEFT:
		// worst case for PaterNoster
		dest = (((i%conf_noc_width) - 1) % conf_noc_width)
		    + ((((i/conf_noc_width) - 1) % conf_noc_height)*conf_noc_width);
		break;

            default:
                fatal("Unknown traffic pattern");
        }

//printf("%lu<%lu %lu->%u\n", rand, conf_inj_prob, i, dest);
            // send directly or add to queue
            if (!node->noc_send_flit(node, dest, node->cycle)) {
                injqueue_entry_t *n = fatal_malloc(sizeof(injqueue_entry_t));
                n->next = 0;
                n->dest = dest;
                n->flit = node->cycle;
                if (node->core.traffic.injqueue_tail!=0) {
                    node->core.traffic.injqueue_tail->next = n;
                    node->core.traffic.injqueue_tail = n;
                } else {
                    node->core.traffic.injqueue_tail = node->core.traffic.injqueue_head = n;
                }
        }
    } else {
        // try to send if queue is not empty
        if (node->core.traffic.injqueue_head!=0
            && node->noc_send_flit(node, 
                node->core.traffic.injqueue_head->dest,
                node->core.traffic.injqueue_head->flit))
        {
            // remove from queue if send was successful
            injqueue_entry_t *h = node->core.traffic.injqueue_head;
            injqueue_entry_t *n = h->next;
            node->core.traffic.injqueue_head = n;
            if (n==0) node->core.traffic.injqueue_tail = 0;
            free(h);
        }
    }



    rank_t src = node->noc_probe_any(node);
    if (src>=0) {
        flit_t f;
        node->noc_recv_flit(node, src, &f);
        node->core.traffic.stat_flit_count++;
        node->core.traffic.stat_total_latency += node->cycle - f;
//printf(" %lu->%lu$%lu\n", src, i, node->cycle-f);
    }



    return 1; // one cycle
}


// init context
void traffic_init_context(node_t *node)
{
    node->cycle = 0;
    node->retired = 0;
    node->state = CS_RUNNING;
    node->pc = 0;
    node->core_type = CT_traffic;
    node->one_cycle = &traffic_one_cycle;

    node->core.traffic.injqueue_head = 0;
    node->core.traffic.injqueue_tail = 0;
    node->core.traffic.stat_flit_count = 0;
    node->core.traffic.stat_total_latency = 0;

    switch (node->core.traffic.type) {
        case TRAFFIC_UNIFORM:
        case TRAFFIC_TRANSPOSE:
        case TRAFFIC_SHUFFLE:
        case TRAFFIC_TORNADO:
        case TRAFFIC_NEIGHBOR:
        case TRAFFIC_UPPERLEFT:
            break;

        case TRAFFIC_BITCOMP:
        case TRAFFIC_BITREV:
            if ((conf_max_rank&(conf_max_rank-1))!=0)
                fatal("Number of cores must be a power of 2");
            node->core.traffic.nodebits = __builtin_ffsl(conf_max_rank)-1;
            break;
        default:
            fatal("Unknown traffic pattern '%c'", node->core.traffic.type);
    }
}



void traffic_finish_context(node_t *node)
{
    // remove linked list of flits to send
    injqueue_entry_t *p=node->core.traffic.injqueue_head;
    while (p) {
        injqueue_entry_t *n = p->next;
        free(p);
        p = n;
    }
}



void traffic_print_context(node_t *node)
{
/*
    printf("%lu flits injected total latency %lu average %g\n",
        node->core.traffic.stat_flit_count,
        node->core.traffic.stat_total_latency,
        (double)node->core.traffic.stat_total_latency /
        (double)node->core.traffic.stat_flit_count);
*/
}

