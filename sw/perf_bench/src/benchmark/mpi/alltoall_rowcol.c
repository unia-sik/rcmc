#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 1

void collect_col(int* buffer, fgmp_info_t* info) {
    int x = fgmp_addr_x(info->address);
    int y = fgmp_addr_y(info->address);

    for (int i = 1; i < info->height; i++) {
        int src = fgmp_addr_gen(x, ((y + info->height - i) % info->height));
        int dest = fgmp_addr_gen(x, ((y + i) % info->height));

        fgmp_srdy(src);
        
/*        
        for (int j = 0; j < local_size * info->width * sizeof(int); j+=256) {
                fgmp_srdy(src);
                fgmp_block_send(
                    dest,
                    256,
                    buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
                );                
                fgmp_block_recv_no_srdy(
                    src,
                    256,
                    buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
                );
        }
        */
        
        fgmp_block_send_recv_no_srdy(
                dest,
                src,
                local_size * info->width * sizeof(int),
                buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest),
                buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
        );

//         if (src == dest) {            
//             if (info->address < dest) {
//                 fgmp_block_send_no_srdy(
//                     dest,
//                     local_size * info->width * sizeof(int),
//                     buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                 );                
//                 fgmp_block_recv_no_srdy(
//                     src,
//                     local_size * info->width * sizeof(int),
//                     buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                 );
//             } else {
//                 fgmp_block_recv_no_srdy(
//                     src,
//                     local_size * info->width * sizeof(int),
//                     buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                 );
//                 fgmp_block_send_no_srdy(
//                     dest,
//                     local_size * info->width * sizeof(int),
//                     buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                 );
//             }
//         } else {
//             if ((info->rank & 1) == 0) {
//                 if (info->address < dest) {
//                     fgmp_block_send_no_srdy(
//                         dest,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                     );
//                     fgmp_block_recv_no_srdy(
//                         src,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                     );                    
//                 } else {
//                     fgmp_block_recv_no_srdy(
//                         src,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                     );
//                     fgmp_block_send_no_srdy(
//                         dest,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                     );                    
//                 }
//             } else {
//                 if (info->address < dest) {
//                     fgmp_block_recv_no_srdy(
//                         src,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                     );
//                     fgmp_block_send_no_srdy(
//                         dest,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                     );                    
//                 } else {
//                     fgmp_block_send_no_srdy(
//                         dest,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(dest)
//                     );
//                     fgmp_block_recv_no_srdy(
//                         src,
//                         local_size * info->width * sizeof(int),
//                         buffer + sizeof(int) * local_size * info->width * fgmp_addr_y(src)
//                     );                    
//                 }
//             }            
//         }
    }
}



void collect_row(int* buffer, fgmp_info_t* info) {
    int x = fgmp_addr_x(info->address);
    int y = fgmp_addr_y(info->address);

    for (int i = 1; i < info->width; i++) {
        int src = fgmp_addr_gen(((x + info->width - i) % info->width), y);
        int dest = fgmp_addr_gen(((x + i) % info->width), y);

        fgmp_srdy(src);

        for (int j = 0; j < info->height; j++) {
            fgmp_block_send_recv_no_srdy(
                dest,
                src,
                local_size * sizeof(int),
                buffer + local_size * sizeof(int) * (info->width * j + fgmp_addr_x(dest)),
                buffer + local_size * sizeof(int) * (info->width * j + fgmp_addr_x(dest))
            );
        }
    }
}

void sort_recv_data_chunk(int* buffer, fgmp_info_t* info, int left, int right) {
    int n = local_size * sizeof(int);
            
    while (left < right) {
        
        for (int k = 0; k < n; k++) {
            int tmp = ((char*)buffer)[left * n + k];
            ((char*)buffer)[left * n + k] = ((char*)buffer)[right * n + k];
            ((char*)buffer)[right * n + k] = tmp;
        }
        
        
        left++;
        right--;
    }
}

void sort_recv_data(int* buffer, fgmp_info_t* info) {
    int zeroPos = (fgmp_addr_x(info->rank) * 2) % info->width;
    for (int i = 0; i < info->height; i++) {
        sort_recv_data_chunk(
            buffer + local_size * sizeof(int) * info->width * i,
            info,
            0,
            zeroPos
        );
        
        sort_recv_data_chunk(
            buffer + local_size * sizeof(int) * info->width * i,
            info,
            zeroPos + 1,
            info->width - 1
        ); 
    }
}

int main()
{
    fgmp_info_t info = fgmp_info();

    int* local = (void*)0x20000;    
    
    collect_col(local, &info);
    collect_row(local, &info);
    sort_recv_data(local, &info);
    
    return 0;
}





