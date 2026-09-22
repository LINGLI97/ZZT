# ZZT 优化构建版本交接记录

更新时间：2026-09-22
工作目录：`/scratch/prj/proj_loukides/scratch_tmp/ling_tmp/kkp/examples`

## 1. 工作目标与当前状态

本次工作的目标是在旧 `ZZT` 工程中加入 `zigzag-libsais` 的优化 ZZA/ZZLCP
构建器，同时满足以下要求：

- 不删除原来的 merge-sort 构建方式；
- 不占用原来的可执行文件名；
- 为全部六种查询方法提供独立的 32 位和 64 位版本；
- 保留原来的 Trie、ESA、R-tree 和查询逻辑；
- 输出 construction time、index size、index memory 和 query time；
- 新旧版本在相同输入上得到一致的查询结果。

当前状态：功能已经接入，12 个优化目标均已成功编译，小数据一致性测试和后端
随机暴力测试均已通过。

## 2. 总体结构

旧流程：

```text
text
  -> SA/LCP/RMQ(text 和 reverse(text))
  -> mergeSortIterativeZigZag
  -> ZZA + ZZLCP
  -> compact trie / truncated ESA / R-tree
  -> CC、LFCS、LCCS、TCPR 查询
```

优化流程：

```text
text
  -> libsais/radix ZigZag backend
  -> ZZA + ZZLCP
  -> 原来的 compact trie / truncated ESA / R-tree
  -> 原来的 CC、LFCS、LCCS、TCPR 查询
```

原查询结构没有被替换。优化版本只替换 ZZA/ZZLCP 的构建阶段。

## 3. 代码位置

### 新增文件

- `src/zigzag_libsais_backend.h`
  - 旧 ZZT 与新构建器之间的稳定接口；
  - 显式支持 `int32_t` 和 `int64_t`；
  - 返回 radix rounds、fallback mode 等统计信息。
- `src/zigzag_libsais_backend.cpp`
  - 对输入字节做保序 alphabet remapping；
  - 调用 `zigzag-libsais/zigzag.cpp` 的构建核心；
  - 启用与旧 ZZT 相同的高位边界符语义；
  - 32 位输入超过 `INT32_MAX` 时抛出错误。
- `src/rmq-offline-local.cpp`
  - LCCS 使用的本地线性空间 offline RMQ；
  - 采用 monotone stack + union/find path compression；
  - 用于替代旧工程中已失效的个人目录 `librmqo` 软链接；
  - 只加入优化 LCCS 目标，原目标的构建定义没有被强行替换。
- `src/zigzag_libsais_backend_test.cpp`
  - 显式生成 ZigZag 字符串作为参考结果；
  - 随机测试优化 32/64 位 ZZA 和 ZZLCP；
  - 同时检查旧 ZZT 的高位边界符排序规则。

### 修改文件

- `src/CC.cpp`
- `src/LFCS.cpp`
- `src/LCCS.cpp`
- `src/topK_SA_Truncated.cpp`
- `src/topK_SA_Truncated_TP.cpp`
- `src/topK_SA_Truncated_GAP.cpp`
- `src/Makefile`
- `README.md`
- `vendor/zigzag-libsais/zigzag.cpp`

六个入口程序通过 `USE_LIBSAIS_ZZ` 条件宏选择新后端。未定义该宏时仍执行原来的
`SA_LCP_LCE + mergeSortIterativeZigZag` 路径。

## 4. 新旧边界符语义

集成过程中发现一个重要差异：

- 独立 `zigzag-libsais` 原本采用普通有限字符串顺序，即较短的真前缀排在前面；
- 旧 ZZT 的 `getZigZagChar()` 对越界位置返回 `255`，即越界标记排在普通 ASCII
  字符之后。

如果不处理这个差异，CC/LFCS 示例可能仍给出相同结果，但 LCCS 的代表位置会发生
变化。为此在新构建器的 `Params` 中加入了默认关闭的 `end_symbol`：

```cpp
int end_symbol = -1;
```

- `-1`：保持独立新代码原来的有限字符串行为；
- 非负数：使用指定的虚拟高位边界字符；
- ZZT backend 将其设置为 alphabet 之后的一个保留编码。

