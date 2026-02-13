#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 4096

#include <stdbool.h>
#include <stdint.h>
typedef struct {
    // mem and cpu in percents only 
    int pid;
    char name[256];
    char cmdline[512]; 

    bool cpu_valid;
    bool mem_valid;
    bool cmd_valid;
    long rss_kb;


    // CPU usage
    uint64_t utime;   // user time (jiffies)
    uint64_t stime;   // system time (jiffies)
    double cpu_percent;    // calculated percentage

    // Memory usage
    uint64_t mem_bytes;   // absolute memory (bytes) of a process, not total system RAM
    double mem_percent;        // relative to total system RAM
} ProcessInfo;

void compute_process_stats(
    ProcessInfo *curr, int curr_count,
    ProcessInfo *prev, int prev_count,
    uint64_t total_cpu_delta,
    uint64_t total_mem_bytes
);

int read_process(int pid, ProcessInfo *p);
int collect_processes(ProcessInfo *list, int max);

#endif