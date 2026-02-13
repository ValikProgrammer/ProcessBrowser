#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/include/process.h"
#include "../src/include/sort.h"

// Test data: synthetic ProcessInfo arrays
static void create_test_data(ProcessInfo *procs, int count)
{
	for (int i = 0; i < count; i++) {
		procs[i].pid = 1000 + i;
		snprintf(procs[i].name, sizeof(procs[i].name), "proc%d", i);
		procs[i].cpu_percent = (double)(count - i) * 10.0;
		procs[i].mem_percent = (double)(i + 1) * 5.0;
		procs[i].cpu_valid = true;
		procs[i].mem_valid = true;
		procs[i].mem_bytes = (i + 1) * 1024 * 1024;
		procs[i].cmdline[0] = '\0';
		procs[i].cmd_valid = false;
	}
}

// Test: sort_by_cpu descending (default)
static int test_sort_cpu_desc(void)
{
	ProcessInfo procs[5];
	create_test_data(procs, 5);

	sort_by_cpu(procs, 5, false);

	// After sort: highest CPU first
	for (int i = 0; i < 4; i++) {
		if (procs[i].cpu_percent < procs[i + 1].cpu_percent) {
			fprintf(stderr, "FAIL: sort_cpu_desc - order incorrect\n");
			return 1;
		}
	}

	printf("PASS: sort_cpu_desc\n");
	return 0;
}

// Test: sort_by_cpu ascending (reversed)
static int test_sort_cpu_asc(void)
{
	ProcessInfo procs[5];
	create_test_data(procs, 5);

	sort_by_cpu(procs, 5, true);

	// After sort: lowest CPU first
	for (int i = 0; i < 4; i++) {
		if (procs[i].cpu_percent > procs[i + 1].cpu_percent) {
			fprintf(stderr, "FAIL: sort_cpu_asc - order incorrect\n");
			return 1;
		}
	}

	printf("PASS: sort_cpu_asc\n");
	return 0;
}

// Test: sort_by_mem descending (default)
static int test_sort_mem_desc(void)
{
	ProcessInfo procs[5];
	create_test_data(procs, 5);

	sort_by_mem(procs, 5, false);

	// After sort: highest MEM first
	for (int i = 0; i < 4; i++) {
		if (procs[i].mem_percent < procs[i + 1].mem_percent) {
			fprintf(stderr, "FAIL: sort_mem_desc - order incorrect\n");
			return 1;
		}
	}

	printf("PASS: sort_mem_desc\n");
	return 0;
}

// Test: sort_by_mem ascending (reversed)
static int test_sort_mem_asc(void)
{
	ProcessInfo procs[5];
	create_test_data(procs, 5);

	sort_by_mem(procs, 5, true);

	// After sort: lowest MEM first
	for (int i = 0; i < 4; i++) {
		if (procs[i].mem_percent > procs[i + 1].mem_percent) {
			fprintf(stderr, "FAIL: sort_mem_asc - order incorrect\n");
			return 1;
		}
	}

	printf("PASS: sort_mem_asc\n");
	return 0;
}

int main(void)
{
	int failures = 0;

	printf("Running unit tests for sorting...\n");

	failures += test_sort_cpu_desc();
	failures += test_sort_cpu_asc();
	failures += test_sort_mem_desc();
	failures += test_sort_mem_asc();

	if (failures == 0) {
		printf("All sorting tests passed.\n");
		return 0;
	} else {
		fprintf(stderr, "%d test(s) failed.\n", failures);
		return 1;
	}
}
