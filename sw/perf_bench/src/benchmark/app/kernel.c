#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include <stdint.h>

#define local_size 32
#define num_iterations 10

//dont change
#define kernel_size 3


void init_kernel(int* kernel) {
    kernel[0] = 1;
    kernel[1] = 2;
    kernel[2] = 1;

    kernel[3] = 2;
    kernel[4] = 4;
    kernel[5] = 2;

    kernel[6] = 1;
    kernel[7] = 2;
    kernel[8] = 1;
}

void apply_kernel(int* src, int* dest, int* kernel) {
    for (int y = 0; y < local_size; y++) {
        for (int x = 0; x < local_size; x++) {
            int sum = 0;
            for (int ofy = -1; ofy != 2; ofy++) {
                for (int ofx = -1; ofx != 2; ofx++) {
                    sum += src[
                               local_size + 2 +//ignore first row
                               1 + //start with second column
                               (y - ofy) *
                               (local_size + 2) +
                               x +
                               ofx
                           ];
                }
            }

            dest[(y + 1) * (local_size + 2) + (x + 1)] = sum / 16;
        }
    }
}

void send_and_recv_vertical_block(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (y != (dimension - 1)) {        
        fgmp_srdy(fgmp_addr_gen(x, y + 1));
        fgmp_block_send_no_srdy(fgmp_addr_gen(x, y + 1), local_size * sizeof(int), src + local_size + 2 + 1);
        fgmp_block_recv_no_srdy(fgmp_addr_gen(x, y + 1), local_size * sizeof(int), src + 1);
    }
}

void recv_and_send_vertical_block(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (y != 0) {        
        fgmp_srdy(fgmp_addr_gen(x, y - 1));
        fgmp_block_recv_no_srdy(fgmp_addr_gen(x, y - 1), local_size * sizeof(int), src + (local_size + 2) * (local_size + 1) + 1);
        fgmp_block_send_no_srdy(fgmp_addr_gen(x, y - 1), local_size * sizeof(int), src + (local_size + 2) * (local_size) + 1);
    }
}

void send_and_recv_horizontal_block(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (x != (dimension - 1)) {        
        fgmp_srdy(fgmp_addr_gen(x + 1, y));
        int border[local_size];

        for (int i = 0; i < local_size; i++) {
            border[i] = src[local_size + (i + 1) * (local_size + 2)];
        }

        fgmp_block_send_no_srdy(fgmp_addr_gen(x + 1, y), local_size * sizeof(int), border);
        fgmp_block_recv_no_srdy(fgmp_addr_gen(x + 1, y), local_size * sizeof(int), border);

        for (int i = 0; i < local_size; i++) {
            src[local_size + (i + 1) * (local_size + 2) + 1] = border[i];
        }
    }
}

void recv_and_send_horizontal_block(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (x != 0) {        
        fgmp_srdy(fgmp_addr_gen(x - 1, y));
        int border[local_size];

        fgmp_block_recv_no_srdy(fgmp_addr_gen(x - 1, y), local_size * sizeof(int), border);

        for (int i = 0; i < local_size; i++) {
            src[(i + 1) * (local_size + 2)] = border[i];
        }

        for (int i = 0; i < local_size; i++) {
            border[i] = src[(i + 1) * (local_size + 2) + 1];
        }

        fgmp_block_send_no_srdy(fgmp_addr_gen(x - 1, y), local_size * sizeof(int), border);
    }
}
void send_and_recv_vertical_corner(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);
    
    if (y != (dimension - 1)) {        
        fgmp_srdy(fgmp_addr_gen(x, y + 1));
        int corner[2];
        corner[0] = src[local_size + 2];
        corner[1] = src[local_size + 2 + local_size + 1];

        fgmp_block_send_no_srdy(fgmp_addr_gen(x, y + 1), 2 * sizeof(int), corner);
        fgmp_block_recv_no_srdy(fgmp_addr_gen(x, y + 1), 2 * sizeof(int), corner);

        src[0] = corner[0];
        src[local_size + 1] = corner[1];
    }
}

