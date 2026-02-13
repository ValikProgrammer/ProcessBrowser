#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"
#include "mem.h"

/**
 * read_total_mem_bytes() - Read total system memory in bytes
 *
 * Reads /proc/meminfo to obtain MemTotal value.
 *
 * Return: Total memory in bytes, or 0 on error
 */
uint64_t read_total_mem_bytes(void)
{
	FILE *f = fopen("/proc/meminfo", "r");
	if (!f) {
		log_error("Failed to open /proc/meminfo");
		return 0;
	}

	char line[256];
	uint64_t mem_total_kb = 0;

	while (fgets(line, sizeof(line), f)) {
		if (sscanf(line, "MemTotal: %lu kB", &mem_total_kb) == 1) {
			break;
		}
	}

	fclose(f);

	if (mem_total_kb == 0) {
		log_error("Failed to read MemTotal from /proc/meminfo");
		return 0;
	}

	return mem_total_kb * 1024;
}

/**
 * read_used_mem_bytes() - Read used system memory from /proc/meminfo
 *
 * Calculates used memory as: MemTotal - MemAvailable
 * This is the correct way to measure system memory usage.
 *
 * Return: Used system memory in bytes on success, 0 on failure.
 */
uint64_t read_used_mem_bytes(void)
{
	FILE *f = fopen("/proc/meminfo", "r");
	if (!f) {
		log_error("Failed to open /proc/meminfo");
		return 0;
	}

	char line[256];
	uint64_t mem_total_kb = 0;
	uint64_t mem_available_kb = 0;
	bool found_total = false;
	bool found_available = false;

	while (fgets(line, sizeof(line), f)) {
		if (sscanf(line, "MemTotal: %lu kB", &mem_total_kb) == 1) {
			found_total = true;
		} else if (sscanf(line, "MemAvailable: %lu kB", &mem_available_kb) == 1) {
			found_available = true;
		}

		if (found_total && found_available) {
			break;
		}
	}
	fclose(f);

	if (!found_total || !found_available) {
		log_error("Failed to read memory info from /proc/meminfo");
		return 0;
	}

	// Used = Total - Available
	uint64_t used_kb = mem_total_kb - mem_available_kb;
	return used_kb * 1024; // Convert KB to bytes
}

/**
 * get_page_size() - Get system page size
 *
 * Returns the system page size in bytes using sysconf().
 *
 * Return: Page size in bytes
 */
uint64_t get_page_size(void)
{
	long page_size = sysconf(_SC_PAGESIZE);
	if (page_size <= 0) {
		log_error("Failed to get page size");
		return 4096; // fallback to common value
	}
	return (uint64_t)page_size;
}

