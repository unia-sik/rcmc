#include <stdint.h>

void exit();
void assert(uint64_t status);
void set_test_result(uint64_t code);

void delay(int n);

uint64_t clock();

uint64_t get_cid();
uint64_t get_max_cid();
uint64_t get_dim();
