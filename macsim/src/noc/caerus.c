/*
 * caerus.c
 *
 *  Created on: 30.03.2015
 *      Author: gorlorom
 */

/*
 * Changes made:
 *
 * #1	A node now remembers all marked flits received and blocks 'send's accordingly. (same issue as paternoster)
 *
 *
 *  IMPORTANT: Allowing the corner-buffer to send if it passes a certain threshold can lead to other messages never reaching their destination.
 *	TODO Create .pdf elaborating the problem.
 *
 */
#include "caerus.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define COUNTER_THRESHOLD	3

extern node_t *nodes[];
// If a corner buffer has less than freeSlotThreshold slots it will send regardless of its state (cornerON)
uint_fast16_t freeSlotThreshold;


void caerus_updateInjectionQueue(node_t *node){
	caerus_context_t *gnode = ((caerus_context_t *)node->noc_context); //readability

	if(gnode->in_core.flit == NULL){
		gnode->in_core.flit = msgdeque(&gnode->injectionQueue);
	}
}

// signals the node west of the given node
void caerus_signalWesternNode(node_t *node){
	if(node->rank%conf_noc_width == 0){
		((caerus_context_t *)nodes[node->rank + conf_noc_width - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank + conf_noc_width - 1);
	}
	else{
		((caerus_context_t *)nodes[node->rank - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank - 1);
	}
}


bool caerus_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    flit_container_t *fc = malloc(sizeof(flit_container_t));
    fc->src = node->rank;
    fc->dest = dest;
    memcpy(fc->payload, &flit, FLIT_LEN);

    caerus_updateInjectionQueue(node);
    // try to enqueue the flit
    if (msgenque(&((caerus_context_t *)node->noc_context)->injectionQueue, fc)) {
        debug(log_intercon, "Node %lu: Flit for %lu queued\n", node->rank, dest);
        return true;
    }
    return false;
}


bool caerus_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container_t *fc;
    fc = msgdeque_rank(&((caerus_context_t *)node->noc_context)->core_buffer, src);
    if(fc == NULL) return false;
    memcpy(flit, fc->payload, FLIT_LEN);
    free(fc);
    info(log_intercon,"Core %lu received flit from %lu\n", node->rank, src);
    return true;
}


bool caerus_sender_ready(node_t *node){
	if(((caerus_context_t *)node->noc_context)->injectionQueue.slots != 0)
		return true;
	return false;
}

bool caerus_probe_rank(node_t *node, rank_t src){
	p_buffer_entry_t *entry = ((caerus_context_t *)node->noc_context)->core_buffer.head;
	while(entry != NULL){
		if(entry->flit->src == src){
			return true;
		}
		entry = entry->next;
	}
	return false;
}

rank_t caerus_probe_any(node_t *node){
	if(((caerus_context_t *)node->noc_context)->core_buffer.head != NULL){
		return ((caerus_context_t *)node->noc_context)->core_buffer.head->flit->src;
	}
	return (rank_t)-1;
}

