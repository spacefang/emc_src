#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <stddef.h> // <--- 新增头文件，用于 offsetof 宏
// ================== 新增头文件 ==================
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../../Shared_Memory/shared_data.h" 
// ===============================================
#define RCV_BUF_LEN 64
#define SND_BUF_LEN 64
#define MAX_SND_BUF_LEN 256
uint8_t crc8(const void* data, size_t len);
void init_crc8_table(void);

// 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f

// 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 5a 5a 5a 5a 5a 5a 5a 5a   a5 a5 a5 a5 a5 a5 a5 a5   5a 5a 5a 5a 5a 5a 5a 5a   a5 a5 a5 a5 a5 a5 a5 a9
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (nBits)
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}
	switch (nEvent)
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}
	switch (nSpeed)
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if (nStop == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if (nStop == 2)
		newtio.c_cflag |= CSTOPB;

	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}

int open_port(int fd, int comport)
{
	char* dev[] = { "/dev/ttyCH9344USB0","/dev/ttyCH9344USB1","/dev/ttyCH9344USB2","/dev/ttyCH9344USB3","/dev/ttyCH9344USB4","/dev/ttyCH9344USB5", "/dev/ttyCH9344USB6","/dev/ttyCH9344USB7" };
	fd = open(dev[comport], O_RDWR | O_NOCTTY | O_NDELAY);

	if (-1 == fd) {
		perror("Can't Open Serial Port");
		return(-1);
	}
	else
		printf("open %s .....\n", dev[comport]);

	if (fcntl(fd, F_SETFL, 0) < 0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));

	if (isatty(STDIN_FILENO) == 0)
		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");

	printf("fd-open=%d\n", fd);
	return fd;

}

int clear_serial_buffer(int fd) 
{
	if (tcflush(fd, TCIOFLUSH) != 0) 
	{
		perror("tcflush");
		close(fd);
		return -1;
	}
	return 0;
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

void get_cur_system_time_filename(char* pNameBuf, int len, int no)
{
	struct timeval tv;
	struct tm tm_info;
	char timbuffer[64] = "";
	char datebuffer[64] = "";
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);
	strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);
	sprintf(pNameBuf, "./serial_log/%s/serial%d_tx_%s.log", datebuffer, no, timbuffer);
}

void get_cur_system_date_dirname(char* pNameBuf, int len)
{
	struct timeval tv;
	struct tm tm_info;
	char datebuffer[64] = "";
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);
	sprintf(pNameBuf, "./serial_log/%s", datebuffer);
}

