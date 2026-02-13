#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "process.h"

typedef struct {
	bool sort_cpu;
	bool sort_mem;
	bool reversed;
	int scroll_offset;
	bool should_exit;
	char search_term[256];
} InputState;

void input_init(InputState *state);
void input_handle(InputState *state, ProcessInfo *processes, int count);

#endif