因此独立 `zigzag` 的默认行为没有改变，而 ZZT 优化目标与旧 ZZT 的排序定义一致。

## 5. 32 位与 64 位版本

所有方法均提供两个独立目标：

| 方法 | 32 位程序 | 64 位程序 |
|---|---|---|
| CC | `run_CC_opt32` | `run_CC_opt64` |
| LFCS | `run_LFCS_opt32` | `run_LFCS_opt64` |
| LCCS | `run_LCCS_opt32` | `run_LCCS_opt64` |
| TCPR-TF | `run_TopK_SA_Truncated_Freq_opt32` | `run_TopK_SA_Truncated_Freq_opt64` |
| TCPR-TP | `run_TopK_SA_Truncated_TP_opt32` | `run_TopK_SA_Truncated_TP_opt64` |
| TCPR-SP | `run_TopK_SA_Truncated_SP_opt32` | `run_TopK_SA_Truncated_SP_opt64` |

位宽不是只控制输出文件：整个旧 ZZT 上层结构也分别使用 `_USE_32` 和 `_USE_64`
重新编译。

选择规则：

- 完整文本长度及所有位置必须小于等于 `2,147,483,647` 时才可使用 `_opt32`；
- 超过该限制必须使用 `_opt64`；
- 当前 1000MB 和 2000MB CHR 文件若实际字节数低于该限制，优先测试 `_opt32`；
- 32 位 ZZA+ZZLCP 理论大小为 `8n` bytes，64 位为 `16n` bytes。

## 6. 编译方法

进入源码目录并加载 Boost：

```bash
cd /scratch/prj/proj_loukides/scratch_tmp/ling_tmp/kkp/examples/ZZT/src
module load boost/1.83.0-gcc-13.2.0
```

编译所有优化版本：

```bash
make -j2 optimized
```

也可以分开编译：

```bash
make -j2 optimized-core
make -j2 optimized-topk
```

只编译某个版本：

```bash
make run_CC_opt32
make run_CC_opt64
```

原来的目标仍然存在：

```bash
make CC
make LFCS
make LCCS
make TopK_Freq
make TopK_TP
make TopK_SP
```

依赖位置：

- SDSL：`ZZT/third_party/sdsl-install`
- libsais：`ZZT/vendor/zigzag-libsais`（已随仓库保存）
- Boost：CREATE 集群 module `boost/1.83.0-gcc-13.2.0`

## 7. 运行示例

CC：

```bash
./run_CC_opt32 \
  -f ../dataset/input.txt \
  -p ../dataset/patterns.txt
```

LFCS：

```bash
./run_LFCS_opt32 \
  -f ../dataset/input.txt \
  -p ../dataset/patterns.txt \
  -t 2
```

LCCS：

```bash
./run_LCCS_opt32 \
  -f ../dataset/multiInput.txt \
  -p ../dataset/patterns.txt \
  -t 2
```

Top-K TF：

```bash
./run_TopK_SA_Truncated_Freq_opt32 \
  -f ../dataset/input.txt \
  -p ../dataset/patterns.txt
```

TP 和 SP 使用对应的 `_TP_opt32`、`_SP_opt32` 程序，命令行参数与原程序相同。

## 8. 输出指标

优化版本会继续输出旧代码已有的整体指标，并额外标记构建后端和位宽。例如：

```text
ZZA backend: optimized libsais/radix (32-bit)
Backend radix rounds: ...
Comparable ZZA + ZZ-LCP construction time: ... seconds
Comparable ZZA + ZZ-LCP index size: ... MB (... bytes)
ZZT construction time: ... seconds
Total construction time: ... seconds
Index memory: ... MB
Query time: ... s
```

正式实验的 construction space 仍应由外部命令测量峰值 RSS：

```bash
/usr/bin/time -v ./run_CC_opt32 ...
```

主要读取：

```text
Maximum resident set size (kbytes)
```

注意不要把程序内部的 `Index memory` 当作完整 construction peak RSS。

## 9. 已完成测试

### 六种方法的端到端测试

