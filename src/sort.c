#include <stdlib.h>
#include "sort.h"

static int compare_cpu_asc(const void *a, const void *b)
{
	const ProcessInfo *p1 = (const ProcessInfo *)a;
	const ProcessInfo *p2 = (const ProcessInfo *)b;

	if (p1->cpu_percent < p2->cpu_percent)
		return -1;
	if (p1->cpu_percent > p2->cpu_percent)
		return 1;
	return 0;
}

static int compare_cpu_desc(const void *a, const void *b)
{
	return -compare_cpu_asc(a, b);
}

static int compare_mem_asc(const void *a, const void *b)
{
	const ProcessInfo *p1 = (const ProcessInfo *)a;
	const ProcessInfo *p2 = (const ProcessInfo *)b;

	if (p1->mem_percent < p2->mem_percent)
		return -1;
	if (p1->mem_percent > p2->mem_percent)
		return 1;
	return 0;
}

static int compare_mem_desc(const void *a, const void *b)
{
	return -compare_mem_asc(a, b);
}

/**
 * sort_by_cpu() - Sort processes by CPU usage
 * @processes: Array of ProcessInfo structures
 * @count: Number of processes in the array
 * @reversed: If false (default), biggest values first; if true, smallest first
 *
 * Sorts the process array in-place by CPU percentage.
 */
void sort_by_cpu(ProcessInfo *processes, int count, bool reversed)
{
	if (reversed) {
		qsort(processes, count, sizeof(ProcessInfo), compare_cpu_asc);
	} else {
		qsort(processes, count, sizeof(ProcessInfo), compare_cpu_desc);
	}
}

/**
 * sort_by_mem() - Sort processes by memory usage
 * @processes: Array of ProcessInfo structures
 * @count: Number of processes in the array
 * @reversed: If false (default), biggest values first; if true, smallest first
 *
 * Sorts the process array in-place by memory percentage.
 */
void sort_by_mem(ProcessInfo *processes, int count, bool reversed)
{
	if (reversed) {
		qsort(processes, count, sizeof(ProcessInfo), compare_mem_asc);
	} else {
		qsort(processes, count, sizeof(ProcessInfo), compare_mem_desc);
	}
}

