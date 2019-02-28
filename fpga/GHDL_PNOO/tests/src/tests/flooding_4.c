#include "test_suite.h"
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#define block_size 256
     
int main()
{
    fgmp_info_t info = fgmp_info();
    
    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);    
    int destination = fgmp_addr_gen((info.width - x - 1),  (info.height - y - 1));  
    
    char blockA[block_size];
    char blockB[block_size];
    
    for (int i = 0; i < block_size; i++) {
        blockA[i] = (info.address ^ i) & 255;
    }
    
    if (destination != info.address) {
        fgmp_block_send_recv(destination, destination, block_size, blockA, blockB);
        
        for (int i = 0; i < block_size; i++) {
            char c = (destination ^ i) & 255;
            assert(blockB[i] == c);
        }
    }  
    
    fgmp_brne();
    
    return 0;
}











