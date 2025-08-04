#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Shared_Memory/shared_data.h" // 假设shared_data.h在此路径

int main() {
    int shmid;
    emc_stats_t *stats;

    printf("Writer: Attempting to create and attach to shared memory (key=%d).\n", SHM_KEY);
    printf("Writer: The expected size of the struct is %lu bytes.\n", sizeof(emc_stats_t));

    // 1. 获取或创建共享内存
    shmid = shmget(SHM_KEY, sizeof(emc_stats_t), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Writer: shmget failed");
        exit(EXIT_FAILURE);
    }

    // 2. 附加到地址空间
    stats = (emc_stats_t *)shmat(shmid, NULL, 0);
    if (stats == (void *)-1) {
        perror("Writer: shmat failed");
        exit(EXIT_FAILURE);
    }

    // 3. 将整个区域清零 (良好的初始习惯)
    memset(stats, 0, sizeof(emc_stats_t));

    // 4. 写入固定的、已知的测试数据
    printf("Writer: Writing predefined values into shared memory...\n");

    // 写入串口 COM0 的数据
    stats->rs485_stats[0].sent_frames = 111;
    stats->rs485_stats[0].recv_frames = 110;
    stats->rs485_stats[0].error_frames = 1;
    stats->rs485_stats[0].dropped_frames = 1;
    stats->rs485_stats[0].sent_bytes = 1000000001; // 一个uint64_t的测试值
    stats->rs485_stats[0].recv_bytes = 1000000002;

    // 写入网口 独立网口1 (索引为0) 的数据
    stats->udp_stats[0].sent_packets = 222;
    stats->udp_stats[0].recv_packets = 221;
    stats->udp_stats[0].timeout_count = 1;
    stats->udp_stats[0].sent_bytes = 2000000001;
    stats->udp_stats[0].recv_bytes = 2000000002;

    printf("Writer: Write complete. Detaching from shared memory.\n");

    // 5. 脱离共享内存
    shmdt(stats);

    return 0;
}