// log.h
#ifndef LOG_H
#define LOG_H
#include <stdbool.h>
void log_init(const char *filename);
void log_output(bool highlight, const char *fmt, ...);
void log_close(void);  // 关闭文件
void log_plain(const char *fmt, ...);
#endif
