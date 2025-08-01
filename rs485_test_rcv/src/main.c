/*
这个程序模拟了一个RS485通信链路中的“从设备”或“应答设备”。它的业务流程如下：
启动与初始化：
	程序启动时，需要从命令行接收一个参数，这个参数决定了它要操作哪一个串口设备（例如，0代表/dev/ttyCH9344USB0）。
	打开并配置指定的串口，设置好波特率、数据位、校验位和停止位等参数。
	创建一个日志文件，文件名包含当前的日期和时间，用于记录后续的操作。
数据接收与处理循环：
程序进入一个无限循环，等待从串口接收数据。
一旦接收到数据，就进行处理：
	打印与记录：在屏幕上打印出接收到的原始数据（以十六进制格式），并记录接收事件的时间戳。
	CRC校验：对接收到的数据帧（除了最后一个字节）进行CRC8校验，然后将计算出的CRC值与数据帧的最后一个字节（对方发送过来的CRC值）进行比较。
处理校验结果：
	如果校验成功：说明数据在传输过程中没有出错。程序会通过串口将刚刚收到的完整数据帧再发送回去，完成一次“回显”。
	如果校验失败：说明数据传输有误。程序会记录一条错误日志，包含当前时间和错误次数。然后清空串口的接收缓冲区，丢弃这次的错误数据，并准备接收下一帧数据。
*/
#include <stdlib.h>//atoi 将字符串转换成整数
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
#define RCV_BUF_LEN 64
#define SND_BUF_LEN 64
#define MAX_SND_BUF_LEN 256
uint8_t crc8(const void* data, size_t len);
void init_crc8_table(void);

// 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f
// 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 5a 5a 5a 5a 5a 5a 5a 5a   a5 a5 a5 a5 a5 a5 a5 a5   5a 5a 5a 5a 5a 5a 5a 5a   a5 a5 a5 a5 a5 a5 a5 a9
//根据用户传入的参数，精确地匹配一个已经打开地串口的通信属性
//串口的“代号”(fd)、通信速率(nSpeed)、数据位(nBits)、奇偶校验方式(nEvent)和停止位(nStop)
//[起始位] -> [数据位 (7或8个)] -> [奇偶校验位 (可选)] -> [停止位 (1或2个)]
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

	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = RCV_BUF_LEN;
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
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);
	strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
	sprintf(pNameBuf, "./serial_log/serial%d_rx_%s.log", no, timbuffer);
}

int main(int argc, char* argv[])
{
	int fd = -1;// 文件描述符，用于存放打开的串口的“代号”。
	int nread = 0;// 用于存放read函数实际读取到的字节数。
	int frameCnt = 0;// 帧计数器，用于统计总共接收了多少个数据包。
	int rt = -1;// 通用返回值变量，用于检查函数调用是否成功。
	int i = 0;// 通用的循环计数器。
	int badCrcCnt = 0;// 坏CRC包计数器，统计校验失败的次数。
	uint8_t crc = 0x00;// 用于存放CRC8计算的结果。
	char buffer[MAX_SND_BUF_LEN];// 接收和发送数据的缓冲区。
	char logfilename[64] = "";// 用于存放完整的日志文件名。
	char timbuffer[64] = "";// 用于存放带格式的时间戳字符串。
	FILE* log_file = NULL;// 指向日志文件的文件指针。
	int rs485no = atoi(argv[1]);// 从命令行参数转换来的串口编号。

	//1.打开并配置串口
	if ((fd = open_port(fd,rs485no)) < 0)
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

	//2.准备CRC校验工具
	init_crc8_table();

	//3.创建一个唯一的,带时间戳的日志文件，用于记录后续的每一次接收和处理活动
	get_cur_system_time_filename(logfilename, sizeof(logfilename), rs485no);
	log_file = fopen(logfilename, "a");

	while (1)
	{
		//等待并接收数据
		//程序会阻塞在read函数上，耐心等待，直到串口接收到一个完整的数据包(RCV_BUF_LEN,即64字节)的数据
		nread = read(fd, buffer, RCV_BUF_LEN);
		get_cur_system_time(timbuffer, sizeof(timbuffer));
		printf("[%s]frameCnt = %d nread = %d\n", timbuffer, frameCnt++, nread);

		for (i = 0;i < 16;i++)
		{
			printf("%02x ", buffer[i]);
		}

		printf("...");

		if (nread >= RCV_BUF_LEN)//nread是read函数实际读到的字节数，RCV_BUF_LEN完整帧长度64
		//只有在接受了一个完整或超长的数据包时，才去尝试打印尾部
		{
		

		for (i = nread - 16;i < nread;i++)
		{
			printf("%02x ", buffer[i]);
		}
		}
		printf("\n\n");
		//对接收到的数据进行计算，并与发送方附加的CRC值进行对比。
		crc = crc8(buffer, RCV_BUF_LEN - 1);
		if (crc != buffer[RCV_BUF_LEN - 1])
		{
			badCrcCnt++;//CRC校验失败计数
			get_cur_system_time(timbuffer, sizeof(timbuffer));// 获取当前精确时间
			//将错误信息写入日志文件
			fprintf(log_file, "[%s] badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			//强制将日志缓冲区内容刷入磁盘，确保关键错误信息不丢失
			fflush(log_file);
			fsync(fileno(log_file));
			//同时，也在屏幕上打印出错误信息，方便实时监控
			printf("[%s] badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			//清空串口的硬件接收缓冲区，丢弃可能存在的其他残留的坏数据
			clear_serial_buffer(fd);
			continue;
		}

		//如果上述if条件不成立，程序会跳过整个错误处理块，直接执行到这里
		write(fd, buffer, RCV_BUF_LEN);//将buffer中接收到的完整64字节数据，再跳过串口fd原封不动地发送回去。
	}

	fclose(log_file);
	close(fd);
	return 0;
}
