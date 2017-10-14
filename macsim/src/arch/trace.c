//#define NEW_NT
#define LIST
// NEW_NT works only with LIST, not with a callback function

/**
 * trace.c
 * Core that sends and receives traces from netrace
 *
 * MacSim project
 *
 */

#include "trace.h"
#include "../../netrace/netrace.h"


// ugly but useful
extern node_t *nodes[];

uint_fast64_t packets_on_the_fly; // remaining packets
uint_fast64_t netrace_injected;
uint_fast64_t netrace_ejected;


nt_header_t *netrace_header;



static void netrace_callback_packet_cleared(nt_packet_t *packet);













#ifdef NEW_NT

#define NETRACE_MAX_CLEARED_PACKETS 0x10000 // 4096
nt_packet_t *netrace_cleared_packets[NETRACE_MAX_CLEARED_PACKETS];
unsigned netrace_num_cleared_packets = 0;

nt_dep_ref_node_t* ntq_add_dependency_node( unsigned int packet_id ) 
{
    assert (nt_dependency_array != NULL);

    // search packet id
    unsigned int index = packet_id % NT_DEPENDENCY_ARRAY_SIZE;
    nt_dep_ref_node_t* d = nt_dependency_array[index];
    nt_dep_ref_node_t *prev = NULL;
    while (d != NULL) {
        if (d->packet_id == packet_id ) return d;
            // return if packet is already in the dependency array
        prev = d;
        d = d->next_node;
    }
    // add new dependency node
    d = nt_checked_malloc( sizeof(nt_dep_ref_node_t) );
    if (prev == NULL) {
        nt_dependency_array[index] = d;
    } else {
        prev->next_node = d;
    }
    d->node_packet = NULL;
    d->packet_id = packet_id;
    d->ref_count = 0;
    d->next_node = NULL;
    return d;
}



void ntq_add_cleared_packet_to_list(nt_packet_t* packet)
{
#ifndef LIST
    netrace_callback_packet_cleared(packet);
#endif
//printf("[%d]", packet->id);

    if (netrace_num_cleared_packets==NETRACE_MAX_CLEARED_PACKETS) {
        nt_error( "Too many cleared packets. Increase NETRACE_MAX_CLEARED_PACKETS.");
    }
    netrace_cleared_packets[netrace_num_cleared_packets] = packet;
    netrace_num_cleared_packets++;
}



void ntq_empty_cleared_packets_list() {
    netrace_num_cleared_packets = 0;
}

nt_packet_t *ntq_read_and_clear()
{
    #pragma pack(push,1)
    struct nt_packet_pack {
        unsigned long long int cycle;
        unsigned int id;
        unsigned int addr;
        unsigned char type;
        unsigned char src;
        unsigned char dst;
        unsigned char node_types;
        unsigned char num_deps;
    };
    #pragma pack(pop)

    struct nt_packet_pack packed;
    assert (nt_input_tracefile != NULL && nt_dependencies_off==0);

    int err = fread(&packed, 1, sizeof(struct nt_packet_pack), nt_input_tracefile);
    if (err != sizeof(struct nt_packet_pack) ) {
        if (err<0) nt_error("Failed to read packet");
        if (err>0) nt_error("Unexpeded end of file");

        // End of file reached.
        // This is the exit condition... how do we signal it to the
        // network simulator? We shouldn't need to... It is tracking
        // whether there are packets in flight.
        // Just in case, we'll provide this global indicator
        nt_done_reading = 1;
        return NULL;
    }

    nt_packet_t *to_return = nt_packet_malloc();
    // necessary for portability
    to_return->cycle = packed.cycle;
    to_return->id = packed.id;
    to_return->addr = packed.addr;
    to_return->type = packed.type;
    to_return->src = packed.src;
    to_return->dst = packed.dst;
    to_return->node_types = packed.node_types;
    to_return->num_deps = packed.num_deps;

    if (packed.num_deps==0) {
        to_return->deps = NULL;
    } else {
        to_return->deps = nt_dependency_malloc(packed.num_deps);
        err = fread( to_return->deps, sizeof(nt_dependency_t), 
            packed.num_deps, nt_input_tracefile );
        if (err < 0 ) nt_error("Failed to read dependencies");

        // Track dependencies: add to_return downward dependencies to array
        unsigned i;
        for (i=0; i<packed.num_deps; i++) {
            unsigned int dep_id = to_return->deps[i];
            nt_dep_ref_node_t* node_ptr = ntq_add_dependency_node(dep_id);
            node_ptr->ref_count++;
        }
    }


    // add to_return to dependencies array
    nt_dep_ref_node_t* n = ntq_add_dependency_node(packed.id);
    n->node_packet = to_return;

    // add to the cleared packet list,  if there are no more dependencies
    if (n->ref_count==0) {
        ntq_add_cleared_packet_to_list(to_return);
    }

    nt_num_active_packets++;
    nt_latest_active_packet_cycle = packed.cycle;

//printf("READ_AND_CLEAR id %d latest=%llu active=%llu fly=%lu\n",
//to_return->id, nt_latest_active_packet_cycle, nt_num_active_packets, packets_on_the_fly);

    return to_return;
}

