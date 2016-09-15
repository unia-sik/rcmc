/*
 * gs_all_to_all.c
 *
 *  Created on: 19.03.2015
 *      Author: gorlorom
 */

/*
 * Note: There is a buffer bypass specifically for the last column, so a whole iteration can still be done in (n*n*(n-1))/2
 */

#include "pnaa.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


extern node_t *nodes[];

void pnaa_updateInjectionQueue(node_t *node){
	pnaa_context_t *gnode = ((pnaa_context_t *)node->noc_context); //readability

	if(gnode->in_core.flit == NULL){
		gnode->in_core.flit = msgdeque(&gnode->injectionQueue);
	}
}


bool pnaa_send_flit(node_t *node, rank_t dest, flit_t flit)
{
    pnaa_updateInjectionQueue(node);
    flit_container_t *fc = malloc(sizeof(flit_container_t));
    fc->src = node->rank;
    fc->dest = dest;
    memcpy(fc->payload, &flit, FLIT_LEN);
    // try to enqueue the flit
    if(msgenque(&((pnaa_context_t *)node->noc_context)->injectionQueue, fc)) {
        debug(log_intercon,"Node %lu: Flit for %lu queued\n", node->rank, dest);
        return true;
    }
    free(fc);
    return false;
}


bool pnaa_recv_flit(node_t *node, rank_t src, flit_t *flit)
{
    flit_container_t *fc =
         msgdeque_rank(&((pnaa_context_t *)node->noc_context)->core_buffer, 
         src);
    if(fc == NULL) return false;
    memcpy(flit, fc->payload, FLIT_LEN);
    free(fc);
    info(log_intercon, "Core %lu received flit from %lu\n", node->rank, src);
    return true;
}



bool pnaa_sender_ready(node_t *node){
	if(((pnaa_context_t *)node->noc_context)->injectionQueue.slots != 0)
		return true;
	return false;
}

bool pnaa_probe_rank(node_t *node, rank_t src){
	p_buffer_entry_t *entry = ((pnaa_context_t *)node->noc_context)->core_buffer.head;
	while(entry != NULL){
		if(entry->flit->src == src){
			return true;
		}
		entry = entry->next;
	}
	return false;
}
rank_t pnaa_probe_any(node_t *node){
	if(((pnaa_context_t *)node->noc_context)->core_buffer.head != NULL){
		return ((pnaa_context_t *)node->noc_context)->core_buffer.head->flit->src;
	}
	return (rank_t)-1;
}

