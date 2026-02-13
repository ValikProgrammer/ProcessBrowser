#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "process.h"

void display_init(void);
void display_cleanup(void);
void display_header(int days, int hours, int minutes, double cpu_load,
		    uint64_t used_mem_mb, uint64_t total_mem_mb,
		    int process_count, bool sort_cpu, bool sort_mem,
		    bool reversed);
void display_process_info(ProcessInfo *processes, int count, int scroll_offset,
			  const char *search_term);
void display_refresh(void);

#endif


