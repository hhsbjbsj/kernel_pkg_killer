#ifndef _KILLER_CONFIG_H_
#define _KILLER_CONFIG_H_

#include <linux/types.h>

#define MODULE_NAME "pkg_killer"

/* 默认拦截的目标包名（占位符） */
#define TARGET_PKG_DEFAULT "xxxx"

/* 最大包名/进程名长度 */
#define MAX_PKG_NAME_LEN 256

/* Android AID_APP 起始 UID（普通 App 普遍 >= 10000），防止误杀 system_server(1000) 等核心进程 */
#define DEFAULT_MIN_UID 10000

/* 日志宏 */
#define killer_info(fmt, ...) pr_info("[" MODULE_NAME "] " fmt, ##__VA_ARGS__)
#define killer_warn(fmt, ...) pr_warn("[" MODULE_NAME "] " fmt, ##__VA_ARGS__)
#define killer_err(fmt, ...)  pr_err("[" MODULE_NAME "] " fmt, ##__VA_ARGS__)

#ifdef DEBUG
#define killer_dbg(fmt, ...)  pr_debug("[" MODULE_NAME "] " fmt, ##__VA_ARGS__)
#else
#define killer_dbg(fmt, ...)  do {} while (0)
#endif

#endif /* _KILLER_CONFIG_H_ */
