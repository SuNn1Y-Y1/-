# malloc.c 内存管理模块分析

## 概述

`malloc.c` 是一个为嵌入式系统（特别是STM32）设计的内存管理模块，实现了动态内存分配功能。与标准C库的 `malloc/free` 不同，这个实现针对嵌入式环境进行了优化，具有以下特点：

1. **多内存池管理**：支持管理多个不同的内存区域
2. **固定块大小分配**：使用内存块管理，减少碎片
3. **轻量级**：代码简洁，适合资源受限的嵌入式系统
4. **可配置**：通过宏定义灵活配置各内存池参数

## 内存池配置

模块定义了三个内存池：

### 1. SRAMIN（内部SRAM）
- **位置**：STM32内部SRAM
- **默认大小**：50KB
- **块大小**：32字节
- **访问权限**：所有外设和CPU均可访问
- **用途**：通用内存分配

### 2. SRAMCCM（CCM内存）
- **位置**：STM32F4/F7系列的CCM内存（Core Coupled Memory）
- **默认大小**：60KB
- **块大小**：32字节
- **访问权限**：仅CPU可访问，速度更快
- **用途**：需要高速访问的数据（如DMA缓冲区、实时任务数据）

### 3. SRAMEX（外部SRAM）
- **位置**：外部扩展的SRAM
- **默认大小**：963KB
- **块大小**：32字节
- **访问权限**：所有外设和CPU均可访问
- **用途**：大容量数据存储（如图像、音频缓冲区）

## 核心数据结构

### 内存管理控制器
```c
struct _m_mallco_dev
{
    void (*init)(uint8_t);              // 初始化函数指针
    uint16_t (*perused)(uint8_t);       // 内存使用率查询函数指针
    uint8_t *membase[SRAMBANK];         // 内存池基地址数组
    MT_TYPE *memmap[SRAMBANK];          // 内存管理状态表
    uint8_t  memrdy[SRAMBANK];          // 内存管理就绪标志
};
```

### 内存管理表
- 使用位图（bitmap）方式管理内存块
- 每个内存块对应一个管理表项
- 表项值为0表示空闲，非0表示已分配（值为连续块数）

## 核心算法原理

### 1. 内存分配算法
- **块对齐**：所有分配按32字节块对齐
- **首次适应（从高地址向低地址搜索）**：从内存表末尾向前搜索连续空闲块
- **连续块分配**：需要连续多个块时，一次性分配并标记

### 2. 内存释放算法
- **偏移计算**：根据释放地址计算对应的内存块索引
- **块数获取**：从管理表获取该块分配的连续块数
- **批量释放**：一次性释放所有连续块

### 3. 内存碎片处理
- 固定块大小设计减少外部碎片
- 但可能存在内部碎片（分配不足整块时）

## 主要函数接口

### 基础函数
```c
// 内存管理初始化
void my_mem_init(uint8_t memx);

// 获取内存使用率（0-1000，代表0.0%-100.0%）
uint16_t my_mem_perused(uint8_t memx);

// 内存设置（类似memset）
void my_mem_set(void *s, uint8_t c, uint32_t count);

// 内存拷贝（类似memcpy）
void my_mem_copy(void *des, void *src, uint32_t n);
```

### 内存分配/释放函数（指定内存池）
```c
// 从指定内存池分配内存
void *mymalloc_memx(uint8_t memx, uint32_t size);

// 释放指定内存池的内存
void myfree_memx(uint8_t memx, void *ptr);

// 重新分配指定内存池的内存
void *myrealloc_memx(uint8_t memx, void *ptr, uint32_t size);
```

### 简化函数（默认使用SRAMIN）
```c
// 从默认内存池（SRAMIN）分配内存
void *mymalloc(uint32_t size);

// 释放默认内存池的内存
void myfree(void *ptr);

// 重新分配默认内存池的内存
void *myrealloc(void *ptr, uint32_t size);
```

## 使用示例

### 示例1：基本使用
```c
#include "malloc.h"

// 分配100字节内存（从默认SRAMIN池）
void *ptr1 = mymalloc(100);
if (ptr1 != NULL) {
    // 使用内存...
    myfree(ptr1);  // 释放内存
}

// 分配200字节内存（从CCM内存池，速度更快）
void *ptr2 = mymalloc_memx(SRAMCCM, 200);
if (ptr2 != NULL) {
    // 使用CCM内存（适合DMA或高速访问）...
    myfree_memx(SRAMCCM, ptr2);
}
```

### 示例2：大内存分配（外部SRAM）
```c
// 分配50KB图像缓冲区（从外部SRAM）
void *image_buffer = mymalloc_memx(SRAMEX, 50 * 1024);
if (image_buffer != NULL) {
    // 处理图像数据...
    myfree_memx(SRAMEX, image_buffer);
}
```

### 示例3：内存重新分配
```c
// 初始分配
void *buffer = mymalloc(100);
// ...使用buffer...

// 需要更大空间
buffer = myrealloc(buffer, 200);  // 重新分配为200字节
// ...使用更大的buffer...

myfree(buffer);  // 最终释放
```

### 示例4：查询内存使用率
```c
// 查询SRAMIN内存池使用率
uint16_t usage = my_mem_perused(SRAMIN);
printf("SRAMIN使用率: %.1f%%\n", usage / 10.0);

// 初始化内存池（如果需要）
my_mem_init(SRAMIN);
```

