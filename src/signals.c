#include <signal.h>
#include <stdbool.h>
extern volatile bool exiting;
// 普通函数作为 signal handler
static void sig_handler(int sig)
{
    exiting = true;
}
// 调用这个函数来注册信号
void setup_signals(void)
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
}
