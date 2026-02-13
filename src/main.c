#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include "logger.h"
#include "cpu.h"
#include "mem.h"
#include "process.h"
#include "display.h"
#include "sort.h"
#include "system.h"
#include "input.h"

#define REFRESH_INTERVAL_MS 800 // 1000 is max, after 1000 will be overflow

int main(void)
{
	log_info("Process monitor started");

	int cpu_cores = get_cpu_cores();
	if (cpu_cores == -1) {
		log_fatal("Failed to get CPU core count");
		return 1;
	}

	uint64_t total_mem_bytes = read_total_mem_bytes();
	if (total_mem_bytes == 0) {
		log_fatal("Failed to read total memory bytes");
		return 1;
	}

	char init_msg[256];
	snprintf(init_msg, sizeof(init_msg),
		 "System initialized: %d cores, %lu MB RAM",
		 cpu_cores, total_mem_bytes / (1024 * 1024));
	log_info(init_msg);


	display_init();
	InputState input_state;
	input_init(&input_state);

	// Use malloc instead of stack to avoid overflow with large arrays
	ProcessInfo *prev_processes = malloc(MAX_PROCESSES * sizeof(ProcessInfo));
	ProcessInfo *curr_processes = malloc(MAX_PROCESSES * sizeof(ProcessInfo));

	if (!prev_processes || !curr_processes) {
		log_fatal("Failed to allocate memory for process arrays");
		display_cleanup();
		free(prev_processes);
		free(curr_processes);
		return 1;
	}

	int prev_count = collect_processes(prev_processes, MAX_PROCESSES);
	long total_cpu_prev = read_total_cpu_time();
	long active_cpu_prev = read_active_cpu_time();

	bool first_iteration = true;
	while (!input_state.should_exit) {
		if (first_iteration) {
			first_iteration = false;
		} else {
			// wait 1 sec in total, track user input 10 times/sec
			for (int i = 0; i < 10; i++) {
				input_handle(&input_state, curr_processes, prev_count);
				struct timespec ts = {0, REFRESH_INTERVAL_MS * 100000}; // 100ms
				nanosleep(&ts, NULL);
			}
		}

		long total_cpu_curr = read_total_cpu_time();
		long active_cpu_curr = read_active_cpu_time();
		int curr_count = collect_processes(curr_processes,
						   MAX_PROCESSES);

		uint64_t total_cpu_delta = total_cpu_curr - total_cpu_prev;
		uint64_t active_cpu_delta = active_cpu_curr - active_cpu_prev;

		compute_process_stats(curr_processes, curr_count,
				      prev_processes, prev_count,
				      total_cpu_delta, total_mem_bytes);

		// Check for conflicting sort flags
		if (input_state.sort_cpu && input_state.sort_mem) {
			log_warning("Both sort_cpu and sort_mem are true; using CPU sort");
			input_state.sort_mem = false;
		}

		// Sort processes
		if (input_state.sort_cpu) {
			sort_by_cpu(curr_processes, curr_count,
				    input_state.reversed);
		} else if (input_state.sort_mem) {
			sort_by_mem(curr_processes, curr_count,
				    input_state.reversed);
		}

		// Calculate CPU load (use active_cpu_delta for load)
		double cpu_load = calculate_cpu_load(active_cpu_delta,
						      REFRESH_INTERVAL_MS,
						      cpu_cores);

		// Read actual system memory usage (not sum of all processes!)
		uint64_t used_mem_bytes = read_used_mem_bytes();
		uint64_t used_mem_mb = used_mem_bytes / (1024 * 1024);
		uint64_t total_mem_mb = total_mem_bytes / (1024 * 1024);

		// Get uptime
		int days, hours, minutes;
		read_uptime(&days, &hours, &minutes);

		// Clear and display
		clear();
		display_header(days, hours, minutes, cpu_load,
			       used_mem_mb, total_mem_mb, curr_count,
			       input_state.sort_cpu, input_state.sort_mem,
			       input_state.reversed);
		display_process_info(curr_processes, curr_count,
				     input_state.scroll_offset,
				     input_state.search_term);
		display_refresh();

		memcpy(prev_processes, curr_processes,
		       curr_count * sizeof(ProcessInfo));
		prev_count = curr_count;
		total_cpu_prev = total_cpu_curr;
		active_cpu_prev = active_cpu_curr;
	}

	display_cleanup();
	free(prev_processes);
	free(curr_processes);
	log_info("Process monitor stopped");
	return 0;
}
