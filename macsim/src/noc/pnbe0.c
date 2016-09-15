/*
 * paternoster_back_up.c
 *
 *  Created on: 24.02.2015
 *      Author: gorlorom
 */

/* XXX Important: There is a better version available -> paternoster_best_effort.c
 * This version is deprecated.
 *
 * This is a slightly modified version of the paternoster router as described in the paper "Low Power Flitwise Routing in an Unidirectional Torus
 * with Minimal Buffering" by Jörg Mische.
 *
 * Changes made:
 * #1
 * - a node now remembers all marked flits and blocks if a flit is to be sent, while its target column is blocked, unless the target node is also
 * in the same row.
 * reason: the correct order of flits could not be guaranteed otherwise
 *
 * #2
 * - a node now may receive flits addressed to it even if a horizontal block, due to a full corner-buffer, is in effect.
 * reason: there is no reason for these flits to clog up the NoC, since they don't jeopardize the correct order.
 *
 * #3
 * - a node will no longer forward flits from the core to the corner-buffer if a horizontal-block is effect at this node.
 * reason: this change is made in order to prevent flits from spending to much time circling from west to east.
 *
 * #4
 * - a node now blocks for a round after having to send a flit coming from the western input, which wants to the local output, to the eastern output
 * reason: the correct order of flits is not preserved otherwise
 *
 * #5
 * - in accordance to #1 and #4 a node will now, after receiving a flit sent by itself, remember this for a round and will not send a flit targeting the same row
 * during this time.
 * reason: other blocks are not necessary due to the changes in #1
 *
 * Additional notes:
 * - a corner-buffer may enqueue a flit even though its 'full', if a slot becomes available in the same cycle (see delayedEnqueue)
 * - a flit cannot be dequeued from a corner buffer the same cycle its been enqueued
 * - a corner-buffer can only enqueue a flit from west OR the core (only one flit per cycle)
 */

#include "pnbe0.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>


extern node_t *nodes[];

// clear the message field
void msg_field_clear_back_up(msg_field_t *field){
	field->flit = NULL;
}

// signals the node west of the given node
void signalWesternNode(node_t *node){
	if(node->rank%conf_noc_width == 0){
		((pnbe0_context_t *)nodes[node->rank + conf_noc_width - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank + conf_noc_width - 1);
	}
	else{
		((pnbe0_context_t *)nodes[node->rank - 1]->noc_context)->isSignaledEast = true;
		debug(log_intercon,"Node %lu signaling %lu\n", node->rank, node->rank - 1);
	}
}

// signals the node south of the given node
void signalSouthernNode(node_t *node){
	if(node->rank > conf_max_rank - conf_noc_width - 1){
		((pnbe0_context_t *)nodes[node->rank % conf_noc_width]->noc_context)->isSignaledNorth = true;
		debug(log_intercon,"Node %lu signaling %lu\n",node->rank, node->rank % conf_noc_width);
	}
	else{
		((pnbe0_context_t *)nodes[node->rank + conf_noc_width]->noc_context)->isSignaledNorth = true;
		debug(log_intercon,"Node %lu signaling %lu\n",node->rank, node->rank + conf_noc_width);
	}
}


// basically an extension of send_packet since a flit is a packet
bool pnbe0_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    pnbe0_context_t *pnode = ((pnbe0_context_t *)node->noc_context); //readability

    // this is necessary since core and NoC may run at different speeds
    if(pnode->paternoster_in_core.flit == NULL){
        pnode->paternoster_in_core.flit = msgdeque(&pnode->injectionQueue);
    }

    // flit couln't be sent yet, injection queue is still full
    if(pnode->injectionQueue.slots == 0) return false;

    flit_container_t *fc = malloc(sizeof(flit_container_t));
    fc->src = node->rank;
    fc->dest = dest;
    memcpy(fc->payload, &flit, FLIT_LEN);

    msgenque(&pnode->injectionQueue, fc);
    info(log_intercon,"Node %lu queues flit for %lu in cycle: %ld \n",
        node->rank, dest, node->cycle);
    return true;
}


