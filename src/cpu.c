#include <stdio.h>
#include <unistd.h>
#include "logger.h"
#include "cpu.h"

/**
 * read_total_cpu_time() - Read total CPU time across all cores
 *
 * Reads /proc/stat to sum all CPU time counters (user, nice, system, idle,
 * iowait, irq, softirq, steal) from the first line.
 *
 * Return: Total CPU time in jiffies, or -1 on error
 */
long read_total_cpu_time(void)
{
	FILE *f = fopen("/proc/stat", "r");
	if (!f) {
		log_error("Failed to open /proc/stat");
		return -1;
	}

    char cpu[5];
    long user, nice, system, idle, iowait, irq, softirq, steal;

    int n = fscanf(
        f,
        "%4s %ld %ld %ld %ld %ld %ld %ld %ld",
        cpu,
        &user,
        &nice,
        &system,
        &idle,
        &iowait,
        &irq,
        &softirq,
        &steal
    );

    fclose(f);

    if (n < 9) {
        log_error("Failed to read total CPU time");
        return -1;
    }

	return user + nice + system + idle + iowait + irq + softirq + steal;
}

/**
 * read_active_cpu_time() - Read active (non-idle) CPU time
 *
 * Reads /proc/stat and returns only active time (excludes idle and iowait).
 * Used for calculating CPU load percentage.
 *
 * Return: Active CPU time in jiffies, or -1 on error
 */
long read_active_cpu_time(void)
{
	FILE *f = fopen("/proc/stat", "r");
	if (!f) {
		log_error("Failed to open /proc/stat");
		return -1;
	}

	char cpu[5];
	long user, nice, system, idle, iowait, irq, softirq, steal;

	int n = fscanf(f, "%4s %ld %ld %ld %ld %ld %ld %ld %ld",
		       cpu, &user, &nice, &system, &idle,
		       &iowait, &irq, &softirq, &steal);

	fclose(f);

	if (n < 9) {
		log_error("Failed to read active CPU time");
		return -1;
	}

	return user + nice + system + irq + softirq + steal;
}

/**
 * get_cpu_cores() - Get the number of online CPU cores
 *
 * Uses sysconf() to determine the number of processors currently online.
 *
 * Return: Number of CPU cores, or -1 on error 
 */
int get_cpu_cores(void)
{
	long cores = sysconf(_SC_NPROCESSORS_ONLN);
	if (cores <= 0) {
		log_error("Failed to get CPU core count");
		return -1;
	}
	return (int)cores;
}