使用 `ZZT/dataset` 中的示例输入完成以下比较：

- CC：32 位与 64 位一致；旧版与优化版一致；
- LFCS：32 位与 64 位一致；旧版与优化版一致；
- LCCS：32 位与 64 位一致；旧版与优化版一致；
- TCPR-TF：32 位与 64 位一致；旧版与优化版一致；
- TCPR-TP：32 位与 64 位一致；旧版与优化版一致；
- TCPR-SP：32 位与 64 位一致；旧版与优化版一致。

共 12 项结果比较，全部 `PASS`。比较时忽略计时和内存数值，只比较查询语义输出。

### 后端随机暴力测试

运行：

```bash
make test-optimized-backend
```

当前结果：

```text
optimized backend randomized 32/64-bit test: PASS
```

测试覆盖长度 1–160、多种随机小字母表，同时核对：

- 完整 ZZA 排序；
- 每个相邻位置的 ZZLCP；
- 32 位与 64 位输出；
- 高位边界符语义。

独立 `zigzag` 的原有测试也通过：

```text
unit tests: 308440 checks, 0 wrong
--check: OK
```

## 10. Top-K 当前实现及限制

三个 Top-K 优化程序当前执行：

```text
构建完整优化 ZZA/ZZLCP
  -> ZZLCP[i] = min(ZZLCP[i], Bound)
  -> 原来的 truncated ESA / R-tree
```

完整字典序保证相同 `Bound` 前缀的元素连续，所以示例查询结果与旧 truncated
构建一致。但是当前实现没有在达到 `Bound` 后提前停止构建，因此：

- 功能和查询语义已经接通；
- 不是原生 bounded 构建；
- 在 `Bound` 很小时可能做了不必要的 radix rounds 或 fallback 工作；
- 下一阶段可给 backend 增加 `max_zigzag_length/Bound`，从构建内部提前停止。

## 11. LCCS 注意事项

LCCS 的旧输入索引是：

```cpp
vector<pair<INT, INT>> indices;  // position, text_id
```

新后端先产生 position 排序，再通过 `IdxSeparators` 恢复每个位置的 `text_id`。
这一步当前需要一个临时 `sorted_positions` 数组，所以 LCCS construction peak memory
可能高于只看最终 index size 得出的估计。数组转换后会立即释放临时空间。

旧 `src/librmqo` 是指向以下个人目录的失效软链接：

```text
/home/ling/OneDrive/PhD/CPM/Zigzag/librmqo
```

优化 LCCS 已改用仓库内的 `rmq-offline-local.cpp`，无需该外部目录。

## 12. 后续建议

优先级建议：

1. 在小型随机输入上把“旧 ZZA/ZZLCP 数组 vs 新数组”的直接比较加入持续测试；
2. 实现 Top-K 原生 `Bound` 提前停止版本；
3. 在 CHR 1000MB、2000MB 上分别测试 `_opt32` 和 `_opt64`；
4. 正式任务使用独占 idle 节点，并用 `/usr/bin/time -v` 保存峰值 RSS；
5. 每个数据集、位宽、partition 使用独立结果目录，避免日志覆盖；
6. 大数据实验同时记录 `OMP_NUM_THREADS`、节点名、commit/diff 和实际文件字节数。

推荐正式记录字段：

```text
dataset
dataset_bytes
method
index_width
OMP_NUM_THREADS
hostname
construction_time_seconds
zza_zzlcp_bytes
maximum_resident_set_kbytes
total_index_memory
query_time_seconds
exit_code
```

## 13. 工作区保护说明

开始本次工作前，`src/CC.cpp`、`src/Makefile`、`construct_only/` 和
`third_party/` 已存在用户改动或未跟踪内容。本次没有执行 `git reset`、`git checkout`
或删除操作。

目前修改尚未提交 Git。接手人提交前应先检查：

```bash
cd /scratch/prj/proj_loukides/scratch_tmp/ling_tmp/kkp/examples/ZZT
git status --short
git diff --check
git diff
```

不要直接清理 `construct_only/`、`third_party/` 或已有结果目录，它们可能属于之前的
实验工作。
