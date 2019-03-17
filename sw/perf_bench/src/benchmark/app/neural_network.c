#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include "debug.h"

#define iterations 4
#define block_width 32

/*
 * Simulates the traffic of a small neural network (feed forward)
 * Each core-column is a unique layer and owns 'block_width' Nodes
 * Hence each layer has 'block_width' * dimension Nodes
 * 
 * The network is fully connected. Therefore each node sends
 * its local 'block_width' data to all cores in the next layer
 * 
 * There is no real input or any learning, so the first row just use undefined values.
 *
 */

void recv_from_prev_layer(const int x, const int dimension, int* values) {
    for (int i = 0; i < dimension; i++) {
        int src = fgmp_addr_gen(x - 1, i);        
        int* data = values + block_width * i;
        int size = block_width * sizeof(int);
        
        fgmp_block_recv(src, size, data);
    }  
}

void send_to_next_layer(const int x, const int dimension, int* values) {
    for (int i = 0; i < dimension; i++) {
        int dest = fgmp_addr_gen(x + 1, i);        
        int size = block_width * sizeof(int);
                
        //bnr is neccessary here, since overlapping causes problems
        fgmp_block_send(dest, size, values);
    }  
}

int calc_single_output(const int n, int* values, int* weights, const int bias) {
    int result = bias;
    
    for (int i = 0; i < n; i++) {
        result += values[i] * weights[i];
    }
    
    //floats would be a cool thing here...
    return result;
}


int main()
{
    fgmp_info_t info = fgmp_info();

    
    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);
    
    int bias = 17;
    int* local = (int*)0x20000; //yeah, thats how you should allocate memory; straight into the memory
    int* edges = local + block_width;
    int* weights = edges + block_width * info.width;
    
    for (int it = 0; it < iterations; it++) {
        if (x > 0) {
            recv_from_prev_layer(x, info.width, edges);
            
            for (int i = 0; i < block_width; i++) {
                local[i] = calc_single_output(block_width * info.width, edges, weights + i * (block_width * info.width), bias);
            }
        }
        if (x < info.width - 1) {
            send_to_next_layer(x, info.width, local);
        }       
    }
    
    return 0;
}