// Simulate the transportation of the messages
void pnaa_route_one_cycle(node_t *node){

	pnaa_context_t *gnode = ((pnaa_context_t *)node->noc_context); //readability

	// update injectionQueue for the new Cycle
	pnaa_updateInjectionQueue(node);

//	if(node->cycle > 500)
//		exit(0);


	if(gnode->smallCycle == ((conf_noc_width*conf_noc_width * (conf_noc_width - 1))/2)){
		gnode->smallCycle = 0;
		gnode->hPhase = 0;
		gnode->hCycle = 0;
		gnode->vCycle = 0;
	}

	if(node->rank == 0){
		debug(log_intercon,"\n\nCycle: %lu hPhase: %lu hCycle: %u vCycle: %lu\n",gnode->smallCycle, gnode->hPhase, gnode->hCycle, gnode->vCycle);
	}

	// first phase
	if(gnode->hPhase == 0){
		int i;
		for(i = 0;i < conf_noc_width - 1;i++){
			if(gnode->in_core.flit != NULL && gnode->in_core.flit->dest%conf_noc_width == node->rank%conf_noc_width){
				if(gnode->in_core.flit->dest <= node->rank){
					// Note: insert flits in the corner-buffer according to the distance to their destination
					if(gnode->corner_buffer[(node->rank - gnode->in_core.flit->dest) / conf_noc_width] != NULL)
						break;
					gnode->corner_buffer[(node->rank - gnode->in_core.flit->dest) / conf_noc_width] = gnode->in_core.flit;
					debug(log_intercon,"Node %lu: Core to corner-buffer slot %lu\n",node->rank,(node->rank - gnode->in_core.flit->dest)/conf_noc_width);
					gnode->in_core.flit = NULL;
				}
				else{
					if(gnode->corner_buffer[conf_noc_width - (gnode->in_core.flit->dest - node->rank)/conf_noc_width] != NULL)
						break;
					gnode->corner_buffer[conf_noc_width - (gnode->in_core.flit->dest - node->rank)/conf_noc_width] = gnode->in_core.flit;
					debug(log_intercon,"Node %lu: Core to corner-buffer slot %lu\n",node->rank,conf_noc_width - (gnode->in_core.flit->dest-node->rank)/conf_noc_width);
					gnode->in_core.flit = NULL;
				}
				pnaa_updateInjectionQueue(node);
			}
			else{
				break;
			}
		}
		gnode->hPhase = 1;
	}


	// horizontal send, starts with hPhase == 1 -------------------------------------------------------------------------------------------------------------
	if(gnode->hTimeout == 0){
		uint_fast32_t target_column_offset, target_node_offset;
		rank_t target_node;

		// determine the target column
		if(gnode->hPhase == conf_noc_width - 1){
			target_column_offset = conf_noc_width - 1;
		}
		// second to last phase for odd number of columns -> send to middle column
		else if(conf_noc_width % 2 == 1 && gnode->hPhase == conf_noc_width - 2){
			target_column_offset = conf_noc_width/2;
		}
		else{
			// far column if conf_noc_width is even
			if(gnode->hPhase%2 == 1 && conf_noc_width%2 == 0){
				target_column_offset = conf_noc_width/2 + gnode->hPhase/2;
			}
			// far column if conf_noc_width is odd
			else if(gnode->hPhase%2 == 1 && conf_noc_width%2 == 1){
				target_column_offset = conf_noc_width/2 + 1;
			}
			// near column
			else{
				target_column_offset = conf_noc_width/2 - gnode->hPhase/2;
			}
		}

		// determine the node offset in the target column
		if(gnode->hCycle == conf_noc_width - 1){
			target_node_offset = 0;
		}
		else if(conf_noc_width%2 == 1 && gnode->hCycle == conf_noc_width - 2){
			target_node_offset = conf_noc_width/2;
		}
		else{
			// far node, even number of cores per line
			if(gnode->hCycle%2 == 0 && conf_noc_width%2 == 0){
				target_node_offset = conf_noc_width/2 + gnode->hCycle/2;
			}
			// far node, odd number of nodes per line
			else if(gnode->hCycle%2 == 0 && conf_noc_width%2 == 1){
				target_node_offset = conf_noc_width/2 + gnode->hCycle/2 + 1;
			}
			// near node
			else{
				target_node_offset = conf_noc_width/2 - gnode->hCycle/2 - 1;
			}
		}

		// calculate target_node
		// a) shift to target column
		if((node->rank + target_column_offset)/conf_noc_width == node->rank/conf_noc_width){
			target_node = node->rank + target_column_offset;
		}
		else{
			target_node = node->rank + target_column_offset - conf_noc_width;
		}
		// b) shift to target node
		if(node->rank/conf_noc_width >= target_node_offset){
			target_node = target_node - target_node_offset * conf_noc_width;
		}
		else{
			target_node = target_node + conf_max_rank - target_node_offset * conf_noc_width;
		}

		// debug
		if(node->rank == 0)
			debug(log_intercon,"Horizontal send: C.Offset: %lu N.Offset: %lu\n",target_column_offset, target_node_offset);

		// check if flit in injectionQueue has the target node as its destination
		if(gnode->in_core.flit != NULL && gnode->in_core.flit->dest == target_node){
			gnode->out_east.flit = gnode->in_core.flit;
			debug(log_intercon,"Node %lu: Sent flit from injectionQueue east\n",node->rank);
			gnode->in_core.flit = NULL;
		}

		// set timeout before the next send
		gnode->hTimeout = target_column_offset - 1;

		// change phase if all flits for a column have been sent
		if(gnode->hCycle == conf_noc_width - 1){
			gnode->hPhase = gnode->hPhase + 1;
			gnode->hCycle = 0;
		}
		else{
			gnode->hCycle = gnode->hCycle + 1;
		}
	}
	else{
		gnode->hTimeout = gnode->hTimeout - 1;
	}


	// vertical send ----------------------------------------------------------------------------------------------------------------------------------------
	// Note: a full vertical send phase takes shorter than a horizontal send phase
	uint_fast32_t target_node_offset = 0;
	bool out_north = false;
	if(gnode->vTimeout == 0){
		if(conf_noc_width%2 == 1 && gnode->vCycle == conf_noc_width - 2){
			target_node_offset = conf_noc_width/2;
		}
		else{
			// far node, even number of cores per line
			if(gnode->vCycle%2 == 0 && conf_noc_width%2 == 0){
				target_node_offset = conf_noc_width/2 + gnode->vCycle/2;
			}
			// far node, odd number of cores per line
			else if(gnode->vCycle%2 == 0 && conf_noc_width%2 == 1){
				target_node_offset = conf_noc_width/2 + gnode->vCycle/2 + 1;
			}
			// near node
			else{
				target_node_offset = conf_noc_width/2 - gnode->vCycle/2 - 1;
			}
		}
		if(node->rank == 0){
			debug(log_intercon,"vertical target offset: %lu\n",target_node_offset);
		}

		// check if there is a flit in the corner-buffer with this distance to its destination
		if(gnode->corner_buffer[target_node_offset] != NULL){
			gnode->out_north.flit = gnode->corner_buffer[target_node_offset];
			debug(log_intercon,"Node %lu: corner-buffer sent slot %lu north\n",node->rank, target_node_offset);
			gnode->corner_buffer[target_node_offset] = NULL;
			out_north = true;
		}

		// set timeout before the next send
		gnode->vTimeout = target_node_offset - 1;

		// reset phase if all flits for a horizontal phase have been forwarded
		if(gnode->vCycle == conf_noc_width - 2){
			gnode->vCycle = 0;
		}
		else{
			gnode->vCycle = gnode->vCycle + 1;
		}
	}
	else{
		gnode->vTimeout = gnode->vTimeout - 1;
	}


	// forwarding and bypass
	if(gnode->in_south.flit != NULL){
		if(gnode->in_south.flit->dest == node->rank){
			if(!msgenque(&gnode->core_buffer,gnode->in_south.flit)){
				fatal("Node %lu: core-buffer overflow\n",node->rank);
			}
			debug(log_intercon,"Node %lu: south to core-buffer\n",node->rank);
		}
		else{
			gnode->out_north.flit = gnode->in_south.flit;
			debug(log_intercon,"Node %lu: south to north\n",node->rank);
			out_north = true;	//XXX this is probably not necessary
		}
		gnode->in_south.flit = NULL;
	}

	if(gnode->in_west.flit != NULL){
		rank_t target_node = conf_max_rank;
		if(target_node_offset != 0){
			if(node->rank/conf_noc_width >= target_node_offset){
				target_node = node->rank - target_node_offset * conf_noc_width;
			}
			else{
				target_node = node->rank + conf_max_rank - target_node_offset * conf_noc_width;
			}
		}
		// west to receiveQueue
		if(gnode->in_west.flit->dest == node->rank){
			if(!msgenque(&gnode->core_buffer,gnode->in_west.flit)){
				fatal("Node %lu: core-buffer overflow\n",node->rank);
			}
			debug(log_intercon,"Node %lu: west to core-buffer\n",node->rank);
		}
		// corner-buffer bypass
		else if(gnode->in_west.flit->dest == target_node && !out_north){
			gnode->out_north.flit = gnode->in_west.flit;
			debug(log_intercon,"Node %lu: west to north\n",node->rank);
		}
		// west to corner-buffer
		else if(gnode->in_west.flit->dest%conf_noc_width == node->rank%conf_noc_width){
			if(gnode->in_west.flit->dest <= node->rank){
				// Note: insert flits in the corner-buffer according to the distance to their destination
				gnode->corner_buffer[(node->rank - gnode->in_west.flit->dest) / conf_noc_width] = gnode->in_west.flit;
				debug(log_intercon,"Node %lu: west to corner-buffer slot %lu\n",node->rank,(node->rank - gnode->in_west.flit->dest)/conf_noc_width);
			}
			else{
				gnode->corner_buffer[conf_noc_width - (gnode->in_west.flit->dest - node->rank)/conf_noc_width] = gnode->in_west.flit;
				debug(log_intercon,"Node %lu: west to corner-buffer slot %lu\n",node->rank,conf_noc_width - (gnode->in_west.flit->dest-node->rank)/conf_noc_width);
			}
		}
		else{
			gnode->out_east.flit = gnode->in_west.flit;
			debug(log_intercon,"Node %lu: west to east\n",node->rank);
		}
		gnode->in_west.flit = NULL;
	}


	// last node updates everything for next cycle
	if(node->rank == conf_max_rank-1){
		rank_t x,y;
		// Update the input fields for next cycle
		for(x = 0; x < conf_max_rank; x++){

			// fetch flits coming from south
			((pnaa_context_t *)nodes[x]->noc_context)->in_south.flit = ((pnaa_context_t *)nodes[(x+conf_noc_width)%conf_max_rank]->noc_context)->out_north.flit;

			//fetch flits coming from west
			if(x%conf_noc_width == 0){			// if the node is the first in the line
				y = x + conf_noc_width - 1;
			}
			else{										// if it isn't
				y = nodes[x]->rank - 1;
			}
			((pnaa_context_t *)nodes[x]->noc_context)->in_west.flit = ((pnaa_context_t *)nodes[y]->noc_context)->out_east.flit;
		}

		// after all output fields have been copied to the input fields they now need to be cleared for the next cycle
		for(x = 0; x < conf_max_rank; x++){
			((pnaa_context_t *)nodes[x]->noc_context)->out_north.flit = NULL;
			((pnaa_context_t *)nodes[x]->noc_context)->out_east.flit = NULL;
		}
	}

	gnode->smallCycle = gnode->smallCycle + 1;
}