void recv_and_send_vertical_corner(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (y != 0) {        
        fgmp_srdy(fgmp_addr_gen(x, y - 1));
        int corner[2];

        fgmp_block_recv_no_srdy(fgmp_addr_gen(x, y - 1), 2 * sizeof(int), corner);

        src[(local_size + 2) * (local_size + 1)] = corner[0];
        src[(local_size + 2) * (local_size + 2) - 1] = corner[1];

        corner[0] = src[(local_size + 2) * (local_size)];
        corner[1] = src[(local_size + 2) * (local_size + 1) - 1];

        fgmp_block_send_no_srdy(fgmp_addr_gen(x, y - 1), 2 * sizeof(int), corner);
    }
}

void send_and_recv_horizontal_corner(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (x != (dimension - 1)) {        
        fgmp_srdy(fgmp_addr_gen(x + 1, y));
        int corner[2];

        corner[0] = src[local_size];
        corner[1] = src[local_size + (local_size + 1) * (local_size + 2)];

        fgmp_block_send_no_srdy(fgmp_addr_gen(x + 1, y), 2 * sizeof(int), corner);
        fgmp_block_recv_no_srdy(fgmp_addr_gen(x + 1, y), 2 * sizeof(int), corner);

        src[local_size + 1] = corner[0];
        src[local_size + (local_size + 1) * (local_size + 2) + 1] = corner[1];
    }
}

void recv_and_send_horizontal_corner(int* src, int core_id, int dimension) {
    int x = fgmp_addr_x(core_id);
    int y = fgmp_addr_y(core_id);

    if (x != 0) {
        fgmp_srdy(fgmp_addr_gen(x - 1, y));
        int corner[2];

        fgmp_block_recv_no_srdy(fgmp_addr_gen(x - 1, y), 2 * sizeof(int), corner);

        src[0] = corner[0];
        src[(local_size + 1) * (local_size + 2)] = corner[1];

        corner[0] = src[1];
        corner[1] = src[(local_size + 1) * (local_size + 2) + 1];

        fgmp_block_send_no_srdy(fgmp_addr_gen(x - 1, y), 2 * sizeof(int), corner);
    }
}

void send_borders(int* src) {
    fgmp_info_t info = fgmp_info();

    int x = fgmp_addr_x(info.address);
    int y = fgmp_addr_y(info.address);

    if ((y % 2) == 0) {
        send_and_recv_vertical_block(src, info.address, info.width);
    } else {
        recv_and_send_vertical_block(src, info.address, info.width);
    }

    if ((y % 2) == 1) {
        send_and_recv_vertical_block(src, info.address, info.width);
    } else {
        recv_and_send_vertical_block(src, info.address, info.width);
    }

    if ((x % 2) == 0) {
        send_and_recv_horizontal_block(src, info.address, info.height);
    } else {
        recv_and_send_horizontal_block(src, info.address, info.height);
    }

    if ((x % 2) == 1) {
        send_and_recv_horizontal_block(src, info.address, info.height);
    } else {
        recv_and_send_horizontal_block(src, info.address, info.height);
    }

    if ((y % 2) == 0) {
        send_and_recv_vertical_corner(src, info.address, info.width);
    } else {
        recv_and_send_vertical_corner(src, info.address, info.width);
    }

    if ((y % 2) == 1) {
        send_and_recv_vertical_corner(src, info.address, info.width);
    } else {
        recv_and_send_vertical_corner(src, info.address, info.width);
    }

    if ((x % 2) == 0) {
        send_and_recv_horizontal_corner(src, info.address, info.height);
    } else {
        recv_and_send_horizontal_corner(src, info.address, info.height);
    }

    if ((x % 2) == 1) {
        send_and_recv_horizontal_corner(src, info.address, info.height);
    } else {
        recv_and_send_horizontal_corner(src, info.address, info.height);
    }
}

void init_data(int* A) {
    int core_id = fgmp_core_id();

    for (int y = 0; y < local_size; y++) {
        for (int x = 0; x < local_size; x++) {
            A[(y + 1) * (local_size + 2) + (x + 1)] = core_id;
        }
    }
}

int main() {
    int* A = (void*)0x20000;
    int* B = A + (local_size + 2) * (local_size + 2);

    int kernel[kernel_size * kernel_size];
    init_kernel(kernel);

    init_data(A);

    for (int i = 0; i < num_iterations; i++) {
        send_borders(A);
        apply_kernel(A, B, kernel);
    }

    return 0;
}

