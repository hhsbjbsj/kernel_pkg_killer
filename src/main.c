#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

#include "../include/killer_config.h"
#include "../include/killer_hook.h"

/* 模块参数定义 */
char target_pkg[MAX_PKG_NAME_LEN] = TARGET_PKG_DEFAULT;
module_param_string(target_pkg, target_pkg, sizeof(target_pkg), 0644);
MODULE_PARM_DESC(target_pkg, "Target application package name / substring to terminate (default: 'xxxx')");

bool enable_kill = true;
module_param(enable_kill, bool, 0644);
MODULE_PARM_DESC(enable_kill, "Whether to actually send SIGKILL (true: kill, false: dry-run/log only)");

unsigned int min_uid = DEFAULT_MIN_UID;
module_param(min_uid, uint, 0644);
MODULE_PARM_DESC(min_uid, "Minimum UID required to kill (protects system services, Android app AID_APP starts at 10000)");

#if defined(CONFIG_ARM64)
char hook_symbol[64] = "__arm64_sys_prctl";
#elif defined(CONFIG_X86_64)
char hook_symbol[64] = "__x64_sys_prctl";
#else
char hook_symbol[64] = "sys_prctl";
#endif
module_param_string(hook_symbol, hook_symbol, sizeof(hook_symbol), 0644);
MODULE_PARM_DESC(hook_symbol, "Target syscall symbol name to hook via kprobe");

static int __init killer_module_init(void)
{
    killer_info("Loading %s module...\n", MODULE_NAME);
    killer_info("Target Package   : '%s'\n", target_pkg);
    killer_info("Termination Mode : %s\n", enable_kill ? "Active (SIGKILL)" : "Dry-run (Log only)");
    killer_info("Minimum UID Filter: %u\n", min_uid);
    killer_info("Syscall Hook Symbol: '%s'\n", hook_symbol);

    return killer_hook_init();
}

static void __exit killer_module_exit(void)
{
    killer_hook_exit();
    killer_info("Module %s unloaded cleanly.\n", MODULE_NAME);
}

module_init(killer_module_init);
module_exit(killer_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KernelDev");
MODULE_DESCRIPTION("Android/Linux Modular Package Killer Kernel Module");
MODULE_VERSION("1.0.0");
