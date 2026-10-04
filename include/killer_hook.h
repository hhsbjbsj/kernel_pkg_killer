#ifndef _KILLER_HOOK_H_
#define _KILLER_HOOK_H_

/**
 * 初始化并注册钩子 (基于 kprobe)
 * @return: 0 成功，负数表示失败
 */
int killer_hook_init(void);

/**
 * 注销并卸载钩子
 */
void killer_hook_exit(void);

#endif /* _KILLER_HOOK_H_ */
