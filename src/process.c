#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "logger.h"
#include "process.h"
#include "mem.h"

/**
 * read_cmdline() - Read process command line from /proc/[pid]/cmdline
 * @pid: Process ID
 * @buf: Output buffer
 * @size: Buffer size
 *
 * Reads cmdline and replaces null bytes with spaces.
 *
 * Return: 0 on success, -1 on error
 */
static int read_cmdline(int pid, char *buf, size_t size)
{
	char path[64];
	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	ssize_t n = read(fd, buf, size - 1);
	close(fd);

	if (n <= 0) {
		return -1;
	}

	// Replace null bytes with spaces
	for (ssize_t i = 0; i < n - 1; i++) {
		if (buf[i] == '\0') {
			buf[i] = ' ';
		}
	}
	buf[n] = '\0';

	return 0;
}

/**
 * read_process() - Read process information from /proc/[pid]/stat
 * @pid: Process ID to read
 * @p: Pointer to ProcessInfo structure to fill
 *
 * Parses /proc/[pid]/stat to extract pid, name, utime, stime, and rss.
 * Sets cpu_valid and mem_valid to false; these are computed later.
 *
 * Return: 0 on success, -1 on error
 */
int read_process(int pid, ProcessInfo *p)
{
	char path[256];
	snprintf(path, sizeof(path), "/proc/%d/stat", pid);

	FILE *f = fopen(path, "r");
	if (!f) {
		return -1;
    }
    
	char comm[256];
	int read_pid;
	char state;
	unsigned long utime, stime, rss;

	// Parse /proc/[pid]/stat fields up to rss (field 24)
	int n = fscanf(f,
		       "%d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u "
		       "%lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %lu",
		       &read_pid, comm, &state, &utime, &stime, &rss);

	fclose(f);

	if (n < 6)
		return -1;

	p->pid = read_pid;

	// Strip parentheses from comm and safely copy to name (max 40 chars)
	size_t len = strlen(comm);
	if (len > 2 && comm[0] == '(' && comm[len - 1] == ')') {
		size_t name_len = len - 2;
		if (name_len > 40) {
			name_len = 40;
		}
		memcpy(p->name, comm + 1, name_len);
		p->name[name_len] = '\0';
	} else {
		size_t name_len = strlen(comm);
		if (name_len > 40) {
			name_len = 40;
		}
		memcpy(p->name, comm, name_len);
		p->name[name_len] = '\0';
	}

	p->utime = utime;
	p->stime = stime;
	p->rss_kb = rss * get_page_size() / 1024;
	p->mem_bytes = rss * get_page_size();

	p->cpu_valid = false;
	p->mem_valid = true;
	p->cpu_percent = 0.0;
	p->mem_percent = 0.0;

	// Read command line
	if (read_cmdline(read_pid, p->cmdline, sizeof(p->cmdline)) == 0) {
		p->cmd_valid = true;
	} else {
		p->cmdline[0] = '\0';
		p->cmd_valid = false;
	}

	return 0;
}

/**
 * collect_processes() - Collect all running processes
 * @list: Array of ProcessInfo to fill
 * @max: Maximum number of processes to collect
 *
 * Scans /proc for numeric directories and reads process info for each.
 *
 * Return: Number of processes collected
 */
int collect_processes(ProcessInfo *list, int max)
{
	DIR *dir = opendir("/proc");
	if (!dir) {
		log_error("Failed to open /proc");
		return 0;
	}

	struct dirent *entry;
	int count = 0;

	while ((entry = readdir(dir)) != NULL && count < max) {
		// Check if directory name is numeric (PID)
		if (!isdigit(entry->d_name[0]))
			continue;

		int pid = atoi(entry->d_name);
		if (read_process(pid, &list[count]) == 0) {
			count++;
		}
	}

	closedir(dir);
	return count;
}

/**
 * compute_process_stats() - Calculate CPU and memory percentages
 * @curr: Current process snapshot array
 * @curr_count: Number of processes in curr
 * @prev: Previous process snapshot array
 * @prev_count: Number of processes in prev
 * @total_cpu_delta: Total CPU time delta across all cores
 * @cpu_cores: Number of CPU cores
 * @total_mem_bytes: Total system memory in bytes
 *
 * For each process in curr, finds its previous state in prev and calculates
 * cpu_percent based on the delta in utime+stime relative to total_cpu_delta.
 * Also calculates mem_percent relative to total system memory.
 */
void compute_process_stats(ProcessInfo *curr, int curr_count,
			   ProcessInfo *prev, int prev_count,
			   uint64_t total_cpu_delta,
			   uint64_t total_mem_bytes)
{
	for (int i = 0; i < curr_count; i++) {
		// Find matching process in prev
		ProcessInfo *prev_proc = NULL;
		for (int j = 0; j < prev_count; j++) {
			if (prev[j].pid == curr[i].pid) {
				prev_proc = &prev[j];
				break;
			}
		}

		if (prev_proc && total_cpu_delta > 0) {
			uint64_t proc_cpu_delta =
				(curr[i].utime + curr[i].stime) -
				(prev_proc->utime + prev_proc->stime);

			// 100% means all cores fully loaded
			curr[i].cpu_percent =
				(double)proc_cpu_delta / (double)total_cpu_delta *
				100.0;
			curr[i].cpu_valid = true;
		} else {
			curr[i].cpu_percent = 0.0;
			curr[i].cpu_valid = false;
		}

		if (total_mem_bytes > 0) {
			curr[i].mem_percent =
				(double)curr[i].mem_bytes /
				(double)total_mem_bytes * 100.0;
			curr[i].mem_valid = true;
		} else {
			curr[i].mem_percent = 0.0;
			curr[i].mem_valid = false;
		}
	}
}
