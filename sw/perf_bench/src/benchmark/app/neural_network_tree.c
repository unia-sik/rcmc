#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include "debug.h"

#include <stdint.h>

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

void recv_from_prev_layer(const int x, const int y, const int dimension, int* values) {
    int src = fgmp_addr_gen(x - 1, y);
    int* data = values + block_width * y;
    int size = block_width * sizeof(int);
    
    fgmp_block_recv(src, size, data);   
    
    int recvData[dimension];
    
    for (int i = 0; i < dimension; i++) {
        recvData[i] = 0;
    }
    
    recvData[y] = 1;
    
    for (int i = 1; i < dimension; i *= 2) {
        int src = fgmp_addr_gen(x, (y + dimension - i) % dimension);
        int dest = fgmp_addr_gen(x, (y + i) % dimension);
        
        fgmp_srdy(src);      
        
        if ((y & i) == 0) {
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_width * sizeof(int), values + block_width * k);
                }
            }
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_width * sizeof(int), values + block_width * k);
            }
        } else {
            for (int j = 0; j < i; j++) {
                fgmp_bre();
                int k = fgmp_rcvp();
                if (recvData[k] == 0) {
                    recvData[k] = i + 1;
                }
                fgmp_block_recv_no_srdy(src, block_width * sizeof(int), values + block_width * k);
            }
            for (int k = 0; k < dimension; k++) {
                if (0 < recvData[k] && recvData[k] <= i) {
                    fgmp_bsf();
                    fgmp_snd(dest, k);
                    fgmp_block_send_no_srdy(dest, block_width * sizeof(int), values + block_width * k);
                }
            }
        }
    }
    
    
}

void send_to_next_layer(const int x, const int y, const int dimension, int* values) {
    int dest = fgmp_addr_gen(x + 1, y);    
    int size = block_width * sizeof(int);
    
    fgmp_block_send(dest, size, values);
}

int calc_single_output(const int n, int* values, int* weights, const int bias) {
    int result = bias;
    
    for (int i = 0; i < n; i++) {
        result += values[i] * weights[i];
    }
    
    //activate functions are not possible without floats
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
            recv_from_prev_layer(x, y, info.width, edges);
            
            for (int i = 0; i < block_width; i++) {
                local[i] = calc_single_output(block_width * info.width, edges, weights + i * (block_width * info.width), bias);
            }
        }
        if (x < info.width - 1) {
            send_to_next_layer(x, y, info.width, local);
        }       
    }
    
    return 0;
}