void ntq_read_ahead( unsigned long long int current_cycle ) 
{
    unsigned long long int read_to_cycle = current_cycle + NT_READ_AHEAD;
    assert (read_to_cycle > current_cycle && "cycle counter overflow");

    if (nt_done_reading) return;
    while (nt_latest_active_packet_cycle<=read_to_cycle &&
        ntq_read_and_clear()!=NULL) {
//printf("NTQ_READ_AHEAD latest=%llu to=%llu num=%llu\n",
//nt_latest_active_packet_cycle, read_to_cycle, NT_READ_AHEAD /* netrace_num_cleared_packets*/);
//printf(" latest=%llu to=%llu\n", nt_latest_active_packet_cycle, read_to_cycle);
}
}

// called from nt_seek_region()
void nt_prime_self_throttle() 
{
    nt_primed_self_throttle = 1;
    nt_packet_t* packet = ntq_read_and_clear();
    ntq_read_ahead( packet->cycle );
}

void ntq_init_self_throttling() 
{
    assert (nt_dependencies_off==0);
    nt_self_throttling = 1;
    nt_track_cleared_packets_list = 1;
    ntq_empty_cleared_packets_list();
    nt_prime_self_throttle();
}

void ntq_clear_dependencies_free_packet( nt_packet_t* packet )
{
    assert (nt_input_tracefile!=NULL && packet!=NULL && nt_self_throttling
        && nt_dependency_array!=NULL && nt_track_cleared_packets_list);
//    ntq_read_ahead(packet->cycle);

    unsigned int i;
    for( i = 0; i < packet->num_deps; i++ ) {
        unsigned int id = packet->deps[i];
        unsigned int index = id % NT_DEPENDENCY_ARRAY_SIZE;
        nt_dep_ref_node_t* d = nt_dependency_array[index];
        while (d != NULL && d->packet_id!=id) {
            d = d->next_node;
        }

        assert (d!=NULL && d->ref_count!=0);

        d->ref_count--;
        if (d->ref_count==0 && d->node_packet) {
            // This test alleviates the possibility of a packet
            // having ref_count zero before it has been read
            // from the trace (node_packet = NULL)
            ntq_add_cleared_packet_to_list(d->node_packet);
        }
    }
    nt_remove_dependency_node( packet->id );
    nt_packet_free( packet );
    nt_num_active_packets--;
}


#endif












// init netrace
static void init()
{
#ifdef NEW_NT
    assert (nt_dependencies_off==0);
    nt_self_throttling = 1;
    nt_track_cleared_packets_list = 1;
    ntq_empty_cleared_packets_list();

    //nt_prime_self_throttle(): 
    nt_primed_self_throttle = 1;
    nt_packet_t* packet = ntq_read_and_clear();
    ntq_read_ahead( packet->cycle );
#else
    nt_init_self_throttling();
#endif
}