// basically an extension of receive_packet since a flit is a packet
bool pnbe0_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
// printf("Core %lu cheking for flit\n",node->rank);
    flit_container_t *fc = msgdeque_rank(&((pnbe0_context_t *)node->noc_context)->core_buffer, src);
    if(fc == NULL){		// no flit from src found in buffer
        return false;
    }
    memcpy(flit, fc->payload, FLIT_LEN);
    free(fc);
    info(log_intercon,"Core %lu received flit in cycle %ld \n",node->rank,node->cycle);
    return true;
}


bool pnbe0_sender_ready(node_t *node)
{
	pnbe0_context_t *pnode = ((pnbe0_context_t *)node->noc_context); //readability

	// prepare next flit in injectionQueue
	if(pnode->paternoster_in_core.flit == NULL){
		pnode->paternoster_in_core.flit = msgdeque(&pnode->injectionQueue);
	}

	if(pnode->injectionQueue.slots != 0)
		return true;
	return false;
}

bool pnbe0_probe_rank(node_t *node, rank_t src)
{
	p_buffer_entry_t *entry = ((pnbe0_context_t *)node->noc_context)->core_buffer.head;
	while(entry != NULL){
		if(entry->flit->src == src){
			return true;
		}
		entry = entry->next;
	}
	return false;
}

rank_t pnbe0_probe_any(node_t *node)
{
	if(((pnbe0_context_t *)node->noc_context)->core_buffer.head != NULL){
		return ((pnbe0_context_t *)node->noc_context)->core_buffer.head->flit->src;
	}
	return (rank_t)-1;
}

/*
 * This function simulates one cycle for a node, it's supposed to be called for every node in order (anything else will lead to errors).
 * rough order of things done:
 * - the input fields are checked for incoming flits
 * - the southern input is processed
 * - the western input is processed
 * - the core input queue is processed
 * - the corner-buffer is processed
 * - input fields are cleared
 * - timers are decremented
 * - THE LAST NODE:
 * - handles the signaling of all nodes for next round
 * - updates the input field of all nodes for the next cycle
 * - clears the output field for all nodes
 */