// Simulate the transportation of the messages for WWABSTRACT_TRANSPORT_DELAY rounds
void caerus_route_one_cycle(node_t *node){
	caerus_context_t *cnode = ((caerus_context_t *)node->noc_context); //readability

	caerus_updateInjectionQueue(node);

	bool sent_core = false;
	bool sent_corner = false;

	if(node->rank == 0)
		debug(log_intercon,"\n\n---------------Cycle Start---------------\n");

//	if(node->cycle > 500)
//		exit(0);

	// south input
	if(cnode->in_south.flit != NULL){
		if(cnode->in_south.flit->dest == node->rank){
			if(!msgenque(&cnode->core_buffer,cnode->in_south.flit))
				fatal("Node %lu: core buffer overflow\n",node->rank);
			sent_core = true;
		}
		else{
			cnode->out_north.flit = cnode->in_south.flit;
		}
		cnode->in_south.flit = NULL;
	}

	// corner-buffer
	if(cnode->corner_buffer.slots != cnode->corner_buffer.size){

		debug(log_intercon,"Node %lu: corner-buffer state %d\n",node->rank, cnode->cornerON);

		if(cnode->cornerON || cnode->corner_buffer.slots < freeSlotThreshold || cnode->cornerCounter == 0){
			// corner-buffer to core
			if(cnode->corner_buffer.head->flit->dest == node->rank && !sent_core){
				flit_container_t *flit;
				flit = msgdeque(&cnode->corner_buffer);
				if(!msgenque(&cnode->core_buffer,flit)){
					debug(log_intercon,"Node %lu: core buffer overflow\n",node->rank);
				}
				else{
					debug(log_intercon,"Node %lu: corner-buffer to core\n",node->rank);
				}
			}
			// corner-buffer to north
			else if(cnode->out_north.flit == NULL){
				cnode->out_north.flit = msgdeque(&cnode->corner_buffer);
				debug(log_intercon,"Node %lu: corner-buffer to north \n",node->rank);
			}
		}

	}

	if(cnode->cornerCounter == COUNTER_THRESHOLD){
		cnode->cornerCounter = 0;
		cnode->cornerON ? (cnode->cornerON = false) : (cnode->cornerON = true);
	}
	else{
		cnode->cornerCounter = cnode->cornerCounter + 1;
	}


	// west input
	if(cnode->in_west.flit != NULL){

		// check for marked flits
		if(cnode->in_west.flit->h_marked && cnode->in_west.flit->dest%conf_noc_width != node->rank%conf_noc_width)
			cnode->hTimeouts[cnode->in_west.flit->dest&conf_noc_width] = conf_noc_width;

		// west to corner-buffer
		if(cnode->in_west.flit->dest%conf_noc_width == node->rank%conf_noc_width && cnode->hTimeouts[node->rank%conf_noc_width] == 0){
			if(!msgenque(&cnode->corner_buffer,cnode->in_west.flit)){
				cnode->out_east.flit = cnode->in_west.flit;
				cnode->out_east.flit->h_marked = true;
				cnode->hTimeouts[node->rank%conf_noc_width] = conf_noc_width;
				debug(log_intercon,"Node %lu: west had to be forwarded east and a horizontal block was triggered \n",node->rank);
			}
			else{
				sent_corner = true;
				debug(log_intercon,"Node %lu: west to corner-buffer\n",node->rank);
			}
		}
		// west to east
		else{
			cnode->out_east.flit = cnode->in_west.flit;
		}
		cnode->in_west.flit = NULL;
	}



	// core input
	if(cnode->in_core.flit != NULL){
		// core to corner-buffer
		if(cnode->in_core.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			if(!sent_corner && cnode->hTimeouts[node->rank%conf_noc_width] == 0 && cnode->corner_buffer.slots > 0){
				msgenque(&cnode->corner_buffer,cnode->in_core.flit);
				cnode->in_core.flit = NULL;
				debug(log_intercon,"Node %lu: core to corner-buffer\n",node->rank);
			}
			else if(sent_corner && cnode->hTimeouts[node->rank%conf_noc_width] == 0)
				cnode->signalWest = true;
		}
		// core to east
		else{
			if(cnode->out_east.flit == NULL && cnode->hTimeouts[cnode->in_core.flit->dest%conf_noc_width] == 0 && (!cnode->isSignaledEast || (cnode->isSignaledEast && !cnode->sentEast))){
				cnode->out_east.flit = cnode->in_core.flit;
				cnode->in_core.flit = NULL;
				cnode->sentEast = true;
				debug(log_intercon,"Node %lu: core to east\n",node->rank);
				// XXX Maybe also forward the signal?
			}
			else if(cnode->out_east.flit == NULL){
				cnode->sentEast = false;
			}
			else if(cnode->out_east.flit != NULL && (cnode->isSignaledEast || cnode->hTimeouts[cnode->in_core.flit->dest%conf_noc_width] == 0)){
				cnode->signalWest = true;
			}
		}
	}
	// free slot wasn't used
	else if(cnode->out_east.flit == NULL){
		cnode->sentEast = false;
	}
	// no free slot passed, but eastern node requests one -> forward signal
	else if(cnode->isSignaledEast)
		cnode->signalWest = true;

	cnode->isSignaledEast = false;

	// decrement timeouts
	int i;
	for(i=0;i<conf_noc_width;i++){
		if(cnode->hTimeouts[i] != 0)
			cnode->hTimeouts[i] = cnode->hTimeouts[i] - 1;
	}

	// last node updates the fields for the next cycle
	if(node->rank == conf_max_rank-1){
		rank_t x,y;
		// Update the input fields for next cycle
		for(x = 0; x < conf_max_rank; x++){

			// signal the other nodes
			if(((caerus_context_t *)nodes[x]->noc_context)->signalWest == true){
				caerus_signalWesternNode(nodes[x]);
				((caerus_context_t *)nodes[x]->noc_context)->signalWest = false;
			}

			// fetch flits coming from south
			((caerus_context_t *)nodes[x]->noc_context)->in_south.flit = ((caerus_context_t *)nodes[(x+conf_noc_width)%conf_max_rank]->noc_context)->out_north.flit;

			//fetch flits coming from west
			if(x%conf_noc_width == 0){			// if the node is the first in the line
				y = x + conf_noc_width - 1;
			}
			else{										// if it isn't
				y = nodes[x]->rank - 1;
			}
			((caerus_context_t *)nodes[x]->noc_context)->in_west.flit = ((caerus_context_t *)nodes[y]->noc_context)->out_east.flit;
		}

		// after all output fields have been copied to the input fields they now need to be cleared for the next cycle
		for(x = 0; x < conf_max_rank; x++){
			((caerus_context_t *)nodes[x]->noc_context)->out_north.flit = NULL;
			((caerus_context_t *)nodes[x]->noc_context)->out_east.flit = NULL;
		}
	}


}

