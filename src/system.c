#include <stdio.h>
#include "system.h"
#include "logger.h"

/**
 * read_uptime() - Read system uptime from /proc/uptime
 * @days: Output parameter for days
 * @hours: Output parameter for hours
 * @minutes: Output parameter for minutes
 *
 * Reads /proc/uptime and converts to days, hours, and minutes.
 */
void read_uptime(int *days, int *hours, int *minutes)
{
	FILE *f = fopen("/proc/uptime", "r");
	if (!f) {
		log_error("Failed to open /proc/uptime");
		*days = 0;
		*hours = 0;
		*minutes = 0;
		return;
	}

	double uptime_seconds;
	if (fscanf(f, "%lf", &uptime_seconds) != 1) {
		log_error("Failed to read uptime");
		fclose(f);
		*days = 0;
		*hours = 0;
		*minutes = 0;
		return;
	}

	fclose(f);

	int total_seconds = (int)uptime_seconds;
	*days = total_seconds / (24 * 3600);
	total_seconds %= (24 * 3600);
	*hours = total_seconds / 3600;
	total_seconds %= 3600;
	*minutes = total_seconds / 60;
}

/**
 * calculate_cpu_load() - Calculate overall CPU load percentage
 * @total_cpu_delta: Delta of total CPU time
 * @interval_ms: Measurement interval in milliseconds
 * @cpu_cores: Number of CPU cores
 *
 * Return: CPU load as percentage (0-100)
 *
 * /proc/stat sums all cores, so we must account for cpu_cores.
 * 100% means all cores are fully busy.
 */
double calculate_cpu_load(uint64_t total_cpu_delta, long interval_ms,
			  int cpu_cores)
{
	if (interval_ms <= 0 || cpu_cores <= 0) {
		return 0.0;
	}

	/*
	 * Convert interval to jiffies (assuming 100 Hz)
	 * and multiply by cpu_cores because /proc/stat sums all cores
	 */
	double interval_jiffies = (interval_ms / 1000.0) * 100.0 * cpu_cores;

	if (interval_jiffies <= 0) {
		return 0.0;
	}

	double load = ((double)total_cpu_delta / interval_jiffies) * 100.0;

	if (load > 100.0) {
		load = 100.0;
	}
	if (load < 0.0) {
		load = 0.0;
	}

	return load;
}