void pnbe0_route_one_cycle(node_t *node)
{
	if(node->rank == 0)
		debug(log_intercon,"\nNoC cycle %ld\n",node->cycle);

//	if(node->cycle > 500)
//		exit(0);

	bool in_west = false;
	bool in_south = false;
	bool in_core = false;

	bool out_north = false;
	bool out_corner = false;
	bool out_core = false;
	bool out_east = false;

	bool h_blocked = false;

	bool delayedEnqueueWest = false;
	bool delayedEnqueueCore = false;


	// Flits injected in the corner-buffer can't leave it in the same round. This is a counter for the flits injected.
	uint_fast8_t justInjected = 0;

	pnbe0_context_t *pnode = ((pnbe0_context_t *)node->noc_context); //readability

	// prepare next flit in injectionQueue
	if(pnode->paternoster_in_core.flit == NULL){
		pnode->paternoster_in_core.flit = msgdeque(&pnode->injectionQueue);
	}

	// check if there is a block in effect
	if(pnode->h_timers[node->rank%conf_noc_width] != 0) h_blocked = true;

	if(h_blocked)
		debug(log_intercon,"Node %lu is h_blocked\n", node->rank);

	if(pnode->receive_timeout)
		debug(log_intercon,"Node %lu not receiving\n",node->rank);


	// check for incoming flits
	if(pnode->paternoster_in_west.flit != NULL){
		in_west = true;
		if(pnode->paternoster_in_west.flit->h_marked == true && pnode->paternoster_in_west.flit->dest%conf_noc_width != node->rank%conf_noc_width){
			pnode->h_timers[pnode->paternoster_in_west.flit->dest % conf_noc_width] = conf_noc_width;
			debug(log_intercon,"Node %lu: Received marked flit\n",node->rank);
		}
		if(pnode->paternoster_in_west.flit->src == node->rank){
			pnode->sendBlock = conf_noc_width;
			debug(log_intercon,"Node %lu: Received own flit\n",node->rank);
		}
	}

	if(pnode->paternoster_in_south.flit != NULL){
		in_south = true;
	}

	// check
	if(pnode->paternoster_in_core.flit != NULL){
		in_core = true;
	}

	// process the south input
	if(in_south){
		int direction;
		//determine the flits destination
		if(pnode->paternoster_in_south.flit->dest == node->rank){
			direction = 2;
		} else{
			direction = 1;
		}

		//send it to the next field accordingly
		switch(direction){
			//south to north
			case 1:
				{
					out_north = true; pnode->paternoster_out_north.flit = pnode->paternoster_in_south.flit;
					debug(log_intercon,"Node %lu: Forwarding south to north\n",node->rank);
					break;
				}
			//south to core
			case 2:
				{
					out_core = true;
					debug(log_intercon,"Node %lu: Forwarding south to local_out\n",node->rank);
					if(!msgenque(&pnode->core_buffer, pnode->paternoster_in_south.flit)){
						fatal("Flit was overwritten at node %lu by a flit coming from south, core needs to handle this", node->rank);
						break;
					}
					break;
				}
			default: fatal("Can't determine exit node");
		}
	}

	// process the west input
	if(in_west){
		int direction = 0;
		// if the node is h_blocked, all flits will simply be forwarded	-> except flits for this core
		if(h_blocked){
			if(pnode->paternoster_in_west.flit->dest == node->rank && !out_core && pnode->receive_timeout == 0){	// however if the core is receive blocked or already receiving, he can't receive
				debug(log_intercon,"Node %lu: Forwarding west to local_out\n",node->rank);
				if(!msgenque(&pnode->core_buffer,pnode->paternoster_in_west.flit))
					fatal("Flit was overwritten at node %lu by a flit coming from west, core needs to handle this", node->rank);
			} else{
				out_east = true;	// to make sure a the core doesn't overwrite flits
				debug(log_intercon,"Node %lu: Forwarding west to east\n",node->rank);
				pnode->paternoster_out_east.flit = pnode->paternoster_in_west.flit;
			}
		} else{
			//determine the flits destination
			if(pnode->paternoster_in_west.flit->dest == node->rank){
				direction = 3;
			}
			else if(pnode->paternoster_in_west.flit->dest%conf_noc_width == node->rank%conf_noc_width){
				direction = 2;
			}
			else{
				direction = 1;
			}
		}
		//send it to the next field accordingly
		switch(direction){
			//west to east
			case 1:
				{
					debug(log_intercon,"Node %lu: Forwarding west to east\n",node->rank);
					pnode->paternoster_out_east.flit = pnode->paternoster_in_west.flit;
					break;
				}
			//west to corner-buffer
			case 2:
			{
				// The corner-buffer causes a one cycle delay, which is implemented by the justInjected counter
				if(!msgenque(&pnode->corner_buffer, pnode->paternoster_in_west.flit)){
					// if a slot becomes available this cycle it can still be used in this cycle
					if(!out_north && !(pnode->isSignaledNorth && pnode->sentNorth)){
						delayedEnqueueWest = true;
						out_corner = true;
						break;
					}
					// send flit around in a cycle since the corner-buffer is full and won't be able to free a slot this cycle
					pnode->paternoster_in_west.flit->h_marked = true;
					pnode->paternoster_out_east.flit = pnode->paternoster_in_west.flit;
					// no flits are taken from the horizontal ring until this flit returns
					pnode->h_timers[node->rank%conf_noc_width] = conf_noc_width; 	// needs to be 1 higher than the timeout is actually takes, since it's going to be decremented in this cycle already
					debug(log_intercon,"Node %lu: Flit marked, forwarding west to east\n",node->rank);
					break;
				}
				debug(log_intercon,"Node %lu: Forwarding west to corner-buffer\n",node->rank);
				justInjected = justInjected + 1;	// Count flits injected this cycle, so they don't get ejected in the same
				out_corner = true;
				break;
			}
			//west to core, unless the core is already receiving a flit coming from in_south or is c_blocked (core blocked), then it will be west to east
			case 3:
			{
				if(!out_core && pnode->receive_timeout == 0){	// local out free and no block in effect
					out_core = true;
					debug(log_intercon,"Node %lu: Forwarding west to local_out\n",node->rank);
					if(!msgenque(&pnode->core_buffer, pnode->paternoster_in_west.flit)){
						fatal("Flit was overwritten at node %lu by a flit coming from west, core needs to handle this", node->rank);
					}
					break;
				} else if(pnode->receive_timeout != 0){		// local out free, but block in effect
					debug(log_intercon,"Node %lu: Receive_block, forwarding west to east\n",node->rank);
					pnode->paternoster_out_east.flit = pnode->paternoster_in_west.flit;
					break;
				} else{		// local out already taken
					debug(log_intercon,"Node %lu: Conflict, forwarding west to east\n",node->rank);
					pnode->receive_timeout = conf_noc_width;	// don't receive any flits for a round
					pnode->paternoster_out_east.flit = pnode->paternoster_in_west.flit;	// send flit on a circle
					break;
				}
			}
			default: break;		// h_block ends up here
		}
	}

	//process the core injection queue
	if(in_core){
		int direction = 0;
		//determine the flits destination
		if(pnode->paternoster_in_core.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			direction = 1;
		}
		else{
			direction = 2;
		}
		switch(direction){
		case 1:		// corner-buffer
		{
			// a h_blocked node can't send north
			if(!h_blocked && !out_corner){
				if(!msgenque(&pnode->corner_buffer,pnode->paternoster_in_core.flit)){
					// if a slot becomes available this cycle the flit can be sent
					if(!delayedEnqueueWest && !(pnode->isSignaledNorth && pnode->sentNorth))
						delayedEnqueueCore = true;
					else
						pnode->signalWest = true;		// Note: Added signaling west for core to corner-buffer
					break;
				}
				justInjected = justInjected + 1;
				pnode->paternoster_in_core.flit = NULL;		// flit sent, clear queue
				info(log_intercon,"Node %lu sent flit from local_in to corner-buffer, cycle: %ld\n",node->rank,node->cycle);
			}
			break;
		}
		case 2:		// east
		{
			// check for blocks preventing the flit from being sent
			bool blocked = false;
			if(node->rank/conf_noc_width == pnode->paternoster_in_core.flit->dest/conf_noc_width){	// check if the target is in the same row;
				if(pnode->sendBlock != 0 )	// the sendBlock is only relevant if the core wants to send a flit to another core in the same row
					blocked = true;
			} else{	// the target is in a different row and column
				if(pnode->h_timers[pnode->paternoster_in_core.flit->dest % conf_noc_width] != 0 )	// check whether information about a corner-buffer block in the target column was received
					blocked = true; // don't send the flit
			}

			if(!blocked){
				if(in_west){
					if(pnode->paternoster_in_west.flit->dest % conf_noc_width != node->rank % conf_noc_width && !out_east){	// check if west flit wants to go east or was forced to go east
						// signal western core
						pnode->signalWest = true;
						break;
					}
				}
				if(pnode->isSignaledEast == true && pnode->sentEast == true){
					pnode->sentEast = false;
					pnode->signalWest = true;
				} else{
					pnode->paternoster_out_east.flit = pnode->paternoster_in_core.flit;
					pnode->paternoster_in_core.flit = NULL;		// flit sent, clear queue
					pnode->sentEast = true;
					info(log_intercon,"Node %lu sent flit from local_in east, cycle: %ld\n",node->rank,node->cycle);
				}
			} else{	// no flit was sent east
				if(pnode->paternoster_out_east.flit == NULL)
					pnode->sentEast = false;
				debug(log_intercon,"Node %lu: Send blocked\n",node->rank);
			}
			break;
		}
		default: break;
		}


	} else if(pnode->paternoster_out_east.flit == NULL){
		pnode->sentEast = false;	// free slot passed by
	} else if(pnode->isSignaledEast){
		pnode->signalWest = true;
	}


	// process the corner-buffer
	if(pnode->corner_buffer.slots + justInjected != pnode->corner_buffer.size){		// check for flits, which can be sent this cycle
		if(out_north){		// check if north output field is already occupied
			pnode->signalSouth = true;	// signal south for a free slot
		}
		else if(pnode->isSignaledNorth && pnode->sentNorth){	// the output field is available, however the node is signaled and already sent a flit the last time
			pnode->sentNorth = false;	// let the free slot pass by
		}
		else{	// flit in the corner-buffer and output field available
			pnode->paternoster_out_north.flit = msgdeque(&pnode->corner_buffer);	// dequeue a flit from the corner-buffer and send it north
			pnode->sentNorth = true;
			debug(log_intercon,"Node %lu corner-buffer sent flit\n",node->rank);
			// if there are flits, waiting to be enqueued, enqueue them
			if(delayedEnqueueWest){
				if(msgenque(&pnode->corner_buffer, pnode->paternoster_in_west.flit))
					debug(log_intercon,"Node %lu: Forwarding west to corner-buffer\n",node->rank);

			} else if(delayedEnqueueCore){
				if(msgenque(&pnode->corner_buffer, pnode->paternoster_in_core.flit)){
					info(log_intercon,"Node %lu sent flit from local_in to corner-buffer, cycle: %ld\n",node->rank,node->cycle);
					pnode->paternoster_in_core.flit = NULL;
				}
			}
		}
	} else{
		if(!out_north)		// no flit going north and corner-buffer is empty
			pnode->sentNorth = false;	// an empty slot passed
		else if(pnode->isSignaledNorth)	// corner-buffer-empty, but there is a flit coming from south and a node north of this node wants to send
			pnode->signalSouth = true;	// relay signal
	}


	// clear paternoster_in_ fields for the next cycle, DO NOT clear the injection queue

	msg_field_clear_back_up(&pnode->paternoster_in_south);
	msg_field_clear_back_up(&pnode->paternoster_in_west);

	// decrement timeouts
	int i;
	for(i = 0; i < conf_noc_width; i++){				// decrement horizontal blocks, caused by a full corner-buffer
		if(pnode->h_timers[i] != 0)
			pnode->h_timers[i] = pnode->h_timers[i] - 1;
	}
	if(pnode->sendBlock != 0){							// decrement sendBlock, caused by receiving one's own flit
		pnode->sendBlock = pnode->sendBlock - 1;
	}
	if(pnode->receive_timeout != 0)						// decrement receive timeout, caused by simultaneously receiving two flits, which reached their destination
		pnode->receive_timeout = pnode->receive_timeout - 1;

	// clear flags
	pnode->isSignaledEast = false;
	pnode->isSignaledNorth = false;

	// fetch new values for the paternoster_in_ fields from the corresponding paternoster_out_ fields

/*
 * Since this program runs sequential the exchange of data can only happen when all nodes have been simulated.
 * So the last node will copy flits from all output fields to the input fields of neighboring nodes and set the signal flags for the next cycle.
 */
	if(node->rank == conf_max_rank-1){
		rank_t x,y;
		for(x = 0; x < conf_max_rank; x++){	// for each node

			// signal the other nodes
			if(((pnbe0_context_t *)nodes[x]->noc_context)->signalWest == true){
				signalWesternNode(nodes[x]);
				((pnbe0_context_t *)nodes[x]->noc_context)->signalWest = false;
			}

			if(((pnbe0_context_t *)nodes[x]->noc_context)->signalSouth == true){
				signalSouthernNode(nodes[x]);
				((pnbe0_context_t *)nodes[x]->noc_context)->signalSouth = false;
			}

			// Update the input fields for next cycle
			// fetch flits coming from south
			((pnbe0_context_t *)nodes[x]->noc_context)->paternoster_in_south.flit = ((pnbe0_context_t *)nodes[(x+conf_noc_width)%conf_max_rank]->noc_context)->paternoster_out_north.flit;

			//fetch flits coming from west
			if(x%conf_noc_width == 0){			// if the node is the first in the line
				y = x + conf_noc_width - 1;
			}
			else{										// if it isn't
			y = nodes[x]->rank - 1;
			}
			((pnbe0_context_t *)nodes[x]->noc_context)->paternoster_in_west.flit = ((pnbe0_context_t *)nodes[y]->noc_context)->paternoster_out_east.flit;
		}

		// after all output fields have been copied to the input fields they now need to be cleared for the next cycle
		for(x = 0; x < conf_max_rank; x++){
			msg_field_clear_back_up(&((pnbe0_context_t *)nodes[x]->noc_context)->paternoster_out_north);
			msg_field_clear_back_up(&((pnbe0_context_t *)nodes[x]->noc_context)->paternoster_out_east);
		}
	}

	if(node->rank == conf_max_rank-1)
		debug(log_intercon,"NoC Cycle End\n\n");
}

