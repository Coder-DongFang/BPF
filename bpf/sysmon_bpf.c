#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include "types.h"
#include "helpers.h"
char LICENSE[] SEC("license") = "GPL";
/* Ring buffer */
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");
/* ------------------- execve ------------------- */
SEC("tracepoint/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
{
    struct event *e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) return 0;
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e->tgid = pid_tgid >> 32;
    e->pid  = pid_tgid & 0xffffffff;
    e->uid  = bpf_get_current_uid_gid() & 0xffffffff;
    bpf_get_current_comm(e->comm, sizeof(e->comm));
    __builtin_memcpy(e->op_type, "EXEC", 5);
    /* 解析 filename */
    __s32 data_loc = ctx->__data_loc_filename;
    if (data_loc) {
        __u16 offset = data_loc & 0xFFFF;
        __u16 len    = data_loc >> 16;
        if (len > 0 && len < FNAME_LEN) {
            const char *fname = (const char *)ctx + offset;
            read_filename(fname, e->filename, sizeof(e->filename));
        } else {
            e->filename[0] = '\0';
        }
    } else {
        e->filename[0] = '\0';
    }
    bpf_ringbuf_submit(e, 0);
    return 0;
}
/* ------------------- open/openat ------------------- */
SEC("tracepoint/syscalls/sys_enter_openat")
int handle_openat(struct trace_event_raw_sys_enter *ctx)
{
    struct event *e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) return 0;
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e->tgid = pid_tgid >> 32;
    e->pid  = pid_tgid & 0xffffffff;
    e->uid  = bpf_get_current_uid_gid() & 0xffffffff;
    bpf_get_current_comm(e->comm, sizeof(e->comm));
    __builtin_memcpy(e->op_type, "OPEN", 5);
    const char *pathname = (const char *)ctx->args[1];
    bpf_probe_read_user_str(e->filename, sizeof(e->filename), pathname);
    bpf_ringbuf_submit(e, 0);
    return 0;
}
/* ------------------- unlink/unlinkat ------------------- */
SEC("tracepoint/syscalls/sys_enter_unlinkat")
int handle_unlinkat(struct trace_event_raw_sys_enter *ctx)
{
    struct event *e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e) return 0;
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e->tgid = pid_tgid >> 32;
    e->pid  = pid_tgid & 0xffffffff;
    e->uid  = bpf_get_current_uid_gid() & 0xffffffff;
    bpf_get_current_comm(e->comm, sizeof(e->comm));
    __builtin_memcpy(e->op_type, "UNLINK", 7);
    const char *pathname = (const char *)ctx->args[1];
    bpf_probe_read_user_str(e->filename, sizeof(e->filename), pathname);
    bpf_ringbuf_submit(e, 0);
    return 0;
}
