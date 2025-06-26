#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#define BUFFER_SIZE 1024
// ./udp_server 192.168.3.20 9092


/*

./udp_client 192.168.3.10 9091 192.168.3.100 9092
./udp_client 192.168.3.20 9091 192.168.3.100 9092


./udp_client 192.168.1.10 9091 192.168.1.20  9092
./udp_client 192.168.2.10 9091 192.168.2.20  9092
./udp_client 192.168.3.10 9091 192.168.3.20  9092
./udp_client 192.168.4.10 9091 192.168.4.20  9092

./udp_client 192.168.1.20 9091 192.168.1.10  9092
./udp_client 192.168.2.20 9091 192.168.2.10  9092
./udp_client 192.168.3.20 9091 192.168.3.10  9092
./udp_client 192.168.4.20 9091 192.168.4.10  9092

*/

/*

./udp_server 192.168.3.10 9092
./udp_server 192.168.3.20 9092

./udp_server 192.168.1.20 9092
./udp_server 192.168.2.20 9092
./udp_server 192.168.3.20 9092
./udp_server 192.168.4.20 9092

./udp_server 192.168.1.10 9092
./udp_server 192.168.2.10 9092
./udp_server 192.168.3.10 9092
./udp_server 192.168.4.10 9092

*/

uint8_t crc8(const void* data, size_t len);
void init_crc8_table(void);

void usage(const char* prog_name) 
{
    printf("Usage: %s <listen_ip> <listen_port>\n", prog_name);
    printf("Example: %s 192.168.3.20 9092\n", prog_name);
    exit(EXIT_FAILURE);
}

void get_cur_system_time(char* pTimBuf, int len)
{
    struct timeval tv;
    struct tm tm_info;
    char timbuffer[64] = "";
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    strftime(timbuffer, len, "%Y-%m-%d %H:%M:%S", &tm_info);
    sprintf(pTimBuf, "%s:%03ld", timbuffer, tv.tv_usec / 1000);
}

void get_cur_system_time_filename(char* pNameBuf, int len, int ethno)
{
    struct timeval tv;
    struct tm tm_info;
    char timbuffer[64] = "";
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
    sprintf(pNameBuf, "./eth_log/eth%d_rx_%s.log", ethno, timbuffer);
}

int main(int argc, char* argv[]) 
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char buffer[BUFFER_SIZE];
    char timbuffer[64] = "";
    char logfilename[64] = "";
    ssize_t nread;
    ssize_t nwrite;
    int frameCnt = 0;
    int i = 0;
    int ipfrg[4] = { 0,0,0,0 };
    FILE* log_file = NULL;

    // 检查命令行参数数量是否正确
    if (argc != 3) 
    {
        usage(argv[0]);
    }

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 初始化服务器地址结构体
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
    {
        perror("Invalid IP address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 绑定套接字到服务器地址
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) 
    {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("UDP server is listening on %s:%d...\n", argv[1], atoi(argv[2]));

    sscanf(argv[1], "%d.%d.%d.%d", &ipfrg[0], &ipfrg[1], &ipfrg[2], &ipfrg[3]);
    printf("ip=%d.%d.%d.%d\n", ipfrg[0], ipfrg[1], ipfrg[2], ipfrg[3]);
    get_cur_system_time_filename(logfilename, sizeof(logfilename), ipfrg[2]);
    log_file = fopen(logfilename, "a");

    while (1) 
    {
        // 接收客户端数据
        

        len = sizeof(cliaddr);
        nread = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,(struct sockaddr*)&cliaddr, &len);
        get_cur_system_time(timbuffer, sizeof(timbuffer));

        if (nread == -1)
        {
            printf("[%s]recvfrom failed frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);
            fprintf(log_file, "[%s]recvfrom failed frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);
            fflush(log_file);
            fsync(fileno(log_file));
            continue;
        }

        printf("[%s]frameCnt = %d nread = %ld\n", timbuffer, frameCnt++, nread);
        fprintf(log_file, "[%s]frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);
        fflush(log_file);
        fsync(fileno(log_file));

        for (i = 0;i < 16;i++)
        {
            printf("%02x ", buffer[i]);
        }

        printf("...");

        for (i = nread - 16;i < nread;i++)
        {
            printf("%02x ", buffer[i]);
        }
        printf("\n\n");

        // 将接收到的数据回显给客户端
        nwrite = sendto(sockfd, buffer, nread, 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr));
        if (nwrite == -1)
        {
            perror("sendto failed\n");
        }
    }
    fclose(log_file);
    close(sockfd);
    return 0;
}