## 应用场景

### 1. 动态数据结构
```c
// 动态创建结构体数组
typedef struct {
    int id;
    float value;
    char name[20];
} SensorData;

SensorData *data_array = (SensorData *)mymalloc(10 * sizeof(SensorData));
if (data_array) {
    // 使用动态数组...
    myfree(data_array);
}
```

### 2. DMA缓冲区管理
```c
// 从CCM内存分配DMA缓冲区（CPU访问速度快）
uint16_t *dma_buffer = (uint16_t *)mymalloc_memx(SRAMCCM, 1024 * sizeof(uint16_t));
if (dma_buffer) {
    // 配置DMA使用此缓冲区...
    // DMA传输完成后...
    myfree_memx(SRAMCCM, dma_buffer);
}
```

### 3. 协议栈缓冲区
```c
// 网络协议栈数据包缓冲区
void *packet_buffer = mymalloc_memx(SRAMEX, 1500);  // 以太网MTU
if (packet_buffer) {
    // 处理网络数据包...
    myfree_memx(SRAMEX, packet_buffer);
}
```

## 配置指南

### 修改内存池大小
在 `malloc.h` 中修改以下宏定义：

```c
// 内部SRAM配置
#define MEM1_BLOCK_SIZE         32      // 块大小（字节）
#define MEM1_MAX_SIZE           50*1024 // 总大小（字节）

// CCM内存配置
#define MEM2_BLOCK_SIZE         32
#define MEM2_MAX_SIZE           60*1024

// 外部SRAM配置
#define MEM3_BLOCK_SIZE         32
#define MEM3_MAX_SIZE           963*1024
```

### 计算公式
- **管理表大小**：`MEMx_ALLOC_TABLE_SIZE = MEMx_MAX_SIZE / MEMx_BLOCK_SIZE`
- **总占用空间**：`总大小 = MEMx_MAX_SIZE + MEMx_ALLOC_TABLE_SIZE * sizeof(MT_TYPE)`

## 注意事项

### 1. 内存对齐
- 内存池按64字节对齐（`__align(64)` 或 `__ALIGNED(64)`）
- 确保DMA访问的内存对齐要求

### 2. 编译器兼容性
- 支持AC5和AC6编译器
- 使用条件编译处理不同编译器的语法差异

### 3. 线程安全性
- 当前实现不是线程安全的
- 在RTOS中使用时需要添加互斥锁保护

### 4. 性能特点
- **分配速度**：O(n)，n为内存表大小
- **释放速度**：O(1)
- **内存开销**：每个内存池需要额外的管理表空间

### 5. 碎片问题
- **内部碎片**：分配不足32字节倍数时产生
- **外部碎片**：固定块大小减少外部碎片，但可能浪费空间

## 与标准malloc的对比

| 特性 | 本实现 | 标准malloc |
|------|--------|------------|
| 可配置内存池 | 支持多个内存池 | 通常单一堆 |
| 块大小 | 固定（可配置） | 可变 |
| 碎片 | 较少外部碎片 | 可能产生外部碎片 |
| 实时性 | 分配时间可预测 | 分配时间不确定 |
| 内存开销 | 管理表额外开销 | 每个块有头部开销 |
| 适合场景 | 嵌入式实时系统 | 通用计算系统 |

## 优化建议

### 1. 根据应用调整块大小
- 小数据频繁分配：使用较小块（如16字节）
- 大数据分配：使用较大块（如64/128字节）
- 混合场景：考虑实现多个不同块大小的内存池

### 2. 添加调试功能
```c
// 可添加的调试功能
void malloc_debug_info(uint8_t memx) {
    printf("内存池%d信息:\n", memx);
    printf("  总大小: %d字节\n", memsize[memx]);
    printf("  块大小: %d字节\n", memblksize[memx]);
    printf("  使用率: %.1f%%\n", my_mem_perused(memx) / 10.0);
    printf("  就绪状态: %s\n", mallco_dev.memrdy[memx] ? "是" : "否");
}
```

### 3. RTOS集成
```c
// 添加互斥锁保护（以FreeRTOS为例）
#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t malloc_mutex;

void malloc_init_with_rtos(void) {
    malloc_mutex = xSemaphoreCreateMutex();
    // 初始化各内存池...
}

void *thread_safe_mymalloc(uint32_t size) {
    xSemaphoreTake(malloc_mutex, portMAX_DELAY);
    void *ptr = mymalloc(size);
    xSemaphoreGive(malloc_mutex);
    return ptr;
}
```

## 总结

`malloc.c` 是一个专为STM32嵌入式系统设计的轻量级内存管理模块，具有以下优势：

1. **多内存池支持**：可管理内部SRAM、CCM内存和外部SRAM
2. **实时性可预测**：固定块大小分配，分配时间相对稳定
3. **碎片控制**：减少外部碎片，适合长期运行的系统
4. **配置灵活**：可通过宏定义调整各内存池参数
5. **编译器兼容**：支持Keil AC5和AC6编译器

适用于需要动态内存管理但又对实时性和可靠性有要求的嵌入式应用，如数据采集、通信协议处理、图形界面等场景。
