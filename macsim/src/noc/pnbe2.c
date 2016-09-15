/*
 * paternoster_best_effort.c
 *
 *  Created on: 02.04.2015
 *      Author: gorlorom
 */

/*
 * Check /doc/Changes.txt for some additional information
 */

#include "pnbe2.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern node_t *nodes[];



bool pnbe2_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    pnbe2_context_t *pnode = 
        ((pnbe2_context_t *)node->noc_context);

    // update injection queues
    // this is necessary since core and NoC may run at different speeds
    if (pnode->in_core.flit == NULL)
        pnode->in_core.flit = msgdeque(&pnode->injectionQueue);

    // flit couln't be sent yet, injection queue is still full
    if(pnode->injectionQueue.slots == 0) {
        debug(log_intercon,"Node %lu injectionQueue full, "
            "flit couln't be enqueued\n",node->rank);
        return false;
    }

    flit_container_t *fc = malloc(sizeof(flit_container_t));
    fc->src = node->rank;
    fc->dest = dest;
    memcpy(fc->payload, &flit, FLIT_LEN);
    msgenque(&pnode->injectionQueue, fc);
    info(log_intercon,"Node %lu queues flit for %lu in cycle: %ld \n",
        node->rank, dest, node->cycle);
    return true;
}


bool pnbe2_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container_t *fc = msgdeque_rank(
        &((pnbe2_context_t *)node->noc_context)->core_buffer,
        src);
    if(fc == NULL) return false; // no flit from src found in buffer
    memcpy(flit, fc->payload, FLIT_LEN);
    free(fc);
    info(log_intercon,"Core %lu received flit in cycle %ld \n", 
        node->rank, node->cycle);
    return true;
}


void pnbe2_updateInjectionQueue(node_t *node){
	pnbe2_context_t *pnode = ((pnbe2_context_t *)node->noc_context);
	if(pnode->in_core.flit == NULL){
		pnode->in_core.flit = msgdeque(&pnode->injectionQueue);
	}
}

