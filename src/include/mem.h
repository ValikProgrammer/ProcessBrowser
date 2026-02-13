#ifndef MEM_H
#define MEM_H

#include <stdint.h>

uint64_t read_total_mem_bytes(void);
uint64_t read_used_mem_bytes(void);
uint64_t get_page_size(void);

#endif
