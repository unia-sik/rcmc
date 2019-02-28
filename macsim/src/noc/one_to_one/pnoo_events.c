#include "pnoo_events.h"

char* pnoo_events_to_string(pnoo_events_t event) {
    switch (event) {
        case PNOO_EVENT_SEND_BUFFER_REMOVE:    return "SEND_BUFFER_REMOVE";
        case PNOO_EVENT_SEND_BUFFER_DECREMENT: return "SEND_BUFFER_DECREMENT";
        case PNOO_EVENT_RECV_BUFFER_INSERT:    return "RECV_BUFFER_INSERT";
        case PNOO_EVENT_RECV_BUFFER_INCREMENT: return "RECV_BUFFER_INCREMENT";
        
        case PNOO_EVENT_RDY_RECV:              return "RDY_RECV";
        case PNOO_EVENT_RDY_RECV_COMMIT:       return "RDY_RECV_COMMIT";
        case PNOO_EVENT_RDY_RECV_RELEASE:      return "RDY_RECV_RELEASE";
        
        case PNOO_EVENT_BARRIER_SET:           return "RDY_BARRIER_SET";
        case PNOO_EVENT_BARRIER_ENABLE:        return "RDY_BARRIER_ENABLE";
        case PNOO_EVENT_BARRIER_CLEAR:         return "RDY_BARRIER_CLEAR";
        
        default: return "UNKNOWN";
    }
    
    return "UNKNOWN";
}