int main(int argc, char* argv[])
{
	int fd = -1;
	int nread = 0;
	int nwrite = 0;
	int sndframeCnt = 0;
	int rcvframeCnt = 0;
	int rt = -1;
	int i = 0;
	int badCrcCnt = 0;
	uint8_t crc = 0x00;
	char buffer[MAX_SND_BUF_LEN];
	char logfilename[64] = "";
	char logdirname[64] = "";
	char timbuffer[64] = "";
	char* sysgpiocmd[2][8] = {
	{"echo 0 > /sys/class/gpio/gpio200/value","echo 0 > /sys/class/gpio/gpio201/value","echo 0 > /sys/class/gpio/gpio202/value","echo 0 > /sys/class/gpio/gpio204/value","echo 0 > /sys/class/gpio/gpio11/value","echo 0 > /sys/class/gpio/gpio12/value","echo 0 > /sys/class/gpio/gpio16/value","echo 0 > /sys/class/gpio/gpio17/value"},
	{"echo 1 > /sys/class/gpio/gpio200/value","echo 1 > /sys/class/gpio/gpio201/value","echo 1 > /sys/class/gpio/gpio202/value","echo 1 > /sys/class/gpio/gpio204/value","echo 1 > /sys/class/gpio/gpio11/value","echo 1 > /sys/class/gpio/gpio12/value","echo 1 > /sys/class/gpio/gpio16/value","echo 1 > /sys/class/gpio/gpio17/value" }
	};
	FILE* log_file = NULL;

	// ================== 新增代码段 1: 初始化共享内存 ==================
    int shmid;
    emc_stats_t *shared_stats = NULL;

    // 1. 获取共享内存ID
    // 使用IPC_CREAT标志，如果共享内存不存在，则创建它
    shmid = shmget(SHM_KEY, sizeof(emc_stats_t), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // 2. 将共享内存附加到本进程的地址空间
    shared_stats = (emc_stats_t *)shmat(shmid, NULL, 0);
    if (shared_stats == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
	//=============================核心诊断代码 ========================
	printf("\n--- rs485_test_snd: Memory Layout Diagnostics ---\n");
    printf("sizeof(emc_stats_t) according to this program is: %lu\n", sizeof(emc_stats_t));
    printf("Offset of rs485_stats[0].sent_bytes is: %zu\n", offsetof(emc_stats_t, rs485_stats[0].sent_bytes));
    printf("Offset of rs485_stats[0].recv_bytes is: %zu\n", offsetof(emc_stats_t, rs485_stats[0].recv_bytes));
    printf("Offset of udp_stats[0].sent_bytes is:   %zu\n", offsetof(emc_stats_t, udp_stats[0].sent_bytes));
    printf("Offset of udp_stats[0].recv_bytes is:   %zu\n", offsetof(emc_stats_t, udp_stats[0].recv_bytes));

    //printf("Successfully attached to shared memory for RS485 statistics.\n");
    // ================================================================
	int rs485no = atoi(argv[1]);
	if ((fd = open_port(fd, rs485no)) < 0)
	{
		perror("open_port error");
		return -1;
	}
	if ((rt = set_opt(fd, 115200, 8, 'N', 1)) < 0)
	{
		perror("set_opt error");
		return -1;
	}
	printf("fd=%d\n", fd);

	init_crc8_table();


	get_cur_system_time_filename(logfilename, sizeof(logfilename), rs485no);
	get_cur_system_date_dirname(logdirname, sizeof(logdirname));
	if (access(logdirname, F_OK) != 0)
	{
		mkdir(logdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	log_file = fopen(logfilename, "a");

	while (1)
	{
		
		for (i = 0; i < 32;i++)
		{
			buffer[i] = i;

		}

		for (i = 32; i < 40;i++)
		{
			buffer[i] = 0x5a;
			buffer[i + 16] = 0x5a;
		}

		for (i = 40; i < 48;i++)
		{
			buffer[i] = 0xa5;
			buffer[i + 16] = 0xa5;
		}
		buffer[RCV_BUF_LEN - 1] = crc8(buffer, RCV_BUF_LEN - 1);

		//发送数据
		nwrite = write(fd, buffer, RCV_BUF_LEN);
		if (nwrite > 0) {
			sndframeCnt++;
			// ================== 修改/新增: 更新共享内存的发送统计 ==================
			if (shared_stats != NULL && rs485no >= 0 && rs485no < NUM_RS485_PORTS) {
				shared_stats->rs485_stats[rs485no].sent_frames = sndframeCnt;
				shared_stats->rs485_stats[rs485no].sent_bytes += nwrite;
			}
			// ===================================================================
		}

		get_cur_system_time(timbuffer, sizeof(timbuffer));

		printf("[%s]sndframeCnt = %d nwrite = %d\n", timbuffer, sndframeCnt, nwrite);

		// 接收服务器回显的数据
		for (i = 0; i < RCV_BUF_LEN;i++)
		{
			buffer[i] = 0;
		}
		usleep(100000);
		nread = read(fd, buffer, RCV_BUF_LEN);

		if (nread <= 0)
		{
			printf("[%s]No data on serial%d cnt = %d\n", timbuffer, rs485no, sndframeCnt - rcvframeCnt);
			fprintf(log_file, "[%s]No data on serial%d cnt = %d\n", timbuffer, rs485no, sndframeCnt- rcvframeCnt);
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);
			fsync(fileno(log_file));
			//continue;
		}
		//==========================新增成功读到数据才能更新接收字节数==================================
		else{
			if (shared_stats != NULL && rs485no >= 0 && rs485no < NUM_RS485_PORTS) {
				shared_stats->rs485_stats[rs485no].recv_bytes += nread;
			}
		}
		//===========================================================================================

		get_cur_system_time(timbuffer, sizeof(timbuffer));


		//CRC校验
		crc = crc8(buffer, RCV_BUF_LEN - 1);
		//if (crc != buffer[RCV_BUF_LEN - 1])
		if (nread > 0 && crc != buffer[RCV_BUF_LEN - 1])
		{
			badCrcCnt++;
			// ================== 修改/新增: 更新共享内存的错误帧数 ==================
			if (shared_stats != NULL && rs485no >= 0 && rs485no < NUM_RS485_PORTS) {
				shared_stats->rs485_stats[rs485no].error_frames = badCrcCnt;
			}
			// =====================================================================
			printf("[%s]recvfrom failed badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			fprintf(log_file, "[%s]recvfrom failed badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);
			fsync(fileno(log_file));
			system(sysgpiocmd[1][rs485no]);
			usleep(5000);
			system(sysgpiocmd[0][rs485no]);
			clear_serial_buffer(fd);
			//continue;
		}
		//==============================新共享内存的接收帧数==============================
		else if (nread > 0) // 校验成功
		{
			rcvframeCnt++;
			if (shared_stats != NULL && rs485no >= 0 && rs485no < NUM_RS485_PORTS) {
				shared_stats->rs485_stats[rs485no].recv_frames = rcvframeCnt;
			}	
		}
		// ===============================================================================

		// ================== 新增代码段 2: 更新计算字段和最终打印 ==================
        if (shared_stats != NULL && rs485no >= 0 && rs485no < NUM_RS485_PORTS) {
            // 更新丢帧数
            shared_stats->rs485_stats[rs485no].dropped_frames = sndframeCnt - rcvframeCnt;
        }

		// (原有日志和打印保持不变)
		printf("[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
		fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
		fflush(log_file);
		fsync(fileno(log_file));

		// (原有数据内容打印保持不变)
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
        // =======================================================================
	}


	// ================== 新增代码段 3: 脱离共享内存 ==================
    if (shared_stats != NULL && shmdt(shared_stats) == -1) {
        perror("shmdt failed");
    }
    // ================================================================

	fclose(log_file);
	close(fd);
	return 0;
}

