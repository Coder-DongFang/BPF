#pragma once
#include <stdbool.h>
void memleak_init(void);
void memleak_watch_pid(int pid, const char *comm);
void memleak_loop_once(void);
