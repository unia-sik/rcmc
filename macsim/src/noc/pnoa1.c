/*
 * gs_one_to_all.c
 *
 *  Created on: 16.03.2015
 *      Author: gorlorom
 */

/* XXX Is this behavior desirable
 * Note: A core can inject a flit as soon as the injection queue is empty thus a node can possibly send more than 1 flit per cycle, which poses no problem though
 * Note: The cycles overlap (meaning: west->corner-buffer, core->east and corner-buffer->north, south->core-buffer might occur simultaneously) this reduces the
 * time for a full phase to  a n *(n-1)-1 rhythm.
 *
 * XXX Is there a line from west to the core-buffer, and can the core-buffer receive a flit from west and south?
 * Currently: YES
 *
 */

#include "pnoa1.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


extern node_t *nodes[];
uint_fast32_t	cycle;


void pnoa1_updateInjectionQueue(node_t *node){
	pnoa1_context_t *gnode = ((pnoa1_context_t *)node->noc_context); //readability

	if(gnode->in_core.flit == NULL){
		gnode->in_core.flit = msgdeque(&gnode->injectionQueue);
	}
}

bool pnoa1_send_flit_internal(node_t *node, rank_t dest, flit_t flit, bool marked)
{
    pnoa1_updateInjectionQueue(node);

    // try to enqueue the flit
    flit_container_t *fc = malloc(sizeof(flit_container_t));
    fc->src = node->rank;
    fc->dest = dest;
    fc->h_marked = marked;
    memcpy(fc->payload, &flit, FLIT_LEN);
    if (msgenque(&((pnoa1_context_t *)node->noc_context)->injectionQueue, fc)) {
        debug(log_intercon,"Node %lu: Flit for %lu queued\n",node->rank, dest);
        return true;
    }
    free(fc);
    return false;
}

bool pnoa1_send_flit(node_t *node, rank_t dest, flit_t flit)
{
	return pnoa1_send_flit_internal(node, dest, flit, false);
}

bool pnoa1_send_ready(node_t *node, rank_t dest)
{
	return pnoa1_send_flit_internal(node, dest, 0x987, true);
}

// checks if dest is marked as ready; returns true, when it is ready and false when we have to wait
bool pnoa1_dest_ready(node_t *node, rank_t dest)
{
	uint64_min_t current_state = ((pnoa1_context_t *)node->noc_context)->ready_state[(dest & 0x3F) >> 6];
	if ((current_state & (1 << dest)) == 0) // dest is not yet ready
		return false;
	else { // dest is ready
		((pnoa1_context_t *)node->noc_context)->ready_state[(dest & 0x3F) >> 6] &= ~(1 << dest);
		return true;
	}
}

bool pnoa1_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container_t *fc = msgdeque_rank(
        &((pnoa1_context_t *)node->noc_context)->core_buffer,
        src);
    if(fc == NULL) return false; // no flit from src found in buffer
    memcpy(flit, fc->payload, FLIT_LEN);
    free(fc);
    info(log_intercon,"Core %lu received flit in cycle %ld \n", 
        node->rank, node->cycle);
    return true;
}



bool pnoa1_sender_ready(node_t *node)
{
	if(((pnoa1_context_t *)node->noc_context)->injectionQueue.slots != 0)
		return true;
	return false;
}

bool pnoa1_probe_rank(node_t *node, rank_t src)
{
	p_buffer_entry_t *entry = ((pnoa1_context_t *)node->noc_context)->core_buffer.head;
	while(entry != NULL){
		if(entry->flit->src == src){
			return true;
		}
		entry = entry->next;
	}
	return false;
}

rank_t pnoa1_probe_any(node_t *node)
{
	if(((pnoa1_context_t *)node->noc_context)->core_buffer.head != NULL){
		return ((pnoa1_context_t *)node->noc_context)->core_buffer.head->flit->src;
	}
	return (rank_t)-1;
}

