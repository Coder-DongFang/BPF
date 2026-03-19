#ifndef __SYSMON_USER_TYPES_H__
#define __SYSMON_USER_TYPES_H__
#include <stdint.h>
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
#ifndef FNAME_LEN
#define FNAME_LEN 256
#endif
struct event {
    uint32_t pid;
    uint32_t tgid;
    uint32_t uid;
    char comm[TASK_COMM_LEN];
    char filename[FNAME_LEN];
    char op_type[16];
} __attribute__((packed));
#endif
