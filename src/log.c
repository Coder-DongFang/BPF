// log.c
#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
static FILE *log_file = NULL;
void log_init(const char *filename) {
    log_file = fopen(filename, "a");
    if (!log_file) perror("fopen log file");
}
void log_output(bool highlight, const char *fmt, ...) {
    char timebuf[64];
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_info);
    va_list args;
    va_start(args, fmt);
    printf("[%s] ", timebuf);
    if (highlight) printf("\x1b[31m");
    vprintf(fmt, args);
    if (highlight) printf("\x1b[0m");
    fflush(stdout);
    va_start(args, fmt);
    if (log_file) {
        fprintf(log_file, "[%s] ", timebuf);
        vfprintf(log_file, fmt, args);
        fflush(log_file);
    }
    va_end(args);
    va_end(args);
}
void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
// 简单输出到控制台，不加时间戳、不写文件、不加前缀
void log_plain(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}
