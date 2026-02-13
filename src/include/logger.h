#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_DEBUG   10
#define LOG_LEVEL_INFO    20
#define LOG_LEVEL_WARNING 30
#define LOG_LEVEL_ERROR   40
#define LOG_LEVEL_FATAL   50


void log_debug(const char *message);
void log_info(const char *message);
void log_warning(const char *message);
void log_error(const char *message);
void log_fatal(const char *message);

void set_log_level(int level);

#endif
 