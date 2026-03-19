#include "memory_leak.h"
#include "log.h"
#include "comm_filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_PIDS 1024
#define EMA_ALPHA 0.3
#define LEAK_THRESHOLD_PERCENT 0.10  // 10%
#define MIN_DATA_POINTS 5
typedef struct {
    int pid;
    double ema;
    double last_rss;
    int count;
    int active;
} proc_stat_t;
static proc_stat_t stats[MAX_PIDS];
static double read_rss_kb(int pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/smaps", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    char line[256];
    double rss = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Rss:", 4) == 0) {
            int val = 0;
            if (sscanf(line, "Rss: %d kB", &val) == 1) {
                rss += val;
            }
        }
    }
    fclose(fp);
    return rss; // KB
}
void memleak_init(void)
{
    memset(stats, 0, sizeof(stats));
}
void memleak_watch_pid(int pid, const char *comm)
{
    // 黑名单过滤（蓝色）
    if (is_blacklisted_comm(comm)) {
        log_plain("\033[34m[memleak] ignore blacklisted process %s (%d)\033[0m\n", comm, pid);
        return;
    }
    // 短命进程过滤（蓝色）
    if (is_short_lived_process(pid)) {
        log_plain("\033[34m[memleak] ignore short-lived process %s (%d)\033[0m\n", comm, pid);
        return;
    }
    // 如果 pid 已在监控中则直接返回
    for (int i = 0; i < MAX_PIDS; i++) {
        if (stats[i].active && stats[i].pid == pid) return;
    }
    for (int i = 0; i < MAX_PIDS; i++) {
        if (!stats[i].active) {
            stats[i].active = 1;
            stats[i].pid = pid;
            stats[i].ema = 0;
            stats[i].last_rss = 0;
            stats[i].count = 0;
            log_output(false, "memleak: start watching PID=%d\n", pid);
            return;
        }
    }
    // 若监控池已满
    log_output(false, "memleak: watch list full, cannot watch PID=%d\n", pid);
}
void memleak_loop_once(void)
{
    for (int i = 0; i < MAX_PIDS; i++) {
        if (!stats[i].active) continue;
        int pid = stats[i].pid;
        double rss = read_rss_kb(pid);
        if (rss < 0) {
            // 进程可能已退出（蓝色）
            log_plain("\033[34m[memleak] PID=%d exited or smaps unreadable, stop watching\033[0m\n", pid);
            stats[i].active = 0;
            continue;
        }
        if (stats[i].count == 0) {
            stats[i].ema = rss;
        } else {
            stats[i].ema = EMA_ALPHA * rss + (1 - EMA_ALPHA) * stats[i].ema;
        }
        if (stats[i].count >= MIN_DATA_POINTS) {
            if (stats[i].last_rss > 0) {
                double growth = (rss - stats[i].last_rss) / stats[i].last_rss;
                if (growth > LEAK_THRESHOLD_PERCENT) {
                    // 红色由 log_output(true, ...) 控制
                    log_output(true,
                               "[MEM-LEAK] PID=%d RSS increased %.2f%% (%.0f KB -> %.0f KB) count=%d\n",
                               pid, growth * 100.0, stats[i].last_rss, rss, stats[i].count);
                }
            }
        }
        stats[i].last_rss = rss;
        stats[i].count++;
    }
}
