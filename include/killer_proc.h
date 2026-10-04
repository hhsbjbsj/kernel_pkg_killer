#ifndef _KILLER_PROC_H_
#define _KILLER_PROC_H_

#include <linux/sched.h>
#include <linux/types.h>

/**
 * 安全地从用户空间地址提取 prctl 设置的进程名
 * @user_ptr: 用户态字符串指针
 * @out_buf: 接收内核缓冲
 * @max_len: 缓冲最大长度
 * @return: 成功读取的字节数，失败返回负数错误码
 */
long killer_proc_extract_name(const char __user *user_ptr, char *out_buf, size_t max_len);

/**
 * 校验目标进程并执行终止操作
 * @task: 目标 task_struct 指针
 * @detected_name: 提取到的进程/包名
 * @return: 1 表示匹配并触发终止，0 表示未匹配或跳过，负数表示错误
 */
int killer_proc_check_and_kill(struct task_struct *task, const char *detected_name);

#endif /* _KILLER_PROC_H_ */