// Simulate the transportation of the messages for one cycle
void pnoa1_route_one_cycle(node_t *node)
{
	pnoa1_context_t *gnode = ((pnoa1_context_t *)node->noc_context); //readability

	pnoa1_updateInjectionQueue(node);

//	if(node->cycle >=500)
//		exit(0);

	// reset round after the last one finished
	if(gnode->currentCycle == gnode->roundTime){
		gnode->currentCycle = 0;
		if(node->rank == 0)			// only used for debug output
			cycle++;
	}

	if(node->rank == 0)
		debug(log_intercon,"\nNoC Cycle: %lu	Schedule cycle: %lu\n",cycle, gnode->currentCycle);

	// process flit in local_in
	if(gnode->in_core.flit != NULL){
		if(gnode->in_core.flit->dest % conf_noc_width == node->rank % conf_noc_width && gnode->currentCycle == 0){
			gnode->corner_buffer.flit = gnode->in_core.flit;
			gnode->in_core.flit = NULL;
			info(log_intercon,"Node %lu: Core to corner-buffer \n",node->rank);
		}
		else{
			unsigned distance;
			if(gnode->in_core.flit->dest%conf_noc_width <= node->rank%conf_noc_width){
				distance = conf_noc_width + gnode->in_core.flit->dest%conf_noc_width - node->rank%conf_noc_width;
			}
			else{
				distance = gnode->in_core.flit->dest%conf_noc_width - node->rank%conf_noc_width;
			}
			if(distance*(conf_noc_width - 2) == gnode->currentCycle && distance < conf_noc_width){
				gnode->out_east.flit = gnode->in_core.flit;
				gnode->in_core.flit = NULL;
				info(log_intercon,"Node %lu: Core to east\n",node->rank);
			}
		}
	}


	// inject flits in the vertical ring
	if(gnode->corner_buffer.flit != NULL){
		// 4 cores (2x2) is a special case and needs to be handled with a separate condition
		if(gnode->currentCycle%(conf_noc_width - 1) == 1 || (conf_noc_width == 2 && gnode->currentCycle != 0)){
			gnode->out_north.flit = gnode->corner_buffer.flit;
			gnode->corner_buffer.flit = NULL;
			debug(log_intercon,"Node %lu: corner-buffer to north\n",node->rank);
		}
	}


	// process flits coming from west
	if(gnode->in_west.flit != NULL){
		if(gnode->in_west.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			if(gnode->in_west.flit->dest == node->rank){
				if(gnode->in_west.flit->h_marked) { // is READY flit
					gnode->ready_state[(gnode->in_west.flit->src & 0x3F) >> 6] |= 1 << ((gnode->in_west.flit->src & 0x3F));
					debug(log_intercon,"Node %lu: ready flit from west from %lu. New ready_state: %lu\n",
						node->rank,gnode->in_west.flit->src, gnode->ready_state[(gnode->in_west.flit->src & 0x3F) >> 6]);
				} else { // is DATA flit
					if(!msgenque(&gnode->core_buffer,gnode->in_west.flit))
						warning(log_intercon,"Node %lu: core buffer overflow\n",node->rank);
					debug(log_intercon,"Node %lu: west to core_buffer. Msg from %lu\n",node->rank,gnode->in_west.flit->src);
				}
			}
			else{
			gnode->corner_buffer.flit = gnode->in_west.flit;
			debug(log_intercon,"Node %lu: west to corner-buffer\n",node->rank);
			}
		}
		else{
			gnode->out_east.flit = gnode->in_west.flit;
			debug(log_intercon,"Node %lu: west to east\n",node->rank);
		}

	}


	// process flits coming from south
	if(gnode->in_south.flit != NULL){
		if(gnode->in_south.flit->dest == node->rank){
			if(gnode->in_south.flit->h_marked) { // is READY flit
				gnode->ready_state[(gnode->in_south.flit->src & 0x3F) >> 6] |= 1 << ((gnode->in_south.flit->src) & 0x3F);
				debug(log_intercon,"Node %lu: ready flit from south from %lu. New ready_state: %lu\n",
					node->rank,gnode->in_south.flit->src, gnode->ready_state[(gnode->in_south.flit->src & 0x3F) >> 6]);
			} else { // is DATA flit
				if(!msgenque(&gnode->core_buffer,gnode->in_south.flit))
					warning(log_intercon,"Node %lu: core buffer overflow\n",node->rank);
				debug(log_intercon,"Node %lu: south to core_buffer. Msg from %lu\n",node->rank,gnode->in_south.flit->src);
			}
		}
		else{
			gnode->out_north.flit = gnode->in_south.flit;
			debug(log_intercon,"Node %lu: south to north\n",node->rank);
		}
	}


	// clear input fields
	gnode->in_south.flit = NULL;
	gnode->in_west.flit = NULL;

	// increment cycleCount
	gnode->currentCycle = gnode->currentCycle + 1;

	// last nodes fetches copies new values to corresponding fields
	if(node->rank == conf_max_rank - 1){
		rank_t x,y;
		for(x = 0; x < conf_max_rank; x++){	// for each node
			// Update the input fields for next cycle
			// fetch flits coming from south
			((pnoa1_context_t *)nodes[x]->noc_context)->in_south = ((pnoa1_context_t *)nodes[(x+conf_noc_width)%conf_max_rank]->noc_context)->out_north;

			//fetch flits coming from west
			if(x%conf_noc_width == 0){			// if the node is the first in the line
				y = x + conf_noc_width - 1;
			} else{										// if it isn't
				y = nodes[x]->rank - 1;
			}
			((pnoa1_context_t *)nodes[x]->noc_context)->in_west.flit = ((pnoa1_context_t *)nodes[y]->noc_context)->out_east.flit;
		}

		for(x = 0; x < conf_max_rank; x++){
			((pnoa1_context_t *)nodes[x]->noc_context)->out_north.flit = NULL;
			((pnoa1_context_t *)nodes[x]->noc_context)->out_east.flit = NULL;
		}

		debug(log_intercon,"Schedule cycle End \n");
	}

}

