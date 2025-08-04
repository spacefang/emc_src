#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Shared_Memory/shared_data.h" // 确保包含完全相同的头文件

int main() {
    int shmid;
    emc_stats_t *stats;

    printf("Reader: Attempting to attach to existing shared memory (key=%d).\n", SHM_KEY);
    printf("Reader: The expected size of the struct is %lu bytes.\n", sizeof(emc_stats_t));

    // 1. 获取已存在的共享内存
    shmid = shmget(SHM_KEY, sizeof(emc_stats_t), 0666);
    if (shmid == -1) {
        perror("Reader: shmget failed. Is the writer running first?");
        exit(EXIT_FAILURE);
    }

    // 2. 附加到地址空间
    stats = (emc_stats_t *)shmat(shmid, NULL, 0);
    if (stats == (void *)-1) {
        perror("Reader: shmat failed");
        exit(EXIT_FAILURE);
    }

    // 3. 读取并打印所有我们关心的字段
    printf("\n--- Reading Shared Memory Data ---\n");

    printf("RS485 Port 0 Stats:\n");
    printf("\tSent Frames:    %u\n", stats->rs485_stats[0].sent_frames);
    printf("\tReceived Frames:  %u\n", stats->rs485_stats[0].recv_frames);
    printf("\tError Frames:     %u\n", stats->rs485_stats[0].error_frames);
    printf("\tDropped Frames:   %u\n", stats->rs485_stats[0].dropped_frames);
    printf("\tSent Bytes:       %llu\n", (unsigned long long)stats->rs485_stats[0].sent_bytes);
    printf("\tReceived Bytes:   %llu\n", (unsigned long long)stats->rs485_stats[0].recv_bytes);

    printf("\nUDP Port 0 Stats:\n");
    printf("\tSent Packets:     %u\n", stats->udp_stats[0].sent_packets);
    printf("\tReceived Packets:   %u\n", stats->udp_stats[0].recv_packets);
    printf("\tTimeout Count:    %u\n", stats->udp_stats[0].timeout_count);
    printf("\tSent Bytes:       %llu\n", (unsigned long long)stats->udp_stats[0].sent_bytes);
    printf("\tReceived Bytes:   %llu\n", (unsigned long long)stats->udp_stats[0].recv_bytes);

    printf("\nReader: Read complete. Detaching from shared memory.\n");

    // 4. 脱离共享内存
    shmdt(stats);

    return 0;
}