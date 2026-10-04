# Android / Linux Kernel Package Killer (`pkg_killer`)

这是一个基于 Linux / Android 内核的高性能进程拦截与终止模块。采用模块化工程结构设计，通过 **Kprobe** 挂钩 `prctl` 系统调用，在应用完成 Zygote 衍生与特化（设置包名/进程名）的关键节点进行实时匹配并终止目标进程。

---

## 目录结构

```text
kernel_pkg_killer/
├── Makefile                # 内核编译工程 Makefile（支持 Native 与 ARM64 交叉编译）
├── .gitignore              # Git 忽略配置（过滤内核编译中间产物）
├── README.md               # 项目技术文档与使用手册
├── include/
│   ├── killer_config.h     # 核心宏定义、默认目标包名、UID 安全门禁与日志配置
│   ├── killer_hook.h       # Kprobe 钩子声明
│   └── killer_proc.h       # 进程检测、包名安全读取与 SIGKILL 信号下发接口
└── src/
    ├── main.c              # 模块入口 (module_init/exit) 与动态模块参数 (module_param)
    ├── killer_hook.c       # Kprobe 注册与跨架构系统调用参数解析
    └── killer_proc.c       # 用户空间内存安全拷贝、UID 过滤与进程终止逻辑
```

---

## 核心机制与原理

1. **Android 进程生命周期拦截点**：
   Android 应用通过 Zygote 进行 `fork`，克隆出的子进程随后调用 `prctl(PR_SET_NAME, ...)` 命名自身包名。该模块在此系统调用入口通过 Kprobe 进行捕获，避免了直接监听 `execve` 无法捕获 Android 应用的痛点。
2. **完整包名解析**：
   绕开内核 `TASK_COMM_LEN` (16字节) 的字符截断问题，直接从用户空间内存指针中安全读取完整包名（支持长包名匹配及子进程匹配）。
3. **UID 安全门禁机制**：
   内置 `min_uid` 过滤（Android 应用默认 `AID_APP` 从 `10000` 开始）。低 UID 的核心系统进程（如 `system_server`, `surfaceflinger`, `init` 等）会被自动跳过，防止因误伤导致系统崩溃陷入 Bootloop。
4. **动态模块参数**：
   无需每次修改包名重新编译源码，可在加载模块时动态指定参数，或进入 Dry-run 演练模式。

---

## 编译方法

### 1. 本地 Linux 测试机编译
在具备内核头文件的 Linux 主机上直接编译：
```bash
make
```

### 2. Android ARM64 交叉编译
针对 Android 目标设备的内核源码树进行交叉编译：
```bash
make android \
    KDIR=/path/to/android/kernel/out \
    CROSS_COMPILE=aarch64-linux-android-
```

编译完成后将生成 `pkg_killer.ko`。

---

## 模块加载与参数说明

### 支持的动态参数

| 参数名 | 类型 | 默认值 | 作用描述 |
| :--- | :--- | :--- | :--- |
| `target_pkg` | string | `"xxxx"` | 目标包名或特征子串（例如 `"com.example.app"`） |
| `enable_kill`| bool | `true` | 是否真正下发 `SIGKILL`（设为 `false` 可在 `dmesg` 中演练观察） |
| `min_uid` | uint | `10000` | 最低 UID 过滤门禁（保护 UID < 10000 的系统服务） |
| `hook_symbol`| string | 自适应 | 钩子符号（ARM64 默认 `__arm64_sys_prctl`） |

### 加载命令示例

```bash
# 1. 默认参数加载（拦截 "xxxx"）
insmod pkg_killer.ko

# 2. 动态指定拦截特定包名，并启用干跑（仅打印日志不强杀）测试
insmod pkg_killer.ko target_pkg="xxxx" enable_kill=0

# 3. 正式启用并指定包名
insmod pkg_killer.ko target_pkg="xxxx" enable_kill=1

# 4. 查看内核实时拦截日志
dmesg -w | grep pkg_killer

# 5. 卸载模块
rmmod pkg_killer
```

---

## 如何推送到 GitHub 仓库

如果你想将本项目推送到你自己的 GitHub 账号上：

1. 打开 [GitHub](https://github.com) 并创建一个新的公开或私有仓库，例如命名为 `kernel_pkg_killer`（**注意：不要勾选 Initialize with README / .gitignore**）。
2. 在本地项目根目录下执行以下命令推送：

```bash
cd C:\Users\user\.gemini\antigravity\scratch\kernel_pkg_killer

# 1. 添加你的远程仓库地址（请将 YOUR_USERNAME 替换为你的 GitHub 用户名）
git remote add origin https://github.com/YOUR_USERNAME/kernel_pkg_killer.git

# 2. 设置主分支为 main
git branch -M main

# 3. 推送代码到 GitHub
git push -u origin main
```

---

## 免责声明与安全警告

* 本项目仅供操作系统底层原理研究、内核安全防护及防沉迷/管控技术研究使用。
* 内核态执行拥有系统最高权限，加载未经充分验证的代码可能导致系统崩溃（Kernel Panic / Kernel Oops），请务必在测试机或虚拟机环境调试。
