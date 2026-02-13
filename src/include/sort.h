#ifndef SORT_H
#define SORT_H

#include <stdbool.h>
#include "process.h"

void sort_by_cpu(ProcessInfo *processes, int count, bool reversed);
void sort_by_mem(ProcessInfo *processes, int count, bool reversed);

#endif