static void netrace_callback_packet_cleared(nt_packet_t *packet)
{
    // Ignore messages with same sender and receiver
    if (packet->src == packet->dst) {

#ifdef NEW_NT
#ifdef LIST
        ntq_read_ahead(packet->cycle);
#endif
        ntq_clear_dependencies_free_packet(packet);
#else
        nt_clear_dependencies_free_packet(packet);
#endif // NEW_NT

//        netrace_injected++;
//        netrace_ejected++;  
        return;
    }

//printf("Clear  %ld id %d\n", nodes[0]->cycle, packet->id);

    // add to injection queue of sending node (produce)
    flit_queue_entry_t *n = fatal_malloc(sizeof(flit_queue_entry_t));
    n->next = 0;
    n->dest = packet->dst;
    n->flit = (flit_t)packet; // flit payload is pointer to packet
    rank_t src = packet->src;
    if (nodes[src]->core.netrace.injqueue_tail!=0) {
        nodes[src]->core.netrace.injqueue_tail->next = n;
        nodes[src]->core.netrace.injqueue_tail = n;
    } else {
        nodes[src]->core.netrace.injqueue_tail = n;
        nodes[src]->core.netrace.injqueue_head = n;
    }
    packets_on_the_fly++;
}



bool netrace_inject_messages(node_t **nodes)
{

#ifdef NEW_NT
    unsigned i;

//    printf("#%ld CLEARED", nodes[0]->cycle);
//    for (i=0; i<netrace_num_cleared_packets; i++)
//        printf(" %d", netrace_cleared_packets[i]->id);
//    printf(" END\n");

    for (i=0; i<netrace_num_cleared_packets; i++) {
        nt_packet_t *packet = netrace_cleared_packets[i];
        assert (packet != NULL);
#ifdef LIST
        netrace_callback_packet_cleared(packet);
#endif
    }
    ntq_empty_cleared_packets_list();
#else
    nt_packet_list_t* list;
    for (list = nt_get_cleared_packets_list();
         list != NULL;
         list = list->next) 
    {
        assert (list->node_packet != NULL);
        nt_packet_t *packet = list->node_packet;
        netrace_callback_packet_cleared(packet);
    }
    nt_empty_cleared_packets_list();
#endif // NEW_NT


    // return false if simulation finished
    return (packets_on_the_fly!=0 ||
        nodes[0]->cycle<=netrace_header->num_cycles);
}









