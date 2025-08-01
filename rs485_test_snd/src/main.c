/*数据发送端 (Sender)
1.产生持续的通信压力：程序会不知疲倦地循环发送数据包。这种不间断的通信是EMC测试的理想模型，因为任何微小的干扰都可能导致数据丢失或错误。
2.验证通信回路：它不仅仅是“盲目地”发送。在发送数据后，它会切换到监听状态，等待接收端（rs485_test_rcv）将数据原封不动地“回声”（Echo）回来。
				能成功收到回声，才证明一次完整的“发送-接收-返回”通信回路是通畅的。

程序的实现逻辑：
循环开始 -> 准备数据包 (序号+1) -> 通过串口发送 -> 尝试从串口接收回声 -> 记录日志 -> 暂停1秒 -> 回到循环开始
				*/
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>//标准输入输出
#include <string.h>//strlen():计算字符串的实际长度
#include <unistd.h>//提供对 POSIX 操作系统 API 的访问，是于底层系统交互的核心。 open() write() read() close() sleep()
#include <fcntl.h>//提供文件控制的选项，通常与 open() 函数配合使用。
#include <errno.h>
#include <termios.h>//提供一个标准的接口来控制异步通信端口，例如设置串口的波特率、数据位、停止位、校验位等。
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
	
	//struct termios 是一个结构体，用于配置终端的属性。它包含了串口通信的各种设置，如波特率、数据位、停止位、校验位等。
	//oldtio 用于保存当前串口的设置，newtio 用于存储新的设置。
	struct termios newtio, oldtio;

	//====准备阶段====

	//1.获取并备份当前串口的设置
	//tcgetattr(fd, &oldtio)：获取当前串口(fd)的设置，并将其存储在 oldtio 中。
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	//2.准备新的配置结构体
	//bezrow(&newtio, sizeof(newtio))：将 newtio 结构体的所有字节设置为 0，确保没有残留的旧数据。以便我们可以从零开始配置新的串口属性。
	//bzero() 函数是一个 POSIX 标准函数，用于将指定内存区域的内容全部设置为零。<string.h> 头文件
	bzero(&newtio, sizeof(newtio));

	//====配置阶段====
	//|按位或  &按位与 ~按位非
	//标志位含义:CLOCAL: (Local Flag) 忽略调制解调器（Modem）的控制线（如 DCD, DSR, CTS, RTS）。
	// 在进行RS232/RS485等直接的串口通信时，通常我们不使用Modem(在本地模式下)，所以必须设置这个标志位，否则可能会因为检测不到Modem信号而无法正常通信。
	//CREAD: (Enable Receiver) 开启接收器。设置这个标志位后，串口才能接收数据(处理从接收线(RXD)上进来的数据)。

	//3.设置最基本的控制标志 (c_cflag)
	newtio.c_cflag |= CLOCAL | CREAD;//确保串口工作在“本地模式”（忽略Modem状态）并开启数据接收功能。
	/*假设c_cflag是一个总控制面板，有很多开关，每个开关控制串口的一个特性。
	例如开关1: CLOCAL (本地模式)开关2: CREAD (开启接收)开关3: CS8 (8位数据位)开关4: PARENB (开启校验)开关5: CSTOPB (2位停止位)
	任务是打开“本地模式”(CLOCAL)和“开启接收”(CREAD)这两个开关，同时不能影响其他任何开关的状态。按位或符合要求*/

	//4.清除数据位设置，为后面的重新设置做准备
	newtio.c_cflag &= ~CSIZE;//清除数据位的设置，准备设置新的数据位。其他为置保持不变。

	//5.根据传入的参数 nBits，选择一个新的数据位设置
	switch (nBits)
	{
	case 7:
		newtio.c_cflag |= CS7;//设置数据位为7位
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	//6.根据传入的参数设置奇偶校验 (nEvent)
	switch (nEvent)
	{
	//设置奇效验
	case 'O':
		newtio.c_cflag |= PARENB;//开启奇偶校验功能
		newtio.c_cflag |= PARODD;//2.设置为奇校验
		newtio.c_iflag |= (INPCK | ISTRIP);
		/*INPCK: 告诉内核要对接收到的每一个字节进行奇偶校验检查。
		ISTRIP: (Strip Bit) 告诉内核将接收到的数据的第8位（即校验位）剥离掉，这样程序 read() 到的就是纯净的7位数据。*/
		break;
	//设置偶校验
	case 'E':
		
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;//设置偶校验
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	//无校验
	case 'N':
		newtio.c_cflag &= ~PARENB;//关闭奇偶校验功能
		break;
	}
	//奇偶校验的不足之处:如果同时有两个、四个、六个...（偶数个）位发生错误，它就完全检测不出来了。
	
	//7. 根据传入的参数设置波特率 (nSpeed)
	switch (nSpeed)
	{
	case 2400:
		cfsetispeed(&newtio, B2400);//输入波特率
		cfsetospeed(&newtio, B2400);//输出波特率
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

	//8. 根据传入的参数设置停止位 (nStop)
	if (nStop == 1)
		newtio.c_cflag &= ~CSTOPB;//清除（关闭）CSTOPB 标志，从而将串口设置为使用1个停止位。
	else if (nStop == 2)
		newtio.c_cflag |= CSTOPB;//开启CSTOPB 标志，从而将串口设置为使用2个停止位。

	//9. 设置read函数的超时和最小接收字符
	//c_cc 是 termios 结构体中的一个数组，包含了各种控制字符的设置。
	newtio.c_cc[VTIME] = 10;//设置超时时间为1秒（10 * 100毫秒），即在读取数据时，如果在1秒内没有接收到数据，则 read() 函数会返回。
	newtio.c_cc[VMIN] = 0;//设置最小读取字符数为0，表示 read() 函数不会阻塞等待数据到来，而是立即返回。

	//===应用阶段===

	//10.清空串口的输入/输出缓冲区
	tcflush(fd, TCIFLUSH);//清空输入缓冲区中的数据。
	//TCIFLUSH：表示清空输入缓冲区中的数据，但保留输出缓冲区中的数据。

	//11.将新配置应用到串口
	//TCSANOW：表示立即应用新的设置，而不等待数据传输完成。
	//tcsetattr(fd, TCSANOW, &newtio)：将配置好的newtio结构体中的所有新设置应用到指定串口(fd)上。
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}

	//12.成功返回
	printf("set done!\n");
	return 0;
//拓展：一般串口完整数据帧的定义：：帧头（2字节，例如AA、BB） + ?数据长度（2字节） + 数据 + CRC16校验（2字节） ?+ ?帧尾（2字节）
	//[帧头][数据长度][数据内容][CRC校验][帧尾]
	//帧头：通常是一个固定的字节，用于标识数据帧的开始。
	//数据长度：表示数据内容的长度，通常是一个字节。
	//数据内容：实际传输的数据，可以是任意长度。
	//CRC校验：用于检测数据传输中的错误，通常是一个字节。
	//帧尾：通常是一个固定的字节，用于标识数据帧的结束。
}

