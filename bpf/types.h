#ifndef __SYSMON_BPF_TYPES_H__
#define __SYSMON_BPF_TYPES_H__
#include <linux/limits.h>
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
#define FNAME_LEN 256
struct event {
    __u32 pid;
    __u32 tgid;
    __u32 uid;
    char comm[TASK_COMM_LEN];
    char filename[FNAME_LEN];
    char op_type[16];  // 操作类型：EXEC/OPEN/UNLINK
};
#endif