// initialize the gs_all_to_all interconnection simulator
void pnaa_init(node_t *node)
{
    node->noc_send_flit         = pnaa_send_flit;
    node->noc_recv_flit         = pnaa_recv_flit;

	node->noc_sender_ready      = pnaa_sender_ready;
	node->noc_probe_rank        = pnaa_probe_rank;
	node->noc_probe_any         = pnaa_probe_any;
	node->noc_route_one_cycle   = pnaa_route_one_cycle;


	// add NoC specific fields to the node
	pnaa_context_t *context = malloc(sizeof(pnaa_context_t));


	if(context == NULL){
		fatal("Malloc failed to allocate memory\n");
	}

	node->noc_context = context;
	pnaa_context_t *gnode = ((pnaa_context_t *)node->noc_context); //readability


	gnode->corner_buffer = malloc((conf_noc_width) * sizeof(flit_container_t *));
	if(gnode->corner_buffer == NULL)
		fatal("Malloc failed to allocate memory\n");

	// initialize the just added fields
	int i;

	for(i=0;i<conf_noc_width;i++)
		gnode->corner_buffer[i] = NULL;

	gnode->in_west.flit = NULL;
	gnode->in_south.flit = NULL;
	gnode->out_east.flit = NULL;
	gnode->out_north.flit = NULL;
	gnode->in_core.flit = NULL;


	gnode->hPhase = 0;
	gnode->hCycle = 0;
	gnode->hTimeout = 0;

	gnode->vCycle = 0;
	gnode->vTimeout = 0;

	gnode->smallCycle = 0;


	uint_fast32_t size;
	// This sets the core buffer size
	size = 2 * conf_noc_width*conf_noc_width;
	buffer_init(&gnode->core_buffer, 80);
	// This sets the injectionQueue size (+1)
	size = conf_noc_width - 1;
	buffer_init(&gnode->injectionQueue, size);

}

void pnaa_destroy(node_t *node){
	free(((pnaa_context_t *)node->noc_context)->corner_buffer);
	free(node->noc_context);
}

