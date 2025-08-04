#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h> // 使用标准整数类型，确保跨平台的数据宽度一致性

// 定义共享内存的唯一Key，上报程序和所有测试程序都将使用这个Key来访问同一块内存
#define SHM_KEY 1234

// 根据您的设备规格和UI设计，定义端口数量
#define NUM_RS485_PORTS 8
#define NUM_UDP_PORTS   8

// ====================== 核心修改点 ======================
// 使用 #pragma pack(1) 来强制1字节对齐，取消所有填充字节
// 确保所有程序对结构体的内存布局有完全一致的理解
#pragma pack(push, 1)
// =========================================================

/**
 * @brief RS485串口的统计数据结构
 */
typedef struct {
    // 已发送帧数 
    // 由 rs485_test_snd 进程在每次调用 write() 成功后累加
    uint32_t sent_frames;

    // 已接收帧数 
    // 由 rs485_test_snd 进程在每次成功收到回环数据并校验通过后累加
    uint32_t recv_frames;

    // 错误帧数 
    // 由 rs485_test_snd 进程在收到回环数据但CRC校验失败时累加
    uint32_t error_frames;

    // 丢帧数
    // 通过 (sent_frames - recv_frames) 计算得出并更新
    uint32_t dropped_frames;

    // 发送总字节数 
    // 使用64位整数，防止长时间运行后数据溢出
    // 由 rs485_test_snd 累加每次 write() 的返回值
    uint64_t sent_bytes;

    // 接收总字节数
    // 由 rs485_test_snd 累加每次 read() 的返回值
    uint64_t recv_bytes;

} rs485_stat_t;


/**
 * @brief UDP网口的统计数据结构
 */
typedef struct {
    // 已发送包数
    // 由 udp_client 进程在每次调用 sendto() 成功后累加
    uint32_t sent_packets;

    // 已接收包数 
    // 由 udp_client 进程在每次成功收到回环数据包后累加
    uint32_t recv_packets;

    // 超时次数 
    // 由 udp_client 进程在 recvfrom() 返回-1 (超时) 时累加
    uint32_t timeout_count;

    // 发送总字节数
    // 使用64位整数，防止长时间运行后数据溢出
    uint64_t sent_bytes;// 由 udp_client 累加每次 sendto() 的返回值

    // 接收总字节数
    // 由 udp_client 累加每次 recvfrom() 的返回值
    uint64_t recv_bytes;

} udp_stat_t;


/**
 * @brief 共享内存中最终的完整数据布局
 * @note  这是一个包含所有端口统计信息的大结构体
 */
typedef struct {
    // 包含8个RS485串口的统计数据数组
    rs485_stat_t rs485_stats[NUM_RS485_PORTS];

    // 包含4个UDP网口的统计数据数组
    udp_stat_t   udp_stats[NUM_UDP_PORTS];

} emc_stats_t;

// ====================== 核心修改点 ======================
// 恢复之前的对齐设置
#pragma pack(pop)
// =========================================================

#endif // SHARED_DATA_H