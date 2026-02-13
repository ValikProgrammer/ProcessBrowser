#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void read_uptime(int *days, int *hours, int *minutes);
double calculate_cpu_load(uint64_t total_cpu_delta, long interval_ms,
			  int cpu_cores);

#endif

