#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "logger.h"

int LEVEL = LOG_LEVEL_INFO;

static FILE *log_file = NULL;
static bool log_file_initialized = false;

static void ensure_logs_folder(void)
{
	struct stat st;
	if (stat("logs", &st) == 0 && S_ISDIR(st.st_mode)) {
		return;
	}

	if (mkdir("logs", 0755) != 0 && errno != EEXIST) {
		fprintf(stderr, "logger: failed to create logs/ directory (%s)\n",
			strerror(errno));
	}
}

static void ensure_log_file(void)
{
	if (log_file_initialized) {
		return;
	}

	log_file_initialized = true;
	ensure_logs_folder();
	log_file = fopen("logs/log1.log", "a");
	if (!log_file) {
		fprintf(stderr, "logger: cannot open logs/log1.log (%s)\n",
			strerror(errno));
	}
}


void log_message(const char *severity,
		 const char *color __attribute__((unused)),
		 const char *message)
{
	ensure_log_file();

	struct timeval now_tv;
	gettimeofday(&now_tv, NULL);
	struct tm tm;
	localtime_r(&now_tv.tv_sec, &tm);
	char timestamp[64];
	size_t ts_len = strftime(timestamp, sizeof(timestamp),
				 "%a %b %d %H:%M:%S", &tm);
	if (ts_len == 0) {
		strcpy(timestamp, "unknown time");
	} else {
		int ms = now_tv.tv_usec / 1000;
		snprintf(timestamp + ts_len, sizeof(timestamp) - ts_len,
			 ".%03d", ms);
	}

	char line[512];
	int len = snprintf(line, sizeof(line), "[%s] %s: %s\n",
			   severity, timestamp, message);
	if (len > 0 && log_file) {
		fwrite(line, 1, len, log_file);
		fflush(log_file);
	}
}

int check_level(const int level)
{
	return level >= LEVEL;
}

void log_debug(const char *message)
{
	if (check_level(LOG_LEVEL_DEBUG)) {
		log_message("DEBUG", "\033[32m", message);
	}
}

void log_info(const char *message)
{
	if (check_level(LOG_LEVEL_INFO)) {
		log_message("INFO", "", message);
	}
}

void log_warning(const char *message)
{
	if (check_level(LOG_LEVEL_WARNING)) {
		log_message("WARNING", "\033[33m", message);
	}
}

void log_error(const char *message)
{
	if (check_level(LOG_LEVEL_ERROR)) {
		log_message("ERROR", "\033[31m", message);
	}
}

/**
 * log_fatal() - Log a fatal error message to both file and console
 * @message: The message to log
 *
 * Writes the message to both the log file and console (stdout) with red color.
 * This function should be used for critical errors that require immediate attention.
 * Always outputs regardless of log level setting to ensure fatal errors are visible.
 */
void log_fatal(const char *message)
{
	if (check_level(LOG_LEVEL_FATAL)) {
		log_message("FATAL", "\033[31m", message);
		printf("\033[31m[FATAL] : %s\033[0m\n", message);
		fflush(stdout);
	}
}

void set_log_level(int level)
{
	if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL) {
		fprintf(stderr, "logger: invalid log level: %d\n", level);
		return;
	}
	
	LEVEL = level;
	
}

