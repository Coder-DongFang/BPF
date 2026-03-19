#ifndef COMM_FILTER_H
#define COMM_FILTER_H
#include <stdbool.h>
bool is_blacklisted_comm(const char *comm);
bool is_short_lived_process(int pid);
#endif
