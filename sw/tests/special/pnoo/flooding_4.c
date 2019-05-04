#include "debug.h"
#include "pnoo.h"

#define block_size 256
     
int main()
{
    pnoo_info_t info = pnoo_info();
    
    int x = pnoo_addr_x(info.address);
    int y = pnoo_addr_y(info.address);    
    int destination = pnoo_addr_gen((info.width - x - 1),  (info.height - y - 1));  
    
    char blockA[block_size];
    char blockB[block_size];
    
    for (int i = 0; i < block_size; i++) {
        blockA[i] = (info.address ^ i) & 255;
    }
    
    if (destination != info.address) {
        pnoo_block_send_recv(destination, destination, block_size, blockA, blockB);
        
        for (int i = 0; i < block_size; i++) {
            char c = (destination ^ i) & 255;
            assert(blockB[i] == c);
        }
    }  
    
    pnoo_brne();
    
    return 0;
}