//用途：简化打开指定串口的操作;接收一个代表串口号的整数 comport（例如0, 1, 2等），然后返回一个代表该串口的数字“代号”（即文件描述符）
int open_port(int fd, int comport)
{
	//定义一个字符数组，存储不同串口的设备文件路径。每个串口对应一个设备文件，例如 /dev/ttyCH9344USB0, /dev/ttyCH9344USB1 等。
	char* dev[] = { "/dev/ttyCH9344USB0","/dev/ttyCH9344USB1","/dev/ttyCH9344USB2","/dev/ttyCH9344USB3","/dev/ttyCH9344USB4","/dev/ttyCH9344USB5", "/dev/ttyCH9344USB6","/dev/ttyCH9344USB7" };
	fd = open(dev[comport], O_RDWR | O_NOCTTY | O_NDELAY);
	//O_NOCTTY：这个标志可以防止打开的串口设备成为这个程序的“控制终端”。对于服务端或后台应用来说，这是一个标准的安全措施。
	//O_NDELAY：这个标志使得打开串口时不会阻塞。如果串口设备当前不可用，open() 函数会立即返回，而不是等待设备变得可用。

	if (-1 == fd) {
		perror("Can't Open Serial Port");
		return(-1);
	}
	else
		printf("open %s .....\n", dev[comport]);

	//fcntl是一个可以改变已打开文件(这里是串口)属性的函数。
	//fcntl(fd, F_SETFL, 0)：将文件描述符 fd 的状态标志设置为 0，通常用于清除非阻塞模式。
	//目的：确保串口在打开后处于阻塞模式，这样 read() 和 write() 调用会等待数据的到来或发送完成。
	if (fcntl(fd, F_SETFL, 0) < 0)
		printf("fcntl failed!\n");
	else
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));//fcntl=0 阻塞模式

	//isatty(fd)：检查文件描述符 fd 是否指向一个终端设备。
	//如果返回值为 0，表示 fd 不是一个终端设备；如果返回值为非零，表示 fd 是一个终端设备。
	if (isatty(STDIN_FILENO) == 0)
		printf("standard input is not a terminal device\n");
	else
		printf("isatty success!\n");

	printf("fd-open=%d\n", fd);
	return fd;

}

