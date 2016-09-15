/*
 * fixedlat_context.h
 *
 *  Created on: 16.02.2015
 *      Author: gorlorom
 */

#ifndef FIXEDLAT_CONTEXT_H_
#define FIXEDLAT_CONTEXT_H_

#include "msgqueue.h"

typedef struct {
    msg_queue_t fixedlat_incomming;
    msg_queue_t fixedlat_intransit[FIXEDLAT_TRANSPORT_DELAY];
    msg_queue_t fixedlat_arrived;
    uint_fast32_t fixedlat_intransit_pos;
}fixedlat_context_t;



#endif /* FIXEDLAT_CONTEXT_H_ */
