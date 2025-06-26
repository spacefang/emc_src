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
	int fd = -1;
	int nread = 0;
	int frameCnt = 0;
	int rt = -1;
	int i = 0;
	int badCrcCnt = 0;
	uint8_t crc = 0x00;
	char buffer[MAX_SND_BUF_LEN];
	char logfilename[64] = "";
	char timbuffer[64] = "";
	FILE* log_file = NULL;
	int rs485no = atoi(argv[1]);
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

	init_crc8_table();

	get_cur_system_time_filename(logfilename, sizeof(logfilename), rs485no);
	log_file = fopen(logfilename, "a");

	while (1)
	{
		nread = read(fd, buffer, RCV_BUF_LEN);
		get_cur_system_time(timbuffer, sizeof(timbuffer));
		printf("[%s]frameCnt = %d nread = %d\n", timbuffer, frameCnt++, nread);

		for (i = 0;i < 16;i++)
		{
			printf("%02x ", buffer[i]);
		}

		printf("...");

		if (nread >= RCV_BUF_LEN)
		{
		

		for (i = nread - 16;i < nread;i++)
		{
			printf("%02x ", buffer[i]);
		}
	}
		printf("\n\n");
		crc = crc8(buffer, RCV_BUF_LEN - 1);
		if (crc != buffer[RCV_BUF_LEN - 1])
		{
			badCrcCnt++;
			get_cur_system_time(timbuffer, sizeof(timbuffer));
			fprintf(log_file, "[%s] badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			fflush(log_file);
			fsync(fileno(log_file));
			printf("[%s] badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			clear_serial_buffer(fd);
			continue;
		}

		write(fd, buffer, RCV_BUF_LEN);
	}

	fclose(log_file);
	close(fd);
	return 0;
}
