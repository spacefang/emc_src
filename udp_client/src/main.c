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

//udp_client 在192.168.1.10:9092 上启动
int main(int argc, char* argv[])
{   
	int sockfd;//创建的套接字的“代号”，是后续所有网络操作的核心句柄。
    struct sockaddr_in servaddr, cliaddr;// 用于存放服务器(servaddr)和客户端(cliaddr)的地址信息（IP地址、端口号等）的结构体。
    socklen_t len;//用于存放地址结构体(如servaddr或cliaddr)的字节大小，是recvfrom和sendto函数必需的参数。
    char buffer[BUFFER_SIZE];//接收和发送数据的缓冲区，大小为1024字节。它及用于构造准备发送给服务器的数据包，也用于存放从服务器接收到的回声数据
    char timbuffer[64] = "";//用于存放格式化后的时间字符串，用于日志记录。
    char logfilename[64] = "";// 用于存放完整的日志文件名。
	char logdirname[64] = "";//用来存放日志文件所在的目录名
    ssize_t nread;// 存放recvfrom函数实际读取到的字节数。
    ssize_t nwrite;// 存放sendto函数实际写入的字节数。
	int sndframeCnt = 0;//记录总共发送了多少个数据包
	int rcvframeCnt = 0;//记录总共成功接收了了多少个有效的数据包
    // int frameCnt = 0;// 帧计数器，用于统计总共接收了多少个数据包。
    int i = 0;// 通用的循环计数器变量。
    int ipfrg[4] = { 0,0,0,0 };// 用于存放从字符串中解析出的IP地址的四个部分。
	char* sysgpiocmd[2][4] = { //第一行关闭4个不同的LED灯命令，第二行点亮4个LED灯的命令。
		{"echo 0 > /sys/class/gpio/gpio206/value","echo 0 > /sys/class/gpio/gpio207/value","echo 0 > /sys/class/gpio/gpio208/value","echo 0 > /sys/class/gpio/gpio209/value"},
		{"echo 1 > /sys/class/gpio/gpio206/value","echo 1 > /sys/class/gpio/gpio207/value","echo 1 > /sys/class/gpio/gpio208/value","echo 1 > /sys/class/gpio/gpio209/value" }
	};
    FILE* log_file = NULL;// 指向日志文件的文件指针。

	if (argc != 5)//程序名本身;自己的IP和端口;目标服务器的IP和段码
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
	cliaddr.sin_port = htons(atoi(argv[2]));//atoi(): 将端口号字符串转换为整数。htons(): 将端口号从主机字节序转换为网络字节序
	if (inet_pton(AF_INET, argv[1], &cliaddr.sin_addr) <= 0)
	//inet_pton(): 将IP地址字符串（如 "192.168.3.10"）转换为网络二进制格式
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

	//注意:上面先初始化了client客户端，后面又初始化了server服务端
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

	//初始化发送缓冲区
	//目的:准备一个内容确定的,非空的数据包，用于发送给服务器。
	for (i = 0; i < BUFFER_SIZE;i++)//遍历buffer这个大小为1024字节的数组 
	{
		buffer[i] = i;
		//因为char类型是一个8位的数据类型，只能表示256个不同的值.
		//执行后的结果：buffer[0]=0, buffer[1]=1, buffer[2]=2, ..., buffer[255]=255, buffer[256]=0 
		// (因为 char 类型通常是8位，超过255会回绕), buffer[257]=1, ... 依此类推。
		//255时就是 1111 1111 256就是 1 0000 0000  char类型只有8位的存储空间
	}
	/* 设置超时*/
	/*为套接字的接收操作设置了一个超时时间。如果没有设置，后续在主循环中调用的 recvfrom 函数将会是永久阻塞的。
	如果服务器因为任何原因（如宕机、网络不通）没有回发数据，客户端程序就会永远地卡在 recvfrom 那里等待，变成一个“僵死”的进程。*/
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 300000;//300000微秒=0.3秒
	//上述两行代码将timeout变量设置成0.3秒

	//setsockopt()函数,用于设置套接字的各种选项(options)。 在后续调用recvfrom函数时会产生作用。
	/*参数设置:sockfd要对哪个套接字进行设置 ; SOL_SOCKET: 表明要设置的选项属于“套接字层面”(Socket Level)。不属于TCP或IP的协议
		SO_RCVTIMEO: 关键的部分，表明要设置的选项是“接收超时” (Receive Timeout)。
		&timeout: 指向配置好的 timeout 结构体的指针，将定义的0.3秒超时时间传递给setsockopt函数。函数会从这个地址读取时间值，并应用到套接字上。*/
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
	{
		//错误处理:如果设置选项失败,程序就会通过perror打印错误信息，但不会退出。即使设置超时失败，程序仍会尝试继续运行
		perror("setsockopt failed:");
	}

	//解析客户端自身的IP地址，并利用这个IP地址的一部分，结合当前时间，创建出一套有组织的、唯一的日志文件和目录名。
	sscanf(argv[1], "%d.%d.%d.%d", &ipfrg[0], &ipfrg[1], &ipfrg[2], &ipfrg[3]);//将一个IP地址字符串拆分成四个独立的整数
	printf("ip=%d.%d.%d.%d\n", ipfrg[0], ipfrg[1], ipfrg[2], ipfrg[3]);//这是一个调试确认步骤。
	get_cur_system_time_filename(logfilename, sizeof(logfilename), ipfrg[2]);//生成一个唯一的、包含完整路径的日志文件名。
	//执行效果:假设 ipfrg[2] 的值是 3，这个函数会生成一个类似 ". /eth_log/2025-07-28/eth3_tx_2025-07-28_05-10-00.log" 的字符串，并存入 logfilename 变量中。
	get_cur_system_date_dirname(logdirname, sizeof(logdirname));//生成日志文件所在的目录路径。
	//执行效果:logdirname 变量中会存入类似 ". /eth_log/2025-07-28" 的字符串。这个变量后续会用于 mkdir（创建目录）操作。

	//access()函数：用于检测程序对指定的文件或目录是否拥有访问权限
	if (access(logdirname, F_OK) != 0)//F_OK 检查模式是只检查“存在性”; 不关心目录是否可读可写
	{
		mkdir(logdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);//mkdir创建一个新的目录
		//文件权限设置;user 读写执行   group 读写执行  other 读和执行权限
	}

	log_file = fopen(logfilename, "a");

	while (1)
	{
		// 向服务器发送数据

		for (i = 0; i < BUFFER_SIZE;i++)
		{
			buffer[i] = i;
			//这个循环将1024字节的buffer填充为一个递增的数字序列（0, 1, 2, ..., 255, 0, 1, ...）。
			// 确保每次发送的数据内容都是已知和固定的，便于后续（如果需要的话）进行内容比对。
		}
		// buffer[BUFFER_SIZE - 1] = crc8(BUFFER_SIZE, RCV_BUF_LEN - 1);


		//UDP客户端程序的核心，作用是将一个UDP数据包通过网络发送给一个特定的服务器。它收集了所有必要信息--要发送什么，发送多少，以及发送到哪里，然后将这些信息交给操作系统的网络协议栈进行传输 
		nwrite = sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		//sendto主动发送UDP数据的系统调用 buffer:指定要发送的数据 BUFFER_SIZE:发送的字节数 0:默认的发送行为
		//关键参数:(struct sockaddr*)&servaddr:目标地址。程序将数据发送到之前配置好的、包含了服务器IP和端口的servaddr地址。
		//返回值：成功，实际发送的字节数; 失败,返回-1
		if (nwrite == -1)
		{
			perror("sendto failed\n");
			continue;
		}

		//以下三行代码:获取时间戳 打印日志 短暂延时
		get_cur_system_time(timbuffer, sizeof(timbuffer));//获取一个格式化好的,精确到毫秒的时间字符串。
		//timbuffer数组里就存放了调用printf之前那一刻的最新时间字符串
		//printf("send to server  (%s:%d): nwrite = %ld\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), nwrite);
		printf("[%s]sndframeCnt = %d nwrite = %ld\n", timbuffer, sndframeCnt++, nwrite);
		//将多个不同类型的变量组合成一条日志信息，并打印到控制台。  sndframeCnt是一个发送帧计数器;依次打印出sndframeCnt=0 ; 1 ; 2...
		usleep(200000);//暂停0.2秒 200000微秒=200毫秒=0.2秒

		// 接收服务器回显的数据  准备接收->执行接收->记录时间
		//1.清空接收缓冲区
		for (i = 0; i < BUFFER_SIZE;i++)
		{
			buffer[i] = 0;
		}

	    len = sizeof(servaddr);//告诉函数servaddr指针指向的内存有多大;函数返回时，会被修改为实际的源地址结构体的大小
		nread = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, &len);//阻塞式等待网络上的UDP数据包的到达
		//并且将发送方的地址信息(IP地址和端口号)填充到这个结构体中。返回值nread>0 实际成功接收到的字节数。
		get_cur_system_time(timbuffer, sizeof(timbuffer));//记录接收时间

		//recvfrom函数调用失败,程序会等待0.3秒
		if (nread == -1)
		{
			printf("[%s]recvfrom failed badcnt = %d\n", timbuffer, sndframeCnt- rcvframeCnt);//已发送帧和已接受帧的差值
			fprintf(log_file, "[%s]recvfrom failed badcnt = %d\n", timbuffer, sndframeCnt - rcvframeCnt);
			fprintf(log_file, "[%s]sndframeCnt = %d rcvframeCnt = %d nwrite = %ld nread = %ld\n", timbuffer, sndframeCnt, rcvframeCnt, nwrite, nread);
			fflush(log_file);//将I/O缓冲区的日志内容立即写入到操作系统的内核缓冲区。
			fsync(fileno(log_file));//请求将内核缓冲区中的内容立即写入到物理硬盘中
			system(sysgpiocmd[1][ipfrg[2]-1]);
			//举例：程序的启动命令是./udp_client 192.168.1.10 ipfrg[2]=1 1-1=0 ,发生接收失败时，system(sysgpiocmd[1][0]) 和 system(sysgpiocmd[0][0])
			usleep(5000);
			system(sysgpiocmd[0][ipfrg[2]-1]);
			continue;
		}

		//printf("recv from client  (%s:%d): nread = %ld \n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), nread);
		
		//调试和可视化显示
		printf("[%s]rcvframeCnt = %d nread = %ld\n", timbuffer, rcvframeCnt++, nread);//接收到的数据包的确切字节大小
		//循环打印数据的前16个字节
		for (i = 0;i < 16;i++)
		{
			printf("%02x ", buffer[i]);
		}

		printf("...");
		//后16个字节
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

//UDP网络回环测试(udp_client & udp_server)
/*
业务流程:
udp_server 程序先在一台设备上（我们称之为B设备）启动，它会监听一个指定的IP地址和端口（例如 192.168.3.20:9092）。
udp_client 程序在另一台设备上（A设备）启动，它会进入一个无限循环。
在循环中，udp_client 首先准备一个内容固定（0, 1, 2, 3...）的1024字节数据包，然后通过 sendto 函数将其发送给B设备的IP和端口。
B设备的 udp_server 收到数据包后，不做任何修改，立即通过 sendto 函数将这个原样的数据包发回给A设备。这被称为“回声（Echo）”。
A设备的 udp_client 在发送数据后，会调用 recvfrom 等待接收B设备的回声数据。为了防止永久等待，它巧妙地设置了0.3秒的接收超时。
udp_client 接收到数据后，业务逻辑隐含着对数据的验证（虽然代码里没有写明具体的比较，但打印日志的行为本身就是在验证收发是否成功）。


健壮性设计 (关键业务逻辑):
超时处理：如果在0.3秒内没有收到回声数据，udp_client 会认为发生了一次通信失败。
硬件自愈：在发生超时失败后，程序会执行最关键的硬件复位操作。它会通过 system() 函数调用shell命令来操作GPIO引脚
		（例如，拉高GPIO206电平，等待5毫秒，再拉低），这很可能是在硬复位网络芯片或相关的硬件模块，试图从硬件异常中自动恢复。
动态硬件适配：程序会根据自己本地IP地址的第三部分（例如 192.168.3.10 中的 3）来动态选择控制哪一个GPIO引脚。
			这使得同一份代码可以部署在IP地址为 192.168.1.x, 192.168.2.x, 192.168.3.x 的不同硬件上，自动适配并控制相应的硬件，非常灵活。
日志记录：每一次发送、接收成功、接收超时都会被精确到毫秒的时间戳记录下来，并保存到日志文件中。日志文件名也由日期和IP地址动态生成，便于管理和追溯。
*/
