#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/prctl.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/ptrace.h>

#include "../include/killer_config.h"
#include "../include/killer_proc.h"
#include "../include/killer_hook.h"

/* 外部可配置的符号名称（可通过模块参数覆盖） */
extern char hook_symbol[64];

static struct kprobe kp;

/**
 * 跨架构提取 prctl(option, arg2, ...) 参数
 */
static void extract_prctl_args(struct pt_regs *regs, int *option, unsigned long *arg2)
{
    *option = 0;
    *arg2 = 0;

#if defined(CONFIG_ARM64)
    /*
     * ARM64 平台内核中，系统调用包装函数 __arm64_sys_prctl 的第一个参数 x0
     * 是指向保存用户上下文 pt_regs 的指针。
     */
    struct pt_regs *user_regs = (struct pt_regs *)regs->regs[0];
    if (user_regs && virt_addr_valid(user_regs)) {
        *option = (int)user_regs->regs[0];
        *arg2 = (unsigned long)user_regs->regs[1];
    } else {
        *option = (int)regs->regs[0];
        *arg2 = (unsigned long)regs->regs[1];
    }
#elif defined(CONFIG_X86_64)
    struct pt_regs *user_regs = (struct pt_regs *)regs->di;
    if (user_regs && virt_addr_valid(user_regs)) {
        *option = (int)user_regs->di;
        *arg2 = (unsigned long)user_regs->si;
    } else {
        *option = (int)regs->di;
        *arg2 = (unsigned long)regs->si;
    }
#else
    killer_warn("Unsupported architecture for register parsing\n");
#endif
}

/**
 * Kprobe 前置回调函数
 */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    int option = 0;
    unsigned long arg2 = 0;
    char name_buf[MAX_PKG_NAME_LEN];
    long len;

    extract_prctl_args(regs, &option, &arg2);

    /* 仅关注 PR_SET_NAME 操作 */
    if (option == PR_SET_NAME && arg2 != 0) {
        len = killer_proc_extract_name((const char __user *)arg2, name_buf, sizeof(name_buf));
        if (len > 0) {
            killer_dbg("Intercepted prctl(PR_SET_NAME) -> '%s' (PID=%d)\n",
                       name_buf, current->pid);

            /* 检测并按策略终止目标进程 */
            killer_proc_check_and_kill(current, name_buf);
        }
    }

    return 0;
}

int killer_hook_init(void)
{
    int ret;

    memset(&kp, 0, sizeof(struct kprobe));
    kp.symbol_name = hook_symbol;
    kp.pre_handler = handler_pre;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        killer_err("Failed to register kprobe on symbol '%s', err = %d\n", hook_symbol, ret);
        return ret;
    }

    killer_info("Kprobe successfully registered on '%s'\n", hook_symbol);
    return 0;
}

void killer_hook_exit(void)
{
    unregister_kprobe(&kp);
    killer_info("Kprobe unregistered from '%s'\n", hook_symbol);
}
