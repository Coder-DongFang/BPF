#ifndef __SYSMON_BPF_HELPERS_H__
#define __SYSMON_BPF_HELPERS_H__
#include <bpf/bpf_core_read.h>
/* 读取文件名 */
static __inline int read_filename(const char *src, char *dst, int size)
{
    return bpf_core_read_str(dst, size, src);
}
#endif
