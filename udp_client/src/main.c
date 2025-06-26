#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h> 

#define BUFFER_SIZE 1024
// ./udp_client 192.168.3.10 9091 192.168.3.100 9092 （本地ip port 服务端ip port)

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
	printf("Usage: %s <server_ip> <port> <message>\n", prog_name);
	printf("Example: %s 192.168.3.10 9091 192.168.3.100 9092", prog_name);
	
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
	sprintf(pTimBuf,"%s:%03ld", timbuffer, tv.tv_usec / 1000);
}

void get_cur_system_time_filename(char* pNameBuf, int len,int ethno)
{
	struct timeval tv;
	struct tm tm_info;
	char timbuffer[64] = "";
	char datebuffer[64] = "";
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);
	strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);
	sprintf(pNameBuf, "./eth_log/%s/eth%d_tx_%s.log", datebuffer, ethno, timbuffer);
}

void get_cur_system_date_dirname(char* pNameBuf, int len)
{
	struct timeval tv;
	struct tm tm_info;
	char datebuffer[64] = "";
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);
	sprintf(pNameBuf, "./eth_log/%s", datebuffer);
}

int main(int argc, char* argv[])
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	char buffer[BUFFER_SIZE];
	char timbuffer[64] = "";
	char logfilename[64] = "";
	char logdirname[64] = "";
	socklen_t len;
	ssize_t nread;
	ssize_t nwrite;
	int sndframeCnt = 0;
	int rcvframeCnt = 0;
	int i = 0;
	int ipfrg[4] = { 0,0,0,0 };
	char* sysgpiocmd[2][4] = {
		{"echo 0 > /sys/class/gpio/gpio206/value","echo 0 > /sys/class/gpio/gpio207/value","echo 0 > /sys/class/gpio/gpio208/value","echo 0 > /sys/class/gpio/gpio209/value"},
		{"echo 1 > /sys/class/gpio/gpio206/value","echo 1 > /sys/class/gpio/gpio207/value","echo 1 > /sys/class/gpio/gpio208/value","echo 1 > /sys/class/gpio/gpio209/value" }
	};
	FILE* log_file = NULL;
	if (argc != 5)
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

	// 初始化本地地址结构体
	memset(&cliaddr, 0, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &cliaddr.sin_addr) <= 0)
	{
		perror("Invalid IP address");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// 绑定套接字到本地地址
	if (bind(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) == -1)
	{
		perror("bind failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// 初始化服务器地址结构体
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[4]));
	if (inet_pton(AF_INET, argv[3], &servaddr.sin_addr) <= 0)
	{
		perror("Invalid IP address");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < BUFFER_SIZE;i++)
	{
		buffer[i] = i;

	}
	/* 设置超时*/
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
	{
		perror("setsockopt failed:");
	}

	sscanf(argv[1], "%d.%d.%d.%d", &ipfrg[0], &ipfrg[1], &ipfrg[2], &ipfrg[3]);
	printf("ip=%d.%d.%d.%d\n", ipfrg[0], ipfrg[1], ipfrg[2], ipfrg[3]);
	get_cur_system_time_filename(logfilename, sizeof(logfilename), ipfrg[2]);
	get_cur_system_date_dirname(logdirname, sizeof(logdirname));
	if (access(logdirname, F_OK) != 0)
	{
		mkdir(logdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	log_file = fopen(logfilename, "a");

	while (1)
	{
		// 向服务器发送数据

		for (i = 0; i < BUFFER_SIZE;i++)
		{
			buffer[i] = i;
		}
		// buffer[BUFFER_SIZE - 1] = crc8(BUFFER_SIZE, RCV_BUF_LEN - 1);
		nwrite = sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if (nwrite == -1)
		{
			perror("sendto failed\n");
			continue;
		}

		get_cur_system_time(timbuffer, sizeof(timbuffer));

		//printf("send to server  (%s:%d): nwrite = %ld\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), nwrite);
		printf("[%s]sndframeCnt = %d nwrite = %ld\n", timbuffer, sndframeCnt++, nwrite);

		usleep(200000);

		// 接收服务器回显的数据
		for (i = 0; i < BUFFER_SIZE;i++)
		{
			buffer[i] = 0;
		}

	    len = sizeof(servaddr);
		nread = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, &len);
		get_cur_system_time(timbuffer, sizeof(timbuffer));
		if (nread == -1)
		{
			printf("[%s]recvfrom failed badcnt = %d\n", timbuffer, sndframeCnt- rcvframeCnt);
			fprintf(log_file, "[%s]recvfrom failed badcnt = %d\n", timbuffer, sndframeCnt - rcvframeCnt);
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %ld nread = %ld\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);
			fsync(fileno(log_file));
			system(sysgpiocmd[1][ipfrg[2]-1]);
			usleep(5000);
			system(sysgpiocmd[0][ipfrg[2]-1]);
			continue;
		}

		//printf("recv from client  (%s:%d): nread = %ld \n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), nread);

		printf("[%s]rcvframeCnt = %d nread = %ld\n", timbuffer, rcvframeCnt++, nread);
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
		
		fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %ld nread = %ld\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
		fflush(log_file);
		fsync(fileno(log_file));	
	}
	fclose(log_file);
	close(sockfd);
	return 0;
}