/*
 * Initialize the paternoster NoC
 * - overwrite generic functions of node
 * - allocate memory for the context specific variables
 * - initialize these variables
 */
void pnbe0_init(node_t *node)
{
	// Override functions
	node->noc_send_flit         = pnbe0_send_flit;
	node->noc_recv_flit         = pnbe0_recv_flit;
	node->noc_sender_ready      = pnbe0_sender_ready;
	node->noc_probe_rank        = pnbe0_probe_rank;
	node->noc_probe_any         = pnbe0_probe_any;
	node->noc_route_one_cycle   = pnbe0_route_one_cycle;

	// add NoC specific fields to the node
	pnbe0_context_t *context = malloc(sizeof(pnbe0_context_t));
	if(context == NULL){
		fatal("Malloc failed to allocate memory");
	}
	node->noc_context = context;

	// initialize the just added fields
	msg_field_clear_back_up(&((pnbe0_context_t *)node->noc_context)->paternoster_in_south);
	msg_field_clear_back_up(&((pnbe0_context_t *)node->noc_context)->paternoster_in_west);
	msg_field_clear_back_up(&((pnbe0_context_t *)node->noc_context)->paternoster_in_core);
	uint_fast32_t size;
	// This sets the corner-buffer size
	size = CORNER_BUFFER_SIZE;
	buffer_init(&((pnbe0_context_t *)node->noc_context)->corner_buffer, size);
	// This sets the core-buffer size
	size = conf_noc_width;
	buffer_init(&((pnbe0_context_t *)node->noc_context)->core_buffer, size);
	// This sets the injectionQueue size (+1)
	size = conf_noc_width - 1;
	buffer_init(&((pnbe0_context_t *)node->noc_context)->injectionQueue, 128);

	((pnbe0_context_t *)node->noc_context)->sentNorth = false;
	((pnbe0_context_t *)node->noc_context)->sentEast = false;
	((pnbe0_context_t *)node->noc_context)->isSignaledNorth = false;
	((pnbe0_context_t *)node->noc_context)->isSignaledEast = false;
	((pnbe0_context_t *)node->noc_context)->signalWest = false;
	((pnbe0_context_t *)node->noc_context)->signalSouth = false;

	// add and initialize timers
	int i;
	((pnbe0_context_t *)node->noc_context)->h_timers = malloc(sizeof(rank_t)*conf_noc_width);
	for(i = 0; i < conf_noc_width; i++){
		((pnbe0_context_t *)node->noc_context)->h_timers[i] = 0;
	}
	((pnbe0_context_t *)node->noc_context)->receive_timeout = 0;
	((pnbe0_context_t *)node->noc_context)->sendBlock = 0;
}

void pnbe0_destroy(node_t *node)
{
	free(((pnbe0_context_t *)node->noc_context)->h_timers);
	free(node->noc_context);
}



