#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "display.h"
#include "process.h"

/**
 * format_memory() - Format memory value with human-readable units
 * @bytes: Memory value in bytes
 * @buf: Output buffer
 * @bufsize: Size of output buffer
 *
 * Formats memory like htop: automatically selects K/M/G/T suffix.
 */
static void format_memory(uint64_t bytes, char *buf, size_t bufsize)
{
	const char *units[] = {"K", "M", "G", "T", "P"};
	double value = bytes / 1024.0; // Start from KB
	int unit_idx = 0;

	// Find appropriate unit
	while (value >= 1024.0 && unit_idx < 4) {
		value /= 1024.0;
		unit_idx++;
	}

	// Format with appropriate precision
	if (value < 10.0) {
		snprintf(buf, bufsize, "%.2f%s", value, units[unit_idx]);
	} else if (value < 100.0) {
		snprintf(buf, bufsize, "%.1f%s", value, units[unit_idx]);
	} else {
		snprintf(buf, bufsize, "%.0f%s", value, units[unit_idx]);
	}
}

/**
 * display_init() - Initialize ncurses display
 *
 * Sets up ncurses for terminal UI.
 */
void display_init(void)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	timeout(100); // 100ms timeout for getch()
	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}

/**
 * display_cleanup() - Clean up ncurses
 *
 * Restores terminal to normal mode.
 */
void display_cleanup(void)
{
	endwin();
}

/**
 * display_header() - Display system header information
 * @days: Uptime days
 * @hours: Uptime hours
 * @minutes: Uptime minutes
 * @cpu_load: Overall CPU load percentage
 * @used_mem_mb: Used memory in MB
 * @total_mem_mb: Total memory in MB
 * @process_count: Number of running processes
 * @sort_cpu: Sorting by CPU flag
 * @sort_mem: Sorting by memory flag
 * @reversed: Reverse sort flag
 *
 * Displays system information at the top of the screen.
 */
void display_header(int days, int hours, int minutes, double cpu_load,
		    uint64_t used_mem_mb, uint64_t total_mem_mb,
		    int process_count, bool sort_cpu, bool sort_mem,
		    bool reversed)
{
	attron(COLOR_PAIR(1) | A_BOLD);
	mvprintw(0, 0, "Process Monitor");
	attroff(COLOR_PAIR(1) | A_BOLD);

	attron(COLOR_PAIR(2));
	mvprintw(1, 0, "Uptime: %d days, %d hours, %d mins", days, hours,
		 minutes);
	mvprintw(2, 0, "CPU Load: %.1f/100.0", cpu_load);
	mvprintw(3, 0, "Processes: %d", process_count);
	mvprintw(4, 0, "Memory: %.1f/%.1f GB",
		 used_mem_mb / 1024.0, total_mem_mb / 1024.0);

	// Display sort mode
	attron(COLOR_PAIR(3) | A_BOLD);
	mvprintw(5, 0, "Sort: ");
	attroff(COLOR_PAIR(3) | A_BOLD);

	if (sort_cpu || sort_mem) {
		attron(A_BOLD | COLOR_PAIR(2));
		if (sort_cpu) {
			printw("CPU");
		} else {
			printw("MEM");
		}
		attroff(A_BOLD | COLOR_PAIR(2));

		if (reversed) {
			printw(" ");
			attron(A_BOLD | COLOR_PAIR(3));
			printw("(reversed)");
			attroff(A_BOLD | COLOR_PAIR(3));
		}
	} else {
		attron(COLOR_PAIR(1));
		printw("OFF");
		attroff(COLOR_PAIR(1));
	}

	attroff(COLOR_PAIR(2));
}

/**
 * display_process_info() - Display process information table
 * @processes: Array of ProcessInfo structures
 * @count: Number of processes in the array
 * @scroll_offset: Number of processes to skip from the beginning
 * @search_term: Optional search filter (empty string for no filter)
 *
 * Displays a formatted table with columns: PID, Name, CPU%, MEM(KB), MEM%.
 * Automatically adjusts number of displayed processes based on terminal height.
 * Name column is 40 characters wide and truncates long process names.
 * Supports scrolling and filtering by process name.
 */
void display_process_info(ProcessInfo *processes, int count, int scroll_offset,
			  const char *search_term)
{
	int header_line = 7;

	attron(COLOR_PAIR(3) | A_BOLD);
	mvprintw(header_line, 0, "%-8s %-15s %-10s %-10s %-10s %-s",
		 "PID", "NAME", "CPU%", "MEM", "MEM%", "COMMAND");
	attroff(COLOR_PAIR(3) | A_BOLD);

	mvprintw(header_line + 1, 0,
		 "%-8s %-15s %-10s %-10s %-10s %-s",
		 "--------", "---------------",
		 "----------", "----------", "----------",
		 "-------------------------------------------------------");

	// Calculate maximum displayable processes based on terminal height
	// LINES - header(7) - table_header(2) - status(1) = LINES - 10
	int max_display = LINES - 11;
	if (max_display < 1) {
		max_display = 1;
	}

	bool has_filter = search_term && search_term[0] != '\0';
	int displayed = 0;
	int skipped = 0;

	for (int i = 0; i < count && displayed < max_display; i++) {
		// Apply search filter
		if (has_filter && strstr(processes[i].name, search_term) == NULL) {
			continue;
		}

		// Apply scroll offset
		if (skipped < scroll_offset) {
			skipped++;
			continue;
		}

		int line = header_line + 2 + displayed;

		// Truncate name to 15 chars
		char name_truncated[16];
		strncpy(name_truncated, processes[i].name, 15);
		name_truncated[15] = '\0';

		mvprintw(line, 0, "%-8d %-15s ",
			 processes[i].pid, name_truncated);

		if (processes[i].cpu_valid) {
			printw("%-10.2f ", processes[i].cpu_percent);
		} else {
			printw("%-10s ", "-");
		}

		if (processes[i].mem_valid) {
			char mem_str[16];
			format_memory(processes[i].mem_bytes, mem_str,
				      sizeof(mem_str));
			printw("%-10s %-10.2f ",
			       mem_str, processes[i].mem_percent);
		} else {
			printw("%-10s %-10s ", "-", "-");
		}

		// Print command line
		if (processes[i].cmd_valid) {
			printw("%s", processes[i].cmdline);
		} else {
			printw("-");
		}

		displayed++;
	}

	// Clear remaining lines
	for (int i = displayed; i < max_display; i++) {
		int line = header_line + 2 + i;
		move(line, 0);
		clrtoeol();
	}

	// Display status bar
	if (search_term && search_term[0] != '\0') {
		attron(COLOR_PAIR(3) | A_BOLD);
		mvprintw(LINES - 1, 0,
			 "Filter: '%s' | ESC:Clear Offset:%d",
			 search_term, scroll_offset);
		attroff(COLOR_PAIR(3) | A_BOLD);
	} else {
		mvprintw(LINES - 1, 0,
			 "q:Quit c:CPU m:MEM r:Rev f:Search k:Kill ESC:Clear Offset:%d",
			 scroll_offset);
	}
	clrtoeol();
}

/**
 * display_refresh() - Refresh the display
 *
 * Updates the screen with current content.
 */
void display_refresh(void)
{
	refresh();
}
