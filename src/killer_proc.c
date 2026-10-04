#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/sched/signal.h>

#include "../include/killer_config.h"
#include "../include/killer_proc.h"

/* 引用在 main.c 中定义的模块动态参数 */
extern char target_pkg[MAX_PKG_NAME_LEN];
extern bool enable_kill;
extern unsigned int min_uid;

long killer_proc_extract_name(const char __user *user_ptr, char *out_buf, size_t max_len)
{
    long len;

    if (!user_ptr || !out_buf || max_len == 0)
        return -EINVAL;

    len = strncpy_from_user(out_buf, user_ptr, max_len - 1);
    if (len < 0) {
        killer_dbg("Failed to read user string: error %ld\n", len);
        return len;
    }

    out_buf[len] = '\0';
    return len;
}

int killer_proc_check_and_kill(struct task_struct *task, const char *detected_name)
{
    uid_t uid;

    if (!task || !detected_name)
        return -EINVAL;

    /* 获取目标进程 UID */
    uid = from_kuid(&init_user_ns, task_uid(task));

    /*
     * 安全门禁：过滤低 UID（系统关键服务如 init、system_server 等），
     * 避免误伤导致 Android 系统崩溃或重启。
     */
    if (uid < min_uid) {
        killer_dbg("Skipping system process PID=%d UID=%u Name=%s\n",
                   task->pid, uid, detected_name);
        return 0;
    }

    /* 
     * 匹配目标包名：
     * 支持包含匹配（strstr），可同时拦截形如 "xxxx" 以及子进程 "xxxx:remote"、"xxxx:push"
     */
    if (strlen(target_pkg) > 0 && strstr(detected_name, target_pkg) != NULL) {
        killer_info("Target application detected! PID=%d, UID=%u, Name='%s', Target='%s'\n",
                    task->pid, uid, detected_name, target_pkg);

        if (enable_kill) {
            killer_info("Enforcing termination: Sending SIGKILL to PID=%d\n", task->pid);
            
            /*
             * 向进程发送 SIGKILL (9号信号)
             * 如果在当前上下文则直接 force_sig(SIGKILL)
             */
            if (task == current) {
                force_sig(SIGKILL);
            } else {
                send_sig_info(SIGKILL, SEND_SIG_PRIV, task);
            }
            return 1;
        } else {
            killer_info("Dry-run mode: Process PID=%d matched but kill is disabled.\n", task->pid);
            return 0;
        }
    }

    return 0;
}
