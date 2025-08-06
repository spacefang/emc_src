#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> // 引入errno.h用于错误处理
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/select.h> // 引入select头文件

// 确保这里的路径与您的项目结构一致
#include "shared_data.h"

// ======================= 配置区 =======================
// 请将此IP地址修改为您PC端运行Qt程序的以太网卡地址
#define SERVER_IP "192.168.3.100"
// 与Qt程序监听的端口一致
#define SERVER_PORT 54321
// ======================================================

// 定义协议中的报文和指令类型
#define PKT_TYPE_RS485  0xA1
#define PKT_TYPE_UDP    0xB1
#define CMD_CLEAR_STATS 0xD1 //清零指令
#define CMD_START_ALL 0xE1 //开始所有测试的指令
#define CMD_STOP_ALL 0xE2 //停止所有测试的指令

/**
 * @brief 计算CRC16-Modbus校验码
 * @param buf 需要计算的数据缓冲区
 * @param len 数据的长度
 * @return 16位的CRC校验码
 */
uint16_t crc16_modbus(const unsigned char *buf, unsigned int len) {
    uint16_t crc = 0xFFFF;
    for (unsigned int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}


int main() {
    int shmid;
    emc_stats_t *shared_stats;
    int sock = 0;
    struct sockaddr_in serv_addr;

    // --- 步骤 1: 获取或创建共享内存 ---
    shmid = shmget(SHM_KEY, sizeof(emc_stats_t), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Reporter Error: shmget failed. Check permissions.");
        exit(EXIT_FAILURE);
    }

    shared_stats = (emc_stats_t *)shmat(shmid, NULL, 0);
    if (shared_stats == (void *)-1) {
        perror("Reporter Error: shmat failed.");
        exit(EXIT_FAILURE);
    }

    // --- 步骤 2: 初始化共享内存 ---
    printf("Reporter: Initializing shared memory segment (key=%d) to zero...\n", SHM_KEY);
    memset(shared_stats, 0, sizeof(emc_stats_t));
    printf("Reporter: Shared memory initialized successfully.\n");


    // --- 步骤 3: 创建TCP客户端并连接到上位机 ---
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Reporter Error: Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Reporter Error: Invalid server IP address");
        return -1;
    }

    printf("Reporter: Connecting to monitoring server at %s:%d...\n", SERVER_IP, SERVER_PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Reporter Error: Connection Failed. Is the server running and reachable?");
        return -1;
    }
    printf("Reporter: Connection successful. Starting data reporting...\n");

    // ====================== 新增：发送身份识别信息 ======================
    const char* id_message = "ID:SENDER";
    send(sock, id_message, strlen(id_message), 0);
    printf("Reporter: Sent identification 'ID:SENDER' to server.\n");
    // =================================================================

    // --- 步骤 4: 进入主循环，使用select来同时处理发送和接收 ---
    fd_set read_fds;
    struct timeval timeout;

    while (1) {
        // ---- 准备select ----
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds); // 将我们的socket加入到监听集合中

        // 设置超时为1秒，这样我们的数据上报逻辑还能每秒执行一次
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // ---- 使用select进行非阻塞式等待 ----
        int activity = select(sock + 1, &read_fds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // ---- 检查是否有来自服务器的指令 (只有当select报告socket可读时才执行) ----
        if (FD_ISSET(sock, &read_fds)) {
            char command_buffer[16];
            int n = read(sock, command_buffer, sizeof(command_buffer) - 1);
            if (n > 0) {
                // 如果收到的是清零指令
                // --- 修改这里的逻辑 ---
                switch(command_buffer[0]) {
                    case CMD_CLEAR_STATS:
                        printf("Reporter: Received 'Clear Stats' command (0xD1).\n");
                        memset(shared_stats, 0, sizeof(emc_stats_t));
                        printf("Reporter: Shared memory cleared.\n");
                        break;
                    case CMD_START_ALL:
                        printf("Reporter: Received 'Start All' command (0xE1).\n");
                        system("/mnt/EMMC/emc_test01/start_all.sh"); // 执行启动脚本
                        break;
                    case CMD_STOP_ALL:
                        printf("Reporter: Received 'Stop All' command (0xE2).\n");
                        system("/mnt/EMMC/emc_test01/stop_all.sh"); // 执行停止脚本
                        break;
                    default:
                        printf("Reporter: Received unknown command.\n");
                }
            } else {
                // 读取错误或返回0，意味着连接已断开
                printf("Reporter: Connection closed by server. Exiting.\n");
                break; // 退出主循环
            }
        }

        // ---- 执行每秒一次的数据上报逻辑 ----
        // 无论select是超时返回还是收到数据后返回，这部分代码都会执行，保证了数据每秒上报

        // 4.1 上报所有RS485串口的数据
        for (int i = 0; i < NUM_RS485_PORTS; i++) {
            unsigned char rs485_packet[38];
            rs485_packet[0] = PKT_TYPE_RS485;
            rs485_packet[1] = sizeof(rs485_packet);
            rs485_packet[2] = i;
            memcpy(&rs485_packet[3],  &shared_stats->rs485_stats[i].sent_frames, 4);
            memcpy(&rs485_packet[7],  &shared_stats->rs485_stats[i].recv_frames, 4);
            memcpy(&rs485_packet[11], &shared_stats->rs485_stats[i].error_frames, 4);
            memcpy(&rs485_packet[15], &shared_stats->rs485_stats[i].dropped_frames, 4);
            memcpy(&rs485_packet[19], &shared_stats->rs485_stats[i].sent_bytes, 8);
            memcpy(&rs485_packet[27], &shared_stats->rs485_stats[i].recv_bytes, 8);
            uint16_t crc = crc16_modbus(rs485_packet, 36);
            memcpy(&rs485_packet[36], &crc, 2);
            send(sock, rs485_packet, sizeof(rs485_packet), 0);
        }

        // 4.2 上报所有UDP网口的数据
        for (int i = 0; i < NUM_UDP_PORTS; i++) {
            unsigned char udp_packet[34];
            udp_packet[0] = PKT_TYPE_UDP;
            udp_packet[1] = sizeof(udp_packet);
            udp_packet[2] = i;
            memcpy(&udp_packet[3],  &shared_stats->udp_stats[i].sent_packets, 4);
            memcpy(&udp_packet[7],  &shared_stats->udp_stats[i].recv_packets, 4);
            memcpy(&udp_packet[11], &shared_stats->udp_stats[i].timeout_count, 4);
            memcpy(&udp_packet[15], &shared_stats->udp_stats[i].sent_bytes, 8);
            memcpy(&udp_packet[23], &shared_stats->udp_stats[i].recv_bytes, 8);
            uint16_t crc = crc16_modbus(udp_packet, 32);
            memcpy(&udp_packet[32], &crc, 2);
            send(sock, udp_packet, sizeof(udp_packet), 0);
        }

        printf("Reporter: All statistics for this cycle have been sent.\n");
    }

    // --- 步骤 5: 清理工作 ---
    printf("Reporter: Shutting down.\n");
    close(sock);
    if (shmdt(shared_stats) == -1) {
        perror("Reporter Error: shmdt failed");
    }

    return 0;
}