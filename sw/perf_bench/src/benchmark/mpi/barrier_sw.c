#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"

#include <stdint.h>
#include <stdio.h>

void send_row(fgmp_info_t* info)
{    
    int x = fgmp_addr_x(info->address);
    int y = fgmp_addr_y(info->address);
    
    for (int i = 1; i < info->width; i *= 2) {
        int src = fgmp_addr_gen((x + info->width - i) % info->width, y) + info->root;
        int dest = fgmp_addr_gen((x + i) % info->width, y) + info->root;
        fgmp_srdy(src);      
        
//         fgmp_bnr(dest);
        fgmp_bsf();        
        fgmp_snd(dest, 0xAC);
        fgmp_bre();
        fgmp_rcvp();
    }    
}

void send_col(fgmp_info_t* info)
{    
    int x = fgmp_addr_x(info->address);
    int y = fgmp_addr_y(info->address);
    
    for (int i = 1; i < info->height; i *= 2) {        
        int src = fgmp_addr_gen(x, ((y + info->height - i) % info->height)) + info->root;
        int dest = fgmp_addr_gen(x, ((y + i) % info->height)) + info->root;
        fgmp_srdy(src);      
        
//         fgmp_bnr(dest);
        fgmp_bsf();
        fgmp_snd(dest, 0xAC);
        fgmp_bre();
        fgmp_rcvp();
    }    
}

int main()
{
    fgmp_info_t info = fgmp_info();
    
    send_row(&info);
    send_col(&info);

    return 0;
}

