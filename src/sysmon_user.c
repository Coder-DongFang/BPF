#include "types.h"
#include "log.h"
#include "event_handler.h"
#include "signals.h"
#include <sys/resource.h>
#include "sysmon_bpf.skel.h"
#include <stdbool.h>
#include "memory_leak.h"
volatile bool exiting = false;
static int tick = 0;
int main(int argc, char **argv)
{
    struct sysmon_bpf *skel;
    struct ring_buffer *rb = NULL;
    struct rlimit rl = {RLIM_INFINITY, RLIM_INFINITY};
    
    //初始化内存泄漏监控模块
    memleak_init();
    if (setrlimit(RLIMIT_MEMLOCK, &rl)) { perror("setrlimit"); return 1; }
    setup_signals();
    
    log_init("sysmon.log");
    skel = sysmon_bpf__open();
    if (!skel) { fprintf(stderr, "Failed to open BPF skeleton\n"); return 1; }
    if (sysmon_bpf__load(skel)) {
        fprintf(stderr, "Failed to load BPF skeleton\n");
        sysmon_bpf__destroy(skel);
        return 1;
    }
    if (sysmon_bpf__attach(skel)) {
        fprintf(stderr, "Failed to attach BPF programs\n");
        sysmon_bpf__destroy(skel);
        return 1;
    }
    int map_fd = bpf_map__fd(skel->maps.events);
    rb = ring_buffer__new(map_fd, handle_event, NULL, NULL);
    if (!rb) { fprintf(stderr, "Failed to create ring buffer\n"); goto cleanup; }
    log_output(false, "sysmon_core started. Monitoring exec (all) and sensitive file access for non-root users. Ctrl-C to exit\n");
    while (!exiting) {
        int err = ring_buffer__poll(rb, 100);
        if (err < 0 && errno != EINTR) {
            log_output(false, "ring buffer poll error: %d\n", err);
            break;
        }
        
        tick++;
        if (tick >= 10) { // 100ms * 10 = 1s
            memleak_loop_once();
            tick = 0;
        }
    }
cleanup:
    if (rb) ring_buffer__free(rb);
    sysmon_bpf__destroy(skel);
        // 退出前
    log_close();
    return 0;
}