// signals the node west of the given node
void pnbe2_signalWesternNode(node_t *node){
	if(node->rank%conf_noc_width == 0){
		((pnbe2_context_t *)nodes[node->rank + conf_noc_width - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank + conf_noc_width - 1);
	}
	else{
		((pnbe2_context_t *)nodes[node->rank - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank - 1);
	}
}

// signals the node south of the given node
void pnbe2_signalSouthernNode(node_t *node){
	if(node->rank > conf_max_rank - conf_noc_width - 1){
		((pnbe2_context_t *)nodes[node->rank % conf_noc_width]->noc_context)->isSignaledNorth = true;
		debug(log_intercon,"Node %lu signaling %lu\n",node->rank, node->rank % conf_noc_width);
	}
	else{
		((pnbe2_context_t *)nodes[node->rank + conf_noc_width]->noc_context)->isSignaledNorth = true;
		debug(log_intercon,"Node %lu signaling %lu\n",node->rank, node->rank + conf_noc_width);
	}
}


bool pnbe2_sender_ready(node_t *node){
	pnbe2_updateInjectionQueue(node);
	if(((pnbe2_context_t *)node->noc_context)->injectionQueue.slots != 0)
		return true;
	return false;
}

bool pnbe2_probe_rank(node_t *node, rank_t src){
	p_buffer_entry_t *entry = ((pnbe2_context_t *)node->noc_context)->core_buffer.head;
	while(entry != NULL){
		if(entry->flit->src == src){
			return true;
		}
		entry = entry->next;
	}
	return false;
}

rank_t pnbe2_probe_any(node_t *node){
	if(((pnbe2_context_t *)node->noc_context)->core_buffer.head != NULL){
			return ((pnbe2_context_t *)node->noc_context)->core_buffer.head->flit->src;
		}
		return (rank_t)-1;
}

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void pnbe2_route_one_cycle(node_t *node){
	pnbe2_context_t *pnode = ((pnbe2_context_t *)node->noc_context); //readability

	bool out_core = false;
	bool out_corner = false;

	if(node->rank == 0)
		debug(log_intercon,"\n\n---------------Cycle Start---------------\n");

	// update injectionQueue
	pnbe2_updateInjectionQueue(node);

//	if(node->cycle > 1000)
//		fatal("STOP\n");

	// south input
	if(pnode->in_south.flit != NULL){
		// south to core
		if(pnode->in_south.flit->dest == node->rank){
			if(!msgenque(&pnode->core_buffer,pnode->in_south.flit)){
				fatal("Node %lu: core-buffer overflow\n",node->rank);
			}
			debug(log_intercon,"Node %lu: south to core-buffer\n",node->rank);
			out_core = true;
		}
		// south to north
		else{
			pnode->out_north.flit = pnode->in_south.flit;
			debug(log_intercon,"Node %lu: south to north\n",node->rank);
		}
		pnode->in_south.flit = NULL;
	}

	// corner-buffer
	if(pnode->corner_buffer.slots != pnode->corner_buffer.size){
		// corner-buffer to north
		if(pnode->out_north.flit == NULL && (!pnode->isSignaledNorth || (pnode->isSignaledNorth && !pnode->sentNorth))){
			pnode->out_north.flit = msgdeque(&pnode->corner_buffer);
			debug(log_intercon,"Node %lu: corner-buffer to north\n",node->rank);
			pnode->sentNorth = true;
			// XXX perhaps add signaling here as well
		}
		// free slot + signaled + already used up the last free slot
		else if(pnode->out_north.flit == NULL && pnode->isSignaledNorth){
			pnode->sentNorth = false;
		}
		// no free slot
		else{
			pnode->signalSouth = true;
		}
	}
	// free slot passed by
	else if(pnode->out_north.flit == NULL){
		pnode->sentNorth = false;
	}
	// no free slot + signaled -> forward signal
	else if(pnode->isSignaledNorth){
		pnode->signalSouth = true;
	}

	// west input
	if(pnode->in_west.flit != NULL){

		// check for own flits
		if(pnode->in_west.flit->src == node->rank)
			pnode->sendBlock = conf_noc_width;
		// check for marked flits
		if(pnode->in_west.flit->h_marked && pnode->in_west.flit->dest%conf_noc_width != node->rank%conf_noc_width){
			pnode->h_timers[pnode->in_west.flit->dest%conf_noc_width] = conf_noc_width;
		}

		// west to core
		if(pnode->in_west.flit->dest == node->rank){
			// local-out already occupied by south
			if(out_core && pnode->receive_timeout == 0){
				pnode->out_east.flit = pnode->in_west.flit;
				debug(log_intercon,"Node %lu: west to east (core-buffer coflict)\n",node->rank);
				pnode->receive_timeout = conf_noc_width;
			}
			// receive timeout in effect
			else if(pnode->receive_timeout != 0){
				pnode->out_east.flit = pnode->in_west.flit;
				debug(log_intercon,"Node %lu: west to east, receive block active\n",node->rank);
			}
			// west to local-out
			else{
				if(!msgenque(&pnode->core_buffer,pnode->in_west.flit)){
					fatal("Node %lu: core-buffer overflow\n",node->rank);
				}
				debug(log_intercon,"Node %lu: west to core\n",node->rank);
			}
		}
		// west to corner-buffer
		else if(pnode->in_west.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			// horizontal block active
			if(pnode->h_timers[node->rank%conf_noc_width] != 0){
				pnode->out_east.flit = pnode->in_west.flit;
				debug(log_intercon,"Node %lu: west to east, horizontal block active\n",node->rank);
			}
			// no horizontal block active
			else{
				if(!msgenque(&pnode->corner_buffer,pnode->in_west.flit)){
					pnode->out_east.flit = pnode->in_west.flit;
					pnode->out_east.flit->h_marked = true;
					debug(log_intercon,"Node %lu: west to east, flit marked\n",node->rank);
					pnode->h_timers[node->rank%conf_noc_width] = conf_noc_width;
				}
				debug(log_intercon,"Node %lu: west to corner-buffer\n",node->rank);
				out_corner = true;
			}
		}
		// west to east
		else{
			pnode->out_east.flit = pnode->in_west.flit;
			debug(log_intercon,"Node %lu: west to east\n",node->rank);
		}
		pnode->in_west.flit = NULL;
	}

	// core input
	if(pnode->in_core.flit != NULL){
		// core to corner-buffer
		if(pnode->in_core.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			if(out_corner){
				pnode->signalWest = true;
				// XXX most effective way? Some sort of signaling is definitely necessary
			}
			else if(pnode->h_timers[node->rank%conf_noc_width] == 0 && msgenque(&pnode->corner_buffer,pnode->in_core.flit)){
				debug(log_intercon,"Node %lu: core to corner-buffer\n",node->rank);
				pnode->in_core.flit = NULL;
			}
		}
		// core to east
		else{
			// is allowed to send
			if(pnode->out_east.flit == NULL && pnode->sendBlock == 0 && pnode->h_timers[pnode->in_core.flit->dest%conf_noc_width] == 0 && (!pnode->isSignaledEast || (pnode->isSignaledEast && !pnode->sentEast))){
				pnode->out_east.flit = pnode->in_core.flit;
				debug(log_intercon,"Node %lu: core to east\n",node->rank);
				pnode->sentEast = true;
				pnode->in_core.flit = NULL;
				// XXX signaling possible
			}
			// free slot + signaled + already used up last free slot
			else if(pnode->out_east.flit == NULL){
				pnode->sentEast = false;
			}
			// signal west, if no free slot was available and the node was either signaled or was ready to inject a flit
			else if(pnode->out_east.flit != NULL && (pnode->isSignaledEast || (pnode->sendBlock == 0 && pnode->h_timers[pnode->in_core.flit->dest%conf_noc_width] == 0))){
				pnode->signalWest = true;
			}
		}
	}
	// free horizontal slot passed
	else if(pnode->out_east.flit == NULL){
		pnode->sentEast = false;
	}
	// signaled + slot was occupied -> signal West
	else if(pnode->isSignaledEast){
		pnode->signalWest = true;
	}

	// reset signaled status
	pnode->isSignaledEast = false;
	pnode->isSignaledNorth = false;

	// decrement timeouts
	int i;
	for(i=0;i<conf_noc_width;i++){
		if(pnode->h_timers[i] != 0)
			pnode->h_timers[i] = pnode->h_timers[i] - 1;
	}

	if(pnode->receive_timeout != 0)
		pnode->receive_timeout = pnode->receive_timeout - 1;

	if(pnode->sendBlock != 0)
		pnode->sendBlock = pnode->sendBlock - 1;

	// last node: updates input/output fields for next cycle and does the signaling
	if(node->rank == conf_max_rank - 1){
		rank_t x,y;
		for(x = 0; x < conf_max_rank; x++){	// for each node

			// signal the other nodes
			if(((pnbe2_context_t *)nodes[x]->noc_context)->signalWest == true){
				pnbe2_signalWesternNode(nodes[x]);
				((pnbe2_context_t *)nodes[x]->noc_context)->signalWest = false;
			}

			if(((pnbe2_context_t *)nodes[x]->noc_context)->signalSouth == true){
				pnbe2_signalSouthernNode(nodes[x]);
				((pnbe2_context_t *)nodes[x]->noc_context)->signalSouth = false;
			}

			// Update the input fields for next cycle
			// fetch flits coming from south
			((pnbe2_context_t *)nodes[x]->noc_context)->in_south.flit = ((pnbe2_context_t *)nodes[(x+conf_noc_width)%conf_max_rank]->noc_context)->out_north.flit;

			//fetch flits coming from west
			if(x%conf_noc_width == 0){			// if the node is the first in the line
				y = x + conf_noc_width - 1;
			}
			else{										// if it isn't
			y = nodes[x]->rank - 1;
			}
			((pnbe2_context_t *)nodes[x]->noc_context)->in_west.flit = ((pnbe2_context_t *)nodes[y]->noc_context)->out_east.flit;
		}

		for(x = 0; x < conf_max_rank; x++){
			((pnbe2_context_t *)nodes[x]->noc_context)->out_north.flit = NULL;
			((pnbe2_context_t *)nodes[x]->noc_context)->out_east.flit = NULL;
		}
	}

}

// initialize the best effort paternoster interconnection simulator
void pnbe2_init(node_t *node)
{
    // set function pointers
    node->noc_send_flit         = pnbe2_send_flit;
    node->noc_recv_flit         = pnbe2_recv_flit;
		node->noc_sender_ready      = pnbe2_sender_ready;
		node->noc_probe_rank        = pnbe2_probe_rank;
		node->noc_probe_any         = pnbe2_probe_any;
		node->noc_route_one_cycle   = pnbe2_route_one_cycle;

		// add NoC specific fields to the node
		pnbe2_context_t *context = malloc(sizeof(pnbe2_context_t));
		if(context == NULL){
			fatal("Malloc failed to allocate memory.\n");
		}
		node->noc_context = context;

		pnbe2_context_t *pnode = ((pnbe2_context_t *)node->noc_context); //readability

		// initialize the added variables
		pnode->out_east.flit = NULL;
		pnode->out_north.flit = NULL;
		pnode->in_west.flit = NULL;
		pnode->in_south.flit = NULL;
		pnode->in_core.flit = NULL;

		pnode->sentNorth = false;
		pnode->sentEast = false;
		pnode->signalSouth = false;
		pnode->signalWest = false;
		pnode->isSignaledEast = false;
		pnode->isSignaledNorth = false;

		pnode->sendBlock = 0;
		pnode->receive_timeout = 0;

		pnode->h_timers = malloc(conf_noc_width * sizeof(uint_fast16_t));
		if(pnode->h_timers == NULL)
			fatal("Malloc failed to allocate memory.\n");

		uint_fast16_t size;
		for(size = 0; size < conf_noc_width; size ++){
			pnode->h_timers[size] = 0;
		}

		// set injectionQueue size
		size = conf_noc_width - 1;
		buffer_init(&pnode->injectionQueue, size);
		// set corner_buffer size
		size = 8;
		buffer_init(&pnode->corner_buffer, size);
		// set core-buffer size
		size = conf_noc_width;
		buffer_init(&pnode->core_buffer, 128);
		    // mpi_alltoall: 2 are enough
		    // mpi_alltoall: 8
		    // ocean: 64
		    // mpi_example: 128

}

void pnbe2_destroy(node_t *node){
	free(((pnbe2_context_t *)node->noc_context)->h_timers);
	free(node->noc_context);
}
