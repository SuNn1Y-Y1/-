/**
 * DMA_INIT 函数使用示例
 * 展示了如何使用新的整合函数替代原有的 DMA_INIT_IN 和 DMA_INIT_OUT 函数
 */

#include "stm32f10x.h"
#include "dma.h"

// 示例1：ADC DMA 采集（外设到内存）
void Example_ADC_DMA_Init(void)
{
    // 定义ADC采集缓冲区
    uint16_t adc_buffer[100];
    
    // 使用新的整合函数初始化ADC DMA（外设到内存）
    // 参数说明：
    // DMA1_Channel1: DMA通道
    // (uint32_t)&ADC1->DR: ADC数据寄存器地址
    // (uint32_t)adc_buffer: 内存缓冲区地址
    // 100: 缓冲区大小（采集100个点）
    // DMA_DIR_PERIPHERAL_TO_MEMORY: 传输方向（外设到内存）
    DMA_INIT(DMA1_Channel1,
             (uint32_t)&ADC1->DR,
             (uint32_t)adc_buffer,
             100,
             DMA_DIR_PERIPHERAL_TO_MEMORY);
    
    printf("ADC DMA 初始化完成（外设到内存）\r\n");
    printf("应用场景：ADC数据自动采集，适合数据采集系统、传感器监测等\r\n");
    printf("buffsize=100 表示采集100个ADC采样点，每个点16位（半字）\r\n");
}

// 示例2：串口发送DMA（内存到外设）
void Example_USART_DMA_Init(void)
{
    // 定义要发送的数据
    uint8_t tx_data[] = "Hello, World!";
    
    // 使用新的整合函数初始化USART发送DMA（内存到外设）
    // 参数说明：
    // DMA1_Channel4: DMA通道（USART1_TX对应DMA1_Channel4）
    // (uint32_t)&USART1->DR: USART数据寄存器地址
    // (uint32_t)tx_data: 内存数据地址
    // sizeof(tx_data): 数据长度
    // DMA_DIR_MEMORY_TO_PERIPHERAL: 传输方向（内存到外设）
    DMA_INIT(DMA1_Channel4,
             (uint32_t)&USART1->DR,
             (uint32_t)tx_data,
             sizeof(tx_data),
             DMA_DIR_MEMORY_TO_PERIPHERAL);
    
    printf("USART DMA 初始化完成（内存到外设）\r\n");
    printf("应用场景：串口数据自动发送，适合通信协议、数据上传等\r\n");
    printf("buffsize=%d 表示发送%d字节数据\r\n", sizeof(tx_data), sizeof(tx_data));
}

// 示例3：SPI接收DMA（外设到内存）
void Example_SPI_DMA_Init(void)
{
    // 定义SPI接收缓冲区
    uint8_t spi_rx_buffer[256];
    
    // 使用新的整合函数初始化SPI接收DMA（外设到内存）
    DMA_INIT(DMA1_Channel2,
             (uint32_t)&SPI1->DR,
             (uint32_t)spi_rx_buffer,
             256,
             DMA_DIR_PERIPHERAL_TO_MEMORY);
    
    printf("SPI DMA 初始化完成（外设到内存）\r\n");
    printf("应用场景：SPI从设备数据接收，适合传感器读取、存储器访问等\r\n");
    printf("buffsize=256 表示接收256字节数据，适合批量数据传输\r\n");
}

// 示例4：比较新旧函数用法
void Example_Compare_Old_New(void)
{
    uint16_t buffer[50];
    
    // 旧方法：使用两个独立的函数
    printf("=== 旧方法（使用两个独立函数）===\r\n");
    DMA_INIT_IN(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)buffer, 50);
    printf("DMA_INIT_IN: 外设到内存传输\r\n");
    
    DMA_INIT_OUT(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)buffer, 50);
    printf("DMA_INIT_OUT: 内存到外设传输\r\n");
    
    // 新方法：使用整合函数
    printf("\n=== 新方法（使用整合函数）===\r\n");
    DMA_INIT(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)buffer, 50, 
             DMA_DIR_PERIPHERAL_TO_MEMORY);
    printf("DMA_INIT: 外设到内存传输\r\n");
    
    DMA_INIT(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)buffer, 50,
             DMA_DIR_MEMORY_TO_PERIPHERAL);
    printf("DMA_INIT: 内存到外设传输\r\n");
    
    printf("\n优势：\n");
    printf("1. 代码更简洁，一个函数代替两个函数\n");
    printf("2. 通过direction参数明确指定传输方向\n");
    printf("3. 便于代码维护和扩展\n");
}

// 示例5：不同buffsize的应用场景
void Example_Buffsize_Scenarios(void)
{
    printf("=== buffsize参数应用场景示例 ===\r\n");
    
    // 场景1：小缓冲区 - 实时控制
    printf("1. 小缓冲区（buffsize=1-10）：\n");
    printf("   - 实时控制应用\n");
    printf("   - 单次触发传输\n");
    printf("   - 低延迟要求\n");
    
    // 场景2：中等缓冲区 - 数据采集
    printf("\n2. 中等缓冲区（buffsize=100-1000）：\n");
    printf("   - 数据采集系统\n");
    printf("   - 音频处理（256, 512, 1024等）\n");
    printf("   - 批量数据传输\n");
    
    // 场景3：大缓冲区 - 图像处理
    printf("\n3. 大缓冲区（buffsize=1000-65535）：\n");
    printf("   - 图像数据传输\n");
    printf("   - 大文件传输\n");
    printf("   - 注意：DMA最大支持65535\n");
    
    // 实际示例
    uint16_t small_buffer[10];     // 小缓冲区
    uint16_t medium_buffer[256];   // 中等缓冲区
    uint16_t large_buffer[2048];   // 大缓冲区
    
    printf("\n实际配置示例：\n");
    printf("- 实时控制：buffsize=1，循环模式\n");
    printf("- 音频处理：buffsize=256，双缓冲区\n");
    printf("- 图像传输：buffsize=320*240=76800（需要分块）\n");
}

// 主示例函数
void DMA_Usage_Examples(void)
{
    printf("=== DMA_INIT 整合函数使用示例 ===\r\n\n");
    
    // 示例1：ADC DMA
    Example_ADC_DMA_Init();
    printf("\n");
    
    // 示例2：USART DMA
    Example_USART_DMA_Init();
    printf("\n");
    
    // 示例3：SPI DMA
    Example_SPI_DMA_Init();
    printf("\n");
    
    // 示例4：比较新旧函数
    Example_Compare_Old_New();
    printf("\n");
    
    // 示例5：buffsize应用场景
    Example_Buffsize_Scenarios();
    printf("\n");
    
    printf("=== 示例结束 ===\r\n");
    printf("总结：新的DMA_INIT函数通过direction参数统一了传输方向配置，\r\n");
    printf("使代码更加清晰、易于维护，同时保留了原有函数的兼容性。\r\n");
}

// 简单的主函数用于测试（如果需要）
int main_example(void)
{
    // 系统初始化
    SystemInit();
    
    // 串口初始化（如果需要打印信息）
    // usart1_init(115200);
    
    // 运行DMA使用示例
    DMA_Usage_Examples();
    
    while(1)
    {
        // 主循环
    }
    
    return 0;
}
