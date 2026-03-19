#include "comm_filter.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
// 系统后台程序黑名单
static const char *BLACKLIST[] = {
    "systemd",
    "dbus-daemon",
    "cron",
    "agetty",
    "sshd",
    "bash",         // 如果你想监控 bash 可删掉
    "zsh",
    "tracker-extract",
    NULL
};
// comm 黑名单过滤
bool is_blacklisted_comm(const char *comm)
{
    for (int i = 0; BLACKLIST[i]; i++) {
        if (strcmp(comm, BLACKLIST[i]) == 0)
            return true;
    }
    return false;
}
// 短命进程过滤（运行时间 < 5 秒）
bool is_short_lived_process(int pid)
{
    char path[64], buf[256];
    long long start_ticks = 0;
    FILE *fp;
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (!fp)
        return true; // 进程不存在，忽略
    // 字段 22 是 starttime（jiffies）
    for (int i = 0; i < 22; i++)
        fscanf(fp, "%s", buf);
    fscanf(fp, "%lld", &start_ticks);
    fclose(fp);
    long ticks_per_sec = sysconf(_SC_CLK_TCK);
    long long now_ticks = (long long)time(NULL) * ticks_per_sec;
    double seconds = (double)(now_ticks - start_ticks) / ticks_per_sec;
    return seconds < 5;  // 小于 5 秒 → 忽略
}