// initialize the caerus interconnection simulator
void caerus_init(node_t *node)
{
    // set function pointers
    node->noc_send_flit         = caerus_send_flit;
    node->noc_recv_flit         = caerus_recv_flit;
		node->noc_sender_ready      = caerus_sender_ready;
		node->noc_probe_rank        = caerus_probe_rank;
		node->noc_probe_any         = caerus_probe_any;
		node->noc_route_one_cycle   = caerus_route_one_cycle;


		// add NoC specific fields to the node
		caerus_context_t *context = malloc(sizeof(caerus_context_t));
		if(context == NULL){
			fatal("Malloc failed to allocate memory\n");
		}
		node->noc_context = context;
		caerus_context_t *cnode = ((caerus_context_t *)node->noc_context); //readability

		cnode->hTimeouts = malloc(conf_noc_width * sizeof(uint_fast16_t));
		if(cnode->hTimeouts == NULL)
			fatal("Malloc failed to allocate memory\n");

		// initialize the just added variables
		int i;
		for(i=0;i<conf_noc_width;i++){
			cnode->hTimeouts[i] = 0;
		}

		cnode->out_north.flit = NULL;
		cnode->out_east.flit = NULL;
		cnode->in_south.flit = NULL;
		cnode->in_west.flit = NULL;
		cnode->in_core.flit = NULL;

		uint_fast32_t size;
		size = conf_noc_width;
		buffer_init(&cnode->injectionQueue,size);
		size = conf_noc_width;
		buffer_init(&cnode->corner_buffer,size);
		size = conf_noc_width;
		buffer_init(&cnode->core_buffer,128);

		freeSlotThreshold = conf_noc_width/2;
		cnode->cornerCounter = 0;
		(node->rank/conf_noc_width)%2 == 0 ? (cnode->cornerON = true) : (cnode->cornerON = false);

		cnode->signalWest = false;
		cnode->isSignaledEast = false;
		cnode->sentEast = false;

}

void caerus_destroy(node_t *node){
	free(((caerus_context_t *)node->noc_context)->hTimeouts);
	free(node->noc_context);
}



