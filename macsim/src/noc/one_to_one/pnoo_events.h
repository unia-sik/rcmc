#pragma once


#define pnoo_events_count 10

/**
* @brief enum containing all events in the noc
* @warning update 'pnoo_events_count' if you change the enum!
* NOTE: Read the warning! 
*/
typedef enum {
    PNOO_EVENT_SEND_BUFFER_REMOVE,
    PNOO_EVENT_SEND_BUFFER_DECREMENT,
    PNOO_EVENT_RECV_BUFFER_INSERT,
    PNOO_EVENT_RECV_BUFFER_INCREMENT,
    
    PNOO_EVENT_RDY_RECV,
    PNOO_EVENT_RDY_RECV_COMMIT,
    PNOO_EVENT_RDY_RECV_RELEASE,
    PNOO_EVENT_BARRIER_SET,
    PNOO_EVENT_BARRIER_ENABLE,
    PNOO_EVENT_BARRIER_CLEAR
} pnoo_events_t;


char* pnoo_events_to_string(pnoo_events_t event);
