/*
 * pbuffer.c
 *
 *  Created on: 17.02.2015
 *      Author: gorlorom
 *
 */

/*
 * FIFO-Buffer utility for flitwise routing.
 */

#include "pbuffer.h"

#include <stdlib.h>

// enqueue a flit in the buffer
bool msgenque(p_buffer_t *buffer, flit_container_t *flit){
	if(buffer->slots==0){
		return false;
	}

	p_buffer_entry_t *entry = malloc(sizeof(p_buffer_entry_t));		//new entry
	if(entry == NULL){
		fatal("Malloc failed to allocate memory");
	}

	// init entry
	entry->flit = flit;
	entry->next = NULL;

	if(buffer->tail == NULL){				//if the buffer is empty, set new head
		buffer->head = entry;
		buffer->tail = entry;
	} else{
		buffer->tail->next = entry;
		buffer->tail = entry;
	}
	buffer->slots = buffer->slots - 1; 	//decrement available slots
	return true;
}

// return the head of the buffer
flit_container_t *msgdeque(p_buffer_t *buffer){
	if(buffer->size == buffer->slots)
		return NULL;
	p_buffer_entry_t *del = buffer->head;
	flit_container_t *f = buffer->head->flit;
	buffer->head = buffer->head->next;
	free(del);
	if(buffer->head == NULL){
		buffer->tail = NULL;
	}
	buffer->slots = buffer->slots + 1;	//increment available slots
	return f;
}

// function is used exclusively by core buffers
// The first flit from the source 'rank' is dequeued
flit_container_t *msgdeque_rank(p_buffer_t *buffer, rank_t rank){
	p_buffer_entry_t *result, *previous;
	result = buffer->head;
	unsigned i;
	for(i = buffer->size - buffer->slots;i>0;i--){
		if(result->flit->src == rank){				// if the entry which is going to be removed is the head flit let the head pointer point to the next entry
			if(result == buffer->head){
				buffer->head = result->next;
				if(buffer->head == NULL){
					buffer->tail = NULL;
				}
			} else{						// else the next-pointer of the previous entry needs to be adjusted
				previous->next = result->next;
				if(result == buffer->tail){
					buffer->tail = previous;
				}
			}
			flit_container_t *flit = result->flit;
			free(result);
			buffer->slots++;				// increment available slots
			return flit;					// pointer to flit_container_t
		}
		previous = result;
		result = result->next;
	}
	return NULL;
}

//initialize the buffer
void buffer_init(p_buffer_t *buffer, uint8_t size){
	buffer->head = NULL;
	buffer->tail = NULL;
	buffer->slots = size;
	buffer->size = size;
}
