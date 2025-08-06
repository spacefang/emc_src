#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// ======================= 配置区 =======================
// 同样需要连接到您的PC端上位机
#define SERVER_IP "192.168.3.100"
#define SERVER_PORT 54321
// ======================================================

// 定义需要响应的指令
#define CMD_START_ALL   0xE1
#define CMD_STOP_ALL    0xE2

// 定义脚本的绝对路径 (请根据您在接收端设备上的实际存放路径修改)
#define START_SCRIPT_PATH "/mnt/EMMC/emc_test01/start_all.sh"
#define STOP_SCRIPT_PATH  "/mnt/EMMC/emc_test01/stop_all.sh"


int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // --- 步骤 1: 创建TCP客户端并连接到上位机 ---
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Receiver Reporter Error: Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Receiver Reporter Error: Invalid server IP address");
        return -1;
    }

    printf("Receiver Reporter: Connecting to monitoring server at %s:%d...\n", SERVER_IP, SERVER_PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Receiver Reporter Error: Connection Failed. Is the server running and reachable?");
        return -1;
    }
    printf("Receiver Reporter: Connection successful. Waiting for commands...\n");

    // ====================== 新增：发送身份识别信息 ======================
    const char* id_message = "ID:RECEIVER";
    send(sock, id_message, strlen(id_message), 0);
    printf("Receiver Reporter: Sent identification 'ID:RECEIVER' to server.\n");
    // =================================================================
    // --- 步骤 2: 进入主循环，只负责接收指令 ---
    while (1) {
        char command_buffer[16];
        // 使用阻塞式 read，一直等待，直到收到数据或连接断开
        int n = read(sock, command_buffer, sizeof(command_buffer) - 1);

        if (n > 0) {
            // 根据收到的指令执行相应的操作
            switch(command_buffer[0]) {
                case CMD_START_ALL:
                    printf("Receiver Reporter: Received 'Start All' command (0xE1).\n");
                    system(START_SCRIPT_PATH); // 执行启动脚本
                    break;
                case CMD_STOP_ALL:
                    printf("Receiver Reporter: Received 'Stop All' command (0xE2).\n");
                    system(STOP_SCRIPT_PATH); // 执行停止脚本
                    break;
                default:
                    printf("Receiver Reporter: Received unknown command (0x%02X).\n", (unsigned char)command_buffer[0]);
            }
        } else {
            // 读取错误或返回0，意味着连接已断开
            if (n == 0) {
                printf("Receiver Reporter: Connection closed by server. Exiting.\n");
            } else {
                perror("Receiver Reporter: Read error");
            }
            break; // 退出主循环
        }
    }

    // --- 步骤 3: 清理工作 ---
    printf("Receiver Reporter: Shutting down.\n");
    close(sock);

    return 0;
}