// Init the gs_one_to_all(NoC) simulator
void pnoa1_init(node_t *node)
{
    node->noc_send_flit         = pnoa1_send_flit;
    node->noc_send_ready        = pnoa1_send_ready;
    node->noc_dest_ready        = pnoa1_dest_ready;
    node->noc_recv_flit         = pnoa1_recv_flit;

	node->noc_sender_ready      = pnoa1_sender_ready;
	node->noc_probe_rank        = pnoa1_probe_rank;
	node->noc_probe_any         = pnoa1_probe_any;
	node->noc_route_one_cycle   = pnoa1_route_one_cycle;

	// add NoC specific fields to the node
	pnoa1_context_t *context = malloc(sizeof(pnoa1_context_t));
	if(context == NULL){
		fatal("Malloc failed to allocate memory\n");
	}
	node->noc_context = context;

	// initialize the just added fields
	((pnoa1_context_t *)node->noc_context)->in_south.flit = NULL;
	((pnoa1_context_t *)node->noc_context)->in_west.flit = NULL;
	((pnoa1_context_t *)node->noc_context)->in_core.flit = NULL;
	((pnoa1_context_t *)node->noc_context)->out_east.flit = NULL;
	((pnoa1_context_t *)node->noc_context)->out_north.flit = NULL;
	((pnoa1_context_t *)node->noc_context)->corner_buffer.flit = NULL;

	((pnoa1_context_t *)node->noc_context)->roundTime = conf_noc_width*(conf_noc_width-1)+1;
	((pnoa1_context_t *)node->noc_context)->currentCycle = 0;

	uint_fast32_t size;
	// This sets the core buffer size
	size = 2 * conf_noc_width*conf_noc_width;
	buffer_init(&((pnoa1_context_t *)node->noc_context)->core_buffer, 80);
	// This sets the injectionQueue size (+1)
	size = conf_noc_width - 1;
	buffer_init(&((pnoa1_context_t *)node->noc_context)->injectionQueue, size);

	for (int i=0; i< MAX_RANK / 64; i++)
		((pnoa1_context_t *)node->noc_context)->ready_state[i] = 0;

	cycle = 0;

}

void pnoa1_destroy(node_t *node){
	free(node->noc_context);
}