// Simulate one cycle
instruction_class_t netrace_one_cycle(node_t *node)
{
    // if flit arrived, clear dependencies
    rank_t src = node->noc_probe_any(node);
    if (src>=0) {
        flit_t flit;
        node->noc_recv_flit(node, src, &flit);
        if (flit!=0) {
            nt_packet_t *packet = (nt_packet_t *)flit;

//printf("Eject  %ld id %d\n", node->cycle, packet->id);

            netrace_ejected++;

#ifdef NEW_NT
            ntq_read_ahead(packet->cycle);
            ntq_clear_dependencies_free_packet(packet);
#else
            nt_clear_dependencies_free_packet(packet);
#endif // NEW_NT


            packets_on_the_fly--;
        }
    }




    if (node->core.netrace.pending_len == 0) {

        // check if a new message from the trace was injected (consumer)
        if (node->core.netrace.injqueue_head!=0) {
            flit_t flit = node->core.netrace.injqueue_head->flit;
            nt_packet_t *packet = (nt_packet_t *)flit;
            // remove from injection queue
            flit_queue_entry_t *h = node->core.netrace.injqueue_head;
            flit_queue_entry_t *n = h->next;
            node->core.netrace.injqueue_head = n;
            if (n==0) node->core.netrace.injqueue_tail = 0;
            free(h);

            node->core.netrace.pending_len = nt_get_packet_size(packet) / FLIT_LEN;
            node->core.netrace.pending_dest = packet->dst;
            node->core.netrace.pending_flit = flit;

            int src_type = nt_get_src_type(packet);
            int dst_type = nt_get_dst_type(packet);

            // calculate additional latency depending on message type
            instruction_class_t latency = 0;
            if (src_type == NT_NODE_TYPE_MC) {
                latency = 150; // memory access latency
            } else if (src_type == NT_NODE_TYPE_L2) {
                if (dst_type == NT_NODE_TYPE_MC) {
                    latency = 2; // L2 tag access latency
                } else if ((dst_type==NT_NODE_TYPE_L1D) ||
                    (dst_type==NT_NODE_TYPE_L1I)) 
                {
                    latency = 8; // L2 cache bank access latency
                }
            } else {
                assert ((src_type==NT_NODE_TYPE_L1D) ||
                    (src_type==NT_NODE_TYPE_L1I));

                unsigned pt = packet->type;
                if (pt==1 || pt==4 || pt==13 || pt==15 || pt==27 || pt==29) {
                    // request from L1:
                    // provide, that the difference to the trace timestamp
                    // is the same as at the last L1 response timestamp
                    latency = packet->cycle + node->core.netrace.behind - node->cycle;
                    if (latency<0) latency=0;
                } else {
                    // response from L1:
                    // remember how many cycles we are behind the trace schedule.
                    node->core.netrace.behind = node->cycle - packet->cycle;
                    if (node->cycle < packet->cycle)
                        node->core.netrace.behind = 0;
                }
            }
            if (packet->cycle > node->cycle+latency)
                latency = packet->cycle - node->cycle;
                // wait at least until the timestamp from the trace is reached



//printf("$%02lx #%lu id=%d lat=%ld behind=%ld\n", 
//    node->rank, node->cycle, packet->id, latency, node->core.netrace.behind);

            if (latency>0) return latency;

        // stop simulating this node, if more cycles than the trace length
        // were simulated and there are no more packets pending or to inject.
//        } else if (node->cycle > netrace_length) {
        } else if (netrace_injected == netrace_header->num_cycles && 
            packets_on_the_fly==0)
        {
            node->state = CS_STOPPED;
            return IC_STOP;
        }

    }


    // continue sending a packet of multiple flits
    if (node->core.netrace.pending_len > 1) {
        // only the last flit contains the pointer to the packet
        if (node->noc_send_flit(node, node->core.netrace.pending_dest, 
            0/*node->core.netrace.pending_len*/)) 
        {
            node->core.netrace.pending_len--;
        }
    } else if (node->core.netrace.pending_len == 1) {
        if (node->noc_send_flit(node,
            node->core.netrace.pending_dest,
            node->core.netrace.pending_flit))
        {

            assert(node->core.netrace.pending_flit != 0);

//printf("Inject %ld id %d\n", node->cycle, 
//((nt_packet_t*)node->core.netrace.pending_flit)->id);

            netrace_injected++;
            node->core.netrace.pending_len = 0;
        }
    }

    return 1; // one cycle
}


// init context
void netrace_init_context(node_t *node)
{
    node->cycle = 0;
    node->retired = 0;
    node->state = CS_RUNNING;
    node->pc = 0;
    node->core_type = CT_netrace;
    node->one_cycle = &netrace_one_cycle;

    node->core.netrace.injqueue_head = 0;
    node->core.netrace.injqueue_tail = 0;
    node->core.netrace.behind = 0;
    node->core.netrace.pending_len = 0;
}



void netrace_finish_context(node_t *node)
{
    if (node->rank == 0) nt_close_trfile();

    // remove linked list of flits to send
    flit_queue_entry_t *p=node->core.netrace.injqueue_head;
    while (p) {
        flit_queue_entry_t *n = p->next;
        free(p);
        p = n;
    }
}



void netrace_print_context(node_t *node)
{
    if (node->rank==0) {
        fprintf(stderr, "#%lu inj: %lu/%llu not ej: %lu active: %llu fly: %lu\n",
            node->cycle, netrace_injected, netrace_header->num_packets,
            netrace_injected-netrace_ejected, nt_num_active_packets,
            packets_on_the_fly);
/*
        fprintf(stderr, "  injqueue $20: ");
        flit_queue_entry_t *n = nodes[20]->core.netrace.injqueue_head;
        while (n) {
            fprintf(stderr, " %d", ((nt_packet_t *)n->flit)->id);
            n = n->next;
        }
        fprintf(stderr, "\n");
*/
    }

}


void netrace_open_file(const char *filename)
{
    nt_open_trfile(filename);
    netrace_header  = nt_get_trheader();
    if (netrace_header->num_nodes != conf_max_rank)
        fatal("This netrace requires %d nodes, but %d available",
            netrace_header->num_nodes, conf_max_rank);

    packets_on_the_fly = 0; // must be done before init()
    init();
    netrace_injected = 0;
    netrace_ejected = 0;
}