//用途:函数定义，接收一个参数：要清空的串口的“代号”（文件描述符）
//通过调用tcflush来确保关键时间点(如开始测试前)串口的数据通道是干净的
int clear_serial_buffer(int fd) 
{
	if (tcflush(fd, TCIOFLUSH) != 0) // TCIOFLUSH：表示清空输入和输出缓冲区中的数据。
	//tcflush() 函数 "Terminal Control Flush"（终端控制刷新）的缩写。
	//作用:丢弃指定终端(终端)中尚未处理的输入和输出数据。
	{
		perror("tcflush");
		close(fd);
		return -1;
	}
	return 0;
}

//核心目标:获取当前系统的精确时间,并格式化成一个包含毫秒,人类可读的字符串。
//char* pTimBuf：一个指向字符缓冲区的指针。
void get_cur_system_time(char* pTimBuf, int len)
{
	struct timeval tv;//用来存放时间戳
	struct tm tm_info;
	char timbuffer[64] = "";//字符串缓冲区，用来存放格式化好的时间字符串的第一部分（即 YYYY-MM-DD HH:MM:SS）
	gettimeofday(&tv, NULL);//获取当前系统的精确到微秒的时间，填充到tv结构体;第二个参数NULL:用于时区信息
	localtime_r(&tv.tv_sec, &tm_info);//将原始的秒数时间戳转换成人可读的类型
	strftime(timbuffer, len, "%Y-%m-%d %H:%M:%S", &tm_info);//strftime:格式化时间为字符串;
	sprintf(pTimBuf, "%s:%03ld", timbuffer, tv.tv_usec / 1000);
	//sprintf():格式化函数，可以将各种变量"打印"到一个字符串  
	//pTimeBuf:指定最终输出的目标缓冲区,也就是用户传入的那个缓冲区。
	//tv.tv_usec / 1000就是毫秒;组合起来就是"2025-07-25 22:40:15"
}

//作用:创建一个结构化,内容完整的日志文件路径
//参数：一个用于存放结果的字符缓冲区(pNameBuf)，缓冲区的长度(len)用于保证安全，以及测试的编号(no)。
void get_cur_system_time_filename(char* pNameBuf, int len, int no)
{
	struct timeval tv;//秒和微秒
	struct tm tm_info;//年月日时分秒
	char timbuffer[64] = "";// 缓冲区，用于存放格式化后的时间字符串 (例如 "2025-07-24_10-30-55")。
	char datebuffer[64] = "";// 缓冲区，用于存放仅包含日期的格式化字符串 (例如 "2025-07-24")。
	//2.获取当前时间
	gettimeofday(&tv, NULL);
	//3.将当前时间转换为本地时间，并存储在 tm_info 中。
	localtime_r(&tv.tv_sec, &tm_info);
	//4.将 tm_info 中的时间格式化为字符串，存储在 timbuffer 中。(即 年-月-日_时-分-秒)
	strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
	//5.将日期部分格式化为字符串，存储在 datebuffer 中。(即 年-月-日)
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);
	//6.将格式化后的时间和日期信息组合成一个完整的日志文件路径，并存储在 pNameBuf 中。
	//sprintf() 函数将格式化的字符串写入 pNameBuf 中，
	sprintf(pNameBuf, "./serial_log/%s/serial%d_tx_%s.log", datebuffer, no, timbuffer);
}

