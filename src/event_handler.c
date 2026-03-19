#include <string.h>
#include "types.h"
#include "log.h"
#include "event_handler.h"
#include "memory_leak.h"   // 新增：当 exec 时启动内存监控
int handle_event(void *ctx, void *data, size_t data_sz)
{
    struct event *e = data;
    if (strncmp(e->op_type, "EXEC", 4) == 0) {
        log_output(false, "[EXEC] UID %u TGID %u PID %u COMM %s FILE %s\n",
                   e->uid, e->tgid, e->pid, e->comm, e->filename);
        
        
        /* 启动对该 PID 的内存监控（若已在监控则内部忽略） */
          memleak_watch_pid(e->pid, e->comm);
    }
    else if ((strncmp(e->op_type, "OPEN", 4) == 0 ||
              strncmp(e->op_type, "UNLINK", 6) == 0) &&
             e->uid != 0 && e->filename[0]) {
        if (strstr(e->filename, "/tmp/passwd") ||
            strstr(e->filename, "/tmp/shadow")) {
            log_output(true, "[ALERT] non-root UID %u performed %s on %s (PID %u COMM %s)\n",
                       e->uid, e->op_type, e->filename, e->pid, e->comm);
        }
    }
    return 0;
}
