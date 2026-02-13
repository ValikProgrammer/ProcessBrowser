#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "input.h"
#include "logger.h"

/**
 * input_init() - Initialize input state
 * @state: Input state structure to initialize
 *
 * Sets default values for input handling.
 */
void input_init(InputState *state)
{
	state->sort_cpu = true;
	state->sort_mem = false;
	state->reversed = false;
	state->scroll_offset = 0;
	state->should_exit = false;
	memset(state->search_term, 0, sizeof(state->search_term));
}

/**
 * handle_search_interactive() - Interactive search mode
 * @state: Input state structure
 *
 * Enters interactive search mode where each keystroke filters the table.
 * ESC exits search mode.
 */
static void handle_search_interactive(InputState *state)
{
	bool in_search = true;
	int cursor_pos = strlen(state->search_term);

	echo();
	curs_set(1);
	timeout(50); // Faster response in search mode

	while (in_search) {
		mvprintw(LINES - 1, 0, "Search: %s", state->search_term);
		clrtoeol();
		move(LINES - 1, 8 + cursor_pos);
		refresh();

		int ch = getch();

		if (ch == ERR) {
			continue;
		}

		if (ch == 27) { // ESC - clear search and exit
			state->search_term[0] = '\0';
			cursor_pos = 0;
			in_search = false;
		} else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
			if (cursor_pos > 0) {
				cursor_pos--;
				state->search_term[cursor_pos] = '\0';
			}
		} else if (ch == '\n' || ch == KEY_ENTER) {
			in_search = false;
		} else if (ch >= 32 && ch < 127 && cursor_pos < 255) {
			state->search_term[cursor_pos] = ch;
			cursor_pos++;
			state->search_term[cursor_pos] = '\0';
		}
	}

	noecho();
	curs_set(0);
	timeout(100); // Restore normal timeout

	char log_msg[300];
	snprintf(log_msg, sizeof(log_msg), "Search term: '%s'",
		 state->search_term);
	log_info(log_msg);
}

/**
 * handle_kill() - Handle kill functionality
 * @processes: Array of processes
 * @count: Number of processes
 * @offset: Current scroll offset
 *
 * Prompts for PID and sends SIGTERM signal to the process.
 */
static void handle_kill(ProcessInfo *processes __attribute__((unused)),
			int count __attribute__((unused)),
			int offset __attribute__((unused)))
{
	echo();
	curs_set(1);
	timeout(-1); // Blocking mode for input

	mvprintw(LINES - 1, 0, "Enter PID to kill (ESC to cancel): ");
	clrtoeol();
	refresh();

	char buf[32] = {0};
	int buf_pos = 0;
	bool cancelled = false;

	while (1) {
		int ch = getch();

		if (ch == 27) { // ESC
			cancelled = true;
			break;
		} else if (ch == '\n' || ch == KEY_ENTER) {
			break;
		} else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
			if (buf_pos > 0) {
				buf_pos--;
				buf[buf_pos] = '\0';
				mvprintw(LINES - 1, 0,
					 "Enter PID to kill (ESC to cancel): %s",
					 buf);
				clrtoeol();
			}
		} else if (ch >= '0' && ch <= '9' && buf_pos < 31) {
			buf[buf_pos++] = ch;
			buf[buf_pos] = '\0';
			mvprintw(LINES - 1, 0,
				 "Enter PID to kill (ESC to cancel): %s", buf);
			clrtoeol();
		}
		refresh();
	}

	noecho();
	curs_set(0);
	timeout(100); // Restore normal timeout

	if (cancelled || buf_pos == 0) {
		return;
	}

	int pid = atoi(buf);
	if (pid <= 0) {
		return;
	}

	// Always send SIGTERM
	if (kill(pid, SIGTERM) == 0) {
		char log_msg[256];
		snprintf(log_msg, sizeof(log_msg),
			 "Sent SIGTERM to PID %d", pid);
		log_info(log_msg);
	} else {
		char log_msg[256];
		snprintf(log_msg, sizeof(log_msg),
			 "Failed to kill PID %d", pid);
		log_error(log_msg);
	}
}

/**
 * input_handle() - Handle user input
 * @state: Input state structure
 * @processes: Array of processes
 * @count: Number of processes
 *
 * Processes keyboard input and updates state accordingly.
 */
void input_handle(InputState *state, ProcessInfo *processes, int count)
{
	int ch = getch();

	if (ch == ERR) {
		return;
	}

	switch (ch) {
	case 'c':
	case 'C':
		if (state->sort_cpu) {
			// Toggle off if already sorting by CPU
			state->sort_cpu = false;
			log_info("CPU sorting disabled");
		} else {
			state->sort_cpu = true;
			state->sort_mem = false;
			log_info("Sorting by CPU");
		}
		break;

	case 'm':
	case 'M':
		if (state->sort_mem) {
			// Toggle off if already sorting by MEM
			state->sort_mem = false;
			log_info("Memory sorting disabled");
		} else {
			state->sort_cpu = false;
			state->sort_mem = true;
			log_info("Sorting by Memory");
		}
		break;

	case 'r':
	case 'R':
		state->reversed = !state->reversed;
		log_info(state->reversed ? "Sort reversed" : "Sort normal");
		break;

	case KEY_UP:
		if (state->scroll_offset > 0) {
			state->scroll_offset--;
		}
		break;

	case KEY_DOWN:
		state->scroll_offset++;
		break;

	case KEY_PPAGE: // Page Up
		state->scroll_offset -= 10;
		if (state->scroll_offset < 0) {
			state->scroll_offset = 0;
		}
		break;

	case KEY_NPAGE: // Page Down
		state->scroll_offset += 10;
		break;

	case KEY_F(3):
	case 'f': // Alternative for F3
		handle_search_interactive(state);
		break;

	case KEY_F(9):
	case 'k': // Alternative for F9
	case 'K':
		handle_kill(processes, count, state->scroll_offset);
		break;

	case 'q':
	case 'Q':
		state->should_exit = true;
		log_info("Exit requested");
		break;

	case 27: // ESC - clear search filter
		if (state->search_term[0] != '\0') {
			state->search_term[0] = '\0';
			log_info("Search filter cleared");
		}
		break;

	case '/': // Alternative search key
		handle_search_interactive(state);
		break;
	}

	// Limit scroll offset
	if (state->scroll_offset < 0) {
		state->scroll_offset = 0;
	}
	if (state->scroll_offset >= count) {
		state->scroll_offset = count - 1;
		if (state->scroll_offset < 0) {
			state->scroll_offset = 0;
		}
	}
}