//用途：获取当前的系统日期，并生成一个用于存放当天所有日志的目录路径字符串。
void get_cur_system_date_dirname(char* pNameBuf, int len)
{
	struct timeval tv;
	struct tm tm_info;
	char datebuffer[64] = "";//用于存放格式化后的日期字符串，例如 "2025-07-24"。
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_info);//转换时间格式
	strftime(datebuffer, len, "%Y-%m-%d", &tm_info);//格式化日期字符串
	sprintf(pNameBuf, "./serial_log/%s", datebuffer);
}

//解析命令行参数
int main(int argc, char* argv[])
{
	int fd = -1;// 用于存放文件描述符，看作是打开的串口的“代号”。 初始化-1表示未成功打开任何设备。
	int nread = 0;//记录通过 read() 函数成功读取到的字节数
	int nwrite = 0;//记录通过 write() 函数成功写入的字节数
	int sndframeCnt = 0;//发送帧计数
	int rcvframeCnt = 0;//接收帧计数
	int rt = -1;// 用于存放函数返回值，通常用于检查函数调用是否成功。
	int i = 0;
	int badCrcCnt = 0;// 记录接收数据时 CRC 校验失败的次数
	uint8_t crc = 0x00;// 用于存放计算得到的 CRC 校验值。 0x00 是一个初始值，表示 CRC 校验的起始状态。
	char buffer[MAX_SND_BUF_LEN];//字符数组缓冲区。预留的内存空间，临时存放准备发送或刚刚接收到的数据。
								// MAX_SND_BUF_LEN 是一个预先定义好的常量，代表这个缓冲区的最大长度256。
	char logfilename[64] = "";//存储完整的日志文件名 (包括路径)。
	char logdirname[64] = "";//存储日志目录的名称。
	char timbuffer[64] = "";//存储格式化后的时间字符串。
	/*GPIO控制命令数组 2行8列
		第0行 (sysgpiocmd[0]): 存放了8条关闭LED灯的命令。
		第1行 (sysgpiocmd[1]): 存放了8条点亮LED灯的命令。*/
	char* sysgpiocmd[2][8] = {
	{"echo 0 > /sys/class/gpio/gpio200/value","echo 0 > /sys/class/gpio/gpio201/value","echo 0 > /sys/class/gpio/gpio202/value","echo 0 > /sys/class/gpio/gpio204/value","echo 0 > /sys/class/gpio/gpio11/value","echo 0 > /sys/class/gpio/gpio12/value","echo 0 > /sys/class/gpio/gpio16/value","echo 0 > /sys/class/gpio/gpio17/value"},
	{"echo 1 > /sys/class/gpio/gpio200/value","echo 1 > /sys/class/gpio/gpio201/value","echo 1 > /sys/class/gpio/gpio202/value","echo 1 > /sys/class/gpio/gpio204/value","echo 1 > /sys/class/gpio/gpio11/value","echo 1 > /sys/class/gpio/gpio12/value","echo 1 > /sys/class/gpio/gpio16/value","echo 1 > /sys/class/gpio/gpio17/value" }
	};
	FILE* log_file = NULL;//声明一个文件指针，用于后续打开的日志文件
	int rs485no = atoi(argv[1]);//将命令行参数转换为整数，表示串口编号。 例如，如果命令行输入是 "./rs485_test_snd 0"，则 rs485no 将被赋值为 0。
	if ((fd = open_port(fd, rs485no)) < 0)//根据串口编号打开对应的串口设备。
	{
		perror("open_port error");
		return -1;
	}

	//波特率设置为115200，数据位8位，奇偶校验位N（无校验），停止位1位。 返回值0:成功 -1:失败
	if ((rt = set_opt(fd, 115200, 8, 'N', 1)) < 0)
	{
		perror("set_opt error");
		return -1;
	}
	printf("fd=%d\n", fd);

	init_crc8_table();//生成并初始化一张CRC8的查找表

	//完整路径的，最终的日志文件名字。日志文件名的格式为 "./serial_log/2025-07-24/serial0_tx_2025-07-24_10-30-55.log"。
	get_cur_system_time_filename(logfilename, sizeof(logfilename), rs485no);
	//完整的日志目录名(确保同一天产生的日志归类到同一个文件夹下)，格式为 "./serial_log/2025-07-24"。
	get_cur_system_date_dirname(logdirname, sizeof(logdirname));


	//逻辑：如果日志目录不存在，则创建它。
	//access() 函数:一个系统调用,用于检查程序是否拥有对指定文件或目录的某种访问权限
	//F_OK: 检查文件其存在性。
	if (access(logdirname, F_OK) != 0)
	{
		mkdir(logdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	//打开日志文件，追加模式（"a"），如果文件不存在则创建它。
	log_file = fopen(logfilename, "a");


	//流程：“发送数据 -> 等待回声 -> 处理结果”的整个闭环测试流程。
	//发送阶段：精心构造一个包含特定数据模式和校验码的数据包，并通过串口发送出去。
	//接收阶段：短暂等待后，尝试从同一个串口读取由对方设备回传的数据（“回声”）。
	//处理与诊断阶段：根据接收到的结果，进行细致的分类处理。
	// 它需要判断并处理所有可能发生的情况，包括成功、超时和数据损坏。
	while (1)
	{
		//====一.数据包的构造与发送====
		//1.填充递增序号
		for (i = 0; i < 32;i++)
		{
			buffer[i] = i;
		}
		//2.填充第一种测试0x5a
		for (i = 32; i < 40;i++)
		{
			buffer[i] = 0x5a;
			buffer[i + 16] = 0x5a;
		}
		//填充第二种测试0xa5
		for (i = 40; i < 48;i++)
		{
			buffer[i] = 0xa5;
			buffer[i + 16] = 0xa5;
		}
		
		//3.计算并附加CRC8校验码
		//对除最后一个字节外的所有数据进行计算，并将结果存入最后一个
		//RCV_BUF_LEN长度是64，buffer[0]到buffer[62]这63个字节是有效数据，buffer[63]是CRC8校验码
		//参数：buffer的起始地址和数据长度63  crc8函数会对这个63个字节的有效数据进程CRC计算，函数返回一个8位的CRC校验码，赋值给buffer[63]
		buffer[RCV_BUF_LEN - 1] = crc8(buffer, RCV_BUF_LEN - 1);
		//4.通过串口发送数据包
		nwrite = write(fd, buffer, RCV_BUF_LEN);//将buffer内存中的从头开始的RCV_BUF_LEN个字节，通过fd这个串口发送出去

		//5.记录发送日志
		get_cur_system_time(timbuffer, sizeof(timbuffer));//获取当前的最新的精确到毫秒级的时间字符串，存放到timbuffer中。
		printf("[%s]sndframeCnt = %d nwrite = %d\n", timbuffer, sndframeCnt++, nwrite);
		//sndframeCnt:定义的一个发送帧的数量,初始化为0   //nwrite:write的返回值，代表实际成功写入的字节数。
		//执行完的日志格式：[2025-07-24 22:50:30:123]sndframeCnt = 15 nwrite = 10


		//====二.准备接收与等待回声=====
		// 接收服务器回显的数据
		//1.清空接收缓冲区,防止旧数据干扰
		for (i = 0; i < RCV_BUF_LEN;i++)
		{
			buffer[i] = 0;//因为在一个循环当中,每次需要buffer来构建一个新的数据包;因此每次发送完一个数据帧后需要清空
		}
		//2.让当前程序停留0.1秒;用途：给另一端设备留出响应时间，在0.1秒内把响应数据发过来
		usleep(100000);//参数单位微秒
		//3.尝试读取数据
		nread = read(fd, buffer, RCV_BUF_LEN);//等待0.1秒，从fd这个串口设备读取数据，数据存储到buffer,返回值是实际读取的字节数
		//buffer:缓冲区，read函数读取的数据存放到这个buffer里;RCV_BUF_LEN是这个缓冲区的最大的容量

		//=====三.结果处理与错误诊断=====
		//情况:通信超时
		if (nread == 0)
		{
			//打印并记录超时日志，cnt记录了当前已发送但未成功接收的数据帧数
			printf("[%s]No data on serial%d cnt = %d\n", timbuffer, rs485no, sndframeCnt - rcvframeCnt);
			//timebuffer:存储格式化后的时间字符串(包含了当前精确时间戳)
			//sndframeCnt - rcvframeCnt:发送帧计数 - 接收帧计数
			//向指定文件logfile写入数据
			fprintf(log_file, "[%s]No data on serial%d cnt = %d\n", timbuffer, rs485no, sndframeCnt- rcvframeCnt);
			//详细信息：发送帧 接收帧 nwrite:上一次实际写入字节数   nread:read()操作实际读到的字节数
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);//实时性; 强制将缓冲区中所有未写入的数据,立刻,马上写入到磁盘文件中。
			//保证数据持久化
			fsync(fileno(log_file));
			//fileno(log_file):这个函数的作用是将FILE*类型的文件指针log_file转换成操作系统内核识别的整数 文件描述符(fd).
			//fflush只保证数据从程序的内存缓冲区写入到操作系统的内核缓冲区，但数据可能仍然在操作系统的缓存里，还没真正写入到物理硬盘上。
			//fsync 则会请求操作系统将内核缓冲区的数据也立刻写入到物理存储设备（如硬盘）中。提供最高级别的数据持久化保证。
			continue;//跳出循环
		}
		
		//获取当前系统的精确时间
		get_cur_system_time(timbuffer, sizeof(timbuffer));


		//数据接收后的CRC校验与错误处理模块
		crc = crc8(buffer, RCV_BUF_LEN - 1);//计算一下接收到的数据帧的CRC8 校验码
		//对比CRC
		if (crc != buffer[RCV_BUF_LEN - 1])
		{
			badCrcCnt++;//CRC校验失败计数

			//记录详细的CRC错误日志
			printf("[%s]recvfrom failed badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			//% x:表示以十六进制的形式打印一个整数  02:表示打印的宽度至少为2个字符，如果不足2个字符，在前面用0补齐.
			fprintf(log_file, "[%s]recvfrom failed badCrcCnt = %d crc = %02x nread = %d\n", timbuffer, badCrcCnt, crc, nread);
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);
			fsync(fileno(log_file));
			
			//===硬件复位/控制操作===
			//通过system()调用shell命令,快速闪烁对应的LED灯,提供视觉警报
			system(sysgpiocmd[1][rs485no]);
			usleep(5000);//5毫秒
			system(sysgpiocmd[0][rs485no]);

			//清空串口终端的输入输出缓冲区
			clear_serial_buffer(fd);//检测到线路存在严重干扰，缓冲区里可能残留这其他的"坏数据"，所以在下一次发送前将缓冲区彻底清空
			continue;
		}

		//通信成功
		rcvframeCnt++;//成功接收计数器加1
		//记录成功的日志
		printf("[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
		fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %d nread = %d\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
		fflush(log_file);//实时性; 强制将缓冲区中所有未写入的数据,立刻,马上写入到磁盘文件中。
		fsync(fileno(log_file));//fsync 则会请求操作系统将内核缓冲区的数据也立刻写入到物理存储设备（如硬盘）中

		//数据抽样打印
		//头部16字节和尾部16字节 确认序列号是否正确,数据格式是否复合预期，而无需查看完整的,冗长的数据包。
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
	}

	fclose(log_file);//关闭日志文件
	close(fd);//关闭串口
	return 0;
}

/*
小结：
好的，我们来详细讲解 `rs485_test_snd` 和 `rs485_test_rcv` 这两个程序之间的业务逻辑。它们是专门为串口通信测试设计的一对“搭档”，共同完成一次严谨的闭环测试。

您可以把它们想象成一个**“问答”**游戏，目的是为了测试在有“噪音”（电磁干扰）的情况下，对话双方是否还能清晰、准确地沟通。

---

### **`rs485_test_snd` 的业务逻辑：一个严格的“提问者”和“评分员”**

`rs485_test_snd` 在这个测试中扮演**“主角”**，是主动发起通信的一方。它的业务逻辑可以概括为：**“不断地提出一个带有编号的问题，并严格地检查对方的回答是否正确、及时。”**

#### 它的工作流程是：

1.  **“准备问题” (初始化)**
    * 启动时，你告诉它要通过哪个串口“喊话”。
    * 它会准备好日志本，用于记录每一次问答的结果。
    * 它会准备好一套“红牌警告”工具（LED灯的控制命令）。
    * 最重要的是，它会提前学习并制作好一本“CRC校验码速查表”，以便快速检查答案是否标准。

2.  **“提问 -> 等待回答 -> 评分” (主循环)**
    * 它会进入一个无限循环，周而复始地执行以下动作：
        * **提问 (Send)**：精心构造一个**结构化的数据包**。这个包里不仅有“问题内容”（特殊的`0x5a`, `0xa5`图案），还有一个独一无二的**“问题编号”**（递增的序列号），最后还贴上了一个根据内容算出来的**“防伪标签”**（CRC8校验码）。然后，它通过 `write()` 函数把这个问题“喊”出去，并在日志里记下：“我问了第100个问题。”
        * **等待回答 (Wait & Read)**：喊出问题后，它会短暂地 `usleep` (暂停) 一下，给对方留出思考和回答的时间。然后，它会调用 `read()` 函数，开始“竖起耳朵听”对方的回答。
        * **评分 (Verify)**：听完回答后，它会进行严格的评分，有三种结果：
            * **情况A：对方没回答 (超时)**：如果 `read()` 在指定时间内什么也没听到，它会判定“对方没回答”。它会在日志里记下：“第101个问题超时了！”，然后立即 `continue`，接着问下一个问题。
            * **情况B：回答得乱七八糟 (CRC校验失败)**：如果收到了回答，但回答的“防伪标签”（CRC码）和内容对不上，它会判定“回答错误！”。它会在日志里记下：“第102个问题回答错误！”，然后**闪烁一下LED灯**（亮红牌警告），并主动清空一下通信线路（`clear_serial_buffer`），最后 `continue`，接着问下一个问题。
            * **情况C：回答得完全正确**：如果收到了回答，并且CRC校验完全正确，它会判定“回答正确！”。它会在日志里记下：“第103个问题回答正确！”，并在屏幕上展示一下对方回答的样本。

**`rs485_test_snd` 的核心特点**：
* **主动**：是它发起了整个通信过程。
* **有状态**：它有各种计数器，精确记录了问了多少问题、成功了多少、失败了多少。
* **是裁判**：测试的**通过/失败标准**（超时、CRC错误）是由它来定义的和判断的。

---

### **`rs485_test_rcv` 的业务逻辑：一个忠实的“应答者”**

`rs485_test_rcv` 在这个测试中扮演一个**简单、被动**的角色。它的业务逻辑可以概括为：**“一直听着，只要听到一个格式正确的问题，就把它原封不动地复述一遍作为回答。”**

#### 它的工作流程是：

1.  **“坐好准备” (初始化)**
    * 启动时，你告诉它要用哪个串口“听讲”。
    * 它会以一种特殊的**“不收满64字节不罢休”**的模式配置好串口。
    * 它也会准备好CRC校验工具和日志本。

2.  **“无限聆听” (主循环)**
    * 它会进入一个无限循环，调用 `read()` 函数，然后就**一直、耐心地等待**。程序会“卡”在这里，直到接收到**一个完整的、64字节长**的数据包。

3.  **“检查并复述” (校验与回声)**
    * 一旦收到了一个完整的数据包，它会先做一个**“内部质检”**（CRC校验）。
    * **如果校验失败**：它会认为听到了“噪音”，而不是一个真正的问题。它会在自己的日志里记一笔“听到了一个坏包”，然后**把这个坏包丢掉**，并清空自己的耳朵（`clear_serial_buffer`），接着回到第二步，继续聆听。**注意：它不会回答坏包。**
    * **如果校验成功**：它确认自己听到了一个**完好的、有效的问题**。于是，它会立刻调用 `write()` 函数，把这个**刚刚收到的、一模一样的、未经任何修改的数据包**，再原封不动地“复述”回去，作为“回答”。

**`rs485_test_rcv` 的核心特点**：
* **被动**：它从不主动说话，只在听到正确问题后才回答。
* **严格**：它只接收规定长度（64字节）的数据包，并且只回答校验正确的包。
* **是镜子**：它的回答就是问题的原文，忠实地反射收到的内容。

### **总结：两者如何协同工作**

`rs485_test_snd` 和 `rs485_test_rcv` 共同构成了一个严谨的**串口通信质量检测闭环**：

1.  `snd` (提问者) 发送一个带编号和防伪标签（CRC）的**问题包**。
2.  `rcv` (应答者) 接收这个包，检查防伪标签。如果标签无误，就将**原包**作为**回答包**再发回去。
3.  `snd` (提问者) 等待这个回答包。收到后，也检查其防伪标签。

通过观察 `snd` 端的日志和LED灯，测试人员就能清晰地知道在这场持续的“问答”中，出现了多少次“对方没回答”（超时/丢包）和多少次“回答得乱七八糟”（数据损坏）。
*/