#include <sys/socket.h> //提供了最基础、最核心的套接字编程接口。包含socket()函数用于创建套接字，bind()用于绑定地址和端口;sendto(),recvfrom()用于UDP数据的收发
#include <netinet/in.h>//提供了专门用于互联网协议(IP协议)的定义。struct sockaddr_in结构体;htons()函数
#include <arpa/inet.h>//提供了IP地址格式转换的功能;inet_pton()函数，将点分十进制转换成网络字节序
#include <stdio.h>//标准输入和输出操作;例如sscanf()是从字符串中读取格式化数据。
#include <stdlib.h>//提供内存管理,进程控制,类型转换等通用功能。exit():立即终止程序。atoi():将字符串转换成整数。
#include <string.h>//提供操作字符串和内存块的函数。 memset():将一块内存区域全部设置位某个特定的值，常用于结构体的初始化。
#include <unistd.h>//提供了对POSIX操作系统API的访问,包括了很多底层的系统调用;close()关闭文件描述符 usleep():以微秒为单位暂停程序。

#include <sys/types.h>//基础的，系统层面的数据类型，比如size_t(用于表示大小)和size_t(表示可能为负的字节数)。
#include <sys/stat.h>//提供获取文件状态和设置文件模式(权限)的功能。mkdir() 函数使用的文件权限宏，如 S_IRWXU
#include <fcntl.h>//提供了对文件描述符进行控制的底层操作。
#include <time.h>
#include <sys/time.h>//这两个文件共同提供了处理日期和时间的功能。
#define BUFFER_SIZE 1024
//实现的业务逻辑:接收任何客户端发来的UDP数据包，然后原封不动将数据包再发回这个客户端
//验证连通性质:客户端可以通过能否收到回声,来判断与服务器之间的网络链路是否通畅。
//测试数据完整性：客户端可以对比自己发送的数据和收到的回声数据是否完全一致，从而判断数据在传输过程中是否因为干扰而出错。
//提供持续的网络负载：在EMC测试中，客户端可以持续不断地向这个服务器发送数据，服务器也会持续不断地回发，从而在网口上产生稳定的数据流，用以测试网口在电磁干扰下的稳定性。
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

//参数:一个指向字符串的常量指针（用于接收自己的名字）
void usage(const char* prog_name) 
{
    //用法是: 程序名 IP地址 端口号
    printf("Usage: %s <listen_ip> <listen_port>\n", prog_name);
    printf("Example: %s 192.168.3.20 9092\n", prog_name);
    exit(EXIT_FAILURE);//终止程序
}

//任务:获取当前系统的精确时间，并将其格式化成 一个包含毫秒的，人类可读的字符串。年月日时分秒毫秒
//pTimeBuf:一个字符缓冲区，用于存放最终生成的时间字符串。 len:pTimBuf缓冲区的长度,用于strftime的安全检查。
void get_cur_system_time(char* pTimBuf, int len)
{
    //1. 声明时间相关的变量
    struct timeval tv;  // 用于存放从系统获取的、包含秒和微秒(usec)的高精度时间。
    struct tm tm_info;  // 用于存放转换后的、分解为年/月/日等成员的日历时间。
    char timbuffer[64] = ""; //一个临时的本地缓冲区，用于存放不包括毫秒部分的标准时间字符串。
    //2.获取当前高精度时间
    gettimeofday(&tv, NULL);//gettimeofday函数会用当前的秒数和微秒数来填充tv结构体。
    //3.将秒数转换为日历时间
    localtime_r(&tv.tv_sec, &tm_info);// 将tv结构体中的秒数部分(tv.tv_sec)转换成tm_info结构体。
    //4.格式化年-月-日 时:分:秒 部分
    strftime(timbuffer, len, "%Y-%m-%d %H:%M:%S", &tm_info);
    //5.拼接最终的、包含毫秒的完整时间字符串 %03——表示打印的数字至少占3位;ld表示打印一个长整型
    sprintf(pTimBuf, "%s:%03ld", timbuffer, tv.tv_usec / 1000);
}

//任务:生成一个唯一的、带有明确标识的日志文件名，并包含完整的存储路径。
//参数:pNameBuf: 一个字符缓冲区，用于存放最终生成的文件名字符串。len: pNameBuf缓冲区的长度，用于安全检查。
//ethno: 一个整数，代表网络接口的编号（例如，从IP地址解析出的'3'）
void get_cur_system_time_filename(char* pNameBuf, int len, int ethno)
{
    //1.声明时间相关的变量
    struct timeval tv; // 用于存放从系统获取的、包含秒和微秒的高精度时间。
    struct tm tm_info;// 用于存放转换后的、分解为年/月/日等成员的日历时间。
    char timbuffer[64] = "";// 一个临时缓冲区，用于存放格式化后的时间戳字符串。

    //2.获取当前系统时间，填充到tv结构体中
    gettimeofday(&tv, NULL);
    //3.转换时间为可读格式,通过调用localtime_r函数，将tv中的秒数部分转换成tm_info结构体
    localtime_r(&tv.tv_sec, &tm_info);
    //4.将时间格式化为字符串，并存入timbuffer
    strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
    //5.将拼接好的完整路径被存入由调用者传入的pNameBuf中。
    sprintf(pNameBuf, "./eth_log/eth%d_rx_%s.log", ethno, timbuffer);
}

/*
1.初始化阶段 (程序启动后，进入循环前)
    参数检查：检查用户运行程序时，是否在命令行输入了正确的参数（IP地址和端口号）。
    创建网络通道：创建一个UDP套接字（Socket），这相当于在系统中开辟一个专门用于UDP通信的端口。
    绑定地址：将这个套接字与用户指定的IP地址和端口号绑定。这就像是给这个网络通道挂上一个明确的“门牌号”，让客户端知道该往哪里发送数据。
    准备日志系统：根据绑定的IP地址，创建一个唯一的、带时间戳的日志文件，用于记录后续的所有通信活动。

2.主服务循环阶段 (while(1) 无限循环)
    等待并接收数据：程序会阻塞在 recvfrom 函数上，耐心等待，直到有客户端的数据包到达。
    记录接收日志：一旦收到数据，立即记录下接收事件的时间、序号和数据长度。
    数据回声：调用 sendto 函数，将刚刚收到的数据包，原封不动地发送回给客户端。
    循环往复：完成一次回声后，程序立即回到循环的起点，继续等待下一个数据包的到来。*/

/*
涉及到的关键函数的用法
socket(AF_INET, SOCK_DGRAM, 0)
    用法：创建一个网络套接字。
    AF_INET：指定使用IPv4协议。
    SOCK_DGRAM：指定使用UDP协议（数据报套接字）。这是与TCP (SOCK_STREAM) 的关键区别。
    0：通常设为0，让系统自动选择协议。
    返回值：成功则返回一个非负整数（即sockfd），失败则返回-1。

memset(&servaddr, 0, sizeof(servaddr))
    用法：将 servaddr 结构体所占的内存区域全部清零。这是一个非常重要的初始化步骤，可以防止垃圾数据影响后续的配置。

htons(atoi(argv[2]))
    用法：这是一个组合操作。
    atoi(argv[2]): 将命令行传入的端口号字符串（如 "9092"）转换成整数。
    htons(): "Host to Network Short"的缩写。它将一个16位的主机字节序整数（我们电脑的存储方式）转换成网络字节序整数（网络传输的标准顺序）。这是保证跨平台兼容性的关键。

inet_pton(AF_INET, argv[1], &servaddr.sin_addr)
    用法：“Presentation to Network”的缩写。它将一个点分十进制的IP地址字符串（如 "192.168.3.20"）转换成网络能够理解的二进制格式，并存入 servaddr.sin_addr 中。

bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))
    用法：将套接字 sockfd 与 servaddr 中定义的地址（IP和端口）绑定在一起。只有绑定后，发送到这个地址的数据才能被该套接字接收。

recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len)
    用法：这是接收UDP数据的核心函数。
    它会从 sockfd 套接字读取数据，存入 buffer。
    0: 通常设为0。
    (struct sockaddr*)&cliaddr: 非常重要，这是一个输出参数。recvfrom 会自动将发送方的地址信息填充到 cliaddr 结构体中。这样服务器就知道是谁发来的数据。
    &len: 传入地址结构体的长度。
    返回值 (nread)：成功则返回接收到的字节数，失败则返回-1。这是一个阻塞函数，如果没有数据到达，它会一直等待。

sendto(sockfd, buffer, nread, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr))
    用法：这是发送UDP数据的核心函数。
    它将 buffer 中的数据通过 sockfd 套接字发送出去。
    nread: 要发送的字节数，这里直接使用刚刚接收到的字节数。
    (struct sockaddr*)&cliaddr: 非常重要，这是目标地址。这里直接使用 recvfrom 获取到的客户端地址，从而实现了“回声”功能。
*/

int main(int argc, char* argv[]) 
{
    int sockfd;//创建的套接字的“代号”，是后续所有网络操作的核心句柄。
    struct sockaddr_in servaddr, cliaddr;// 用于存放服务器(servaddr)和客户端(cliaddr)的地址信息（IP地址、端口号等）的结构体。
    socklen_t len;//用于存放地址结构体的长度，是recvfrom和sendto函数必需的参数。
    char buffer[BUFFER_SIZE];//接收和发送数据的缓冲区，大小为1024字节。
    char timbuffer[64] = "";//用于存放格式化后的时间字符串，用于日志记录。
    char logfilename[64] = "";// 用于存放完整的日志文件名。
    ssize_t nread;// 存放recvfrom函数实际读取到的字节数。
    ssize_t nwrite;// 存放sendto函数实际写入的字节数。
    int frameCnt = 0;// 帧计数器，用于统计总共接收了多少个数据包。
    int i = 0;// 通用的循环计数器变量。
    int ipfrg[4] = { 0,0,0,0 };// 用于存放从字符串中解析出的IP地址的四个部分。
    FILE* log_file = NULL;// 指向日志文件的文件指针。

    //====1.检查命令行参数数量是否正确====
    if (argc != 3) 
    {
        usage(argv[0]);
    }
    //终端的命令: ./udp_server 192.168.3.20 9092

    //====2.创建 UDP 套接字====
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);//socket()向操作系统内核申请创建一个"套接字"的系统调用。想象成一个"网络插座",后续所有的数据收发，都可以通过此窗口进行
    //SOCK_DGRAM：指定使用UDP协议（数据报套接字）。这是与TCP (SOCK_STREAM) 的关键区别。
    /*UDP(用户数据报协议)特点:
        无连接:发送数据前不需要像打电话一样先“建立连接”。
        不可靠:不保证数据一定能送达，也不保证顺序。
        速度快:开销小，传输效率高。*/
    if (sockfd == -1) 
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //====3.初始化服务器地址结构体====
    //创建一个用于描述服务器自身网络地址的"名片"(即servaddr结构体),并用用户提供的IP地址和端口号来填充这张名片。
    //只有当服务器有了明确的地址后，bind()函数才能将其与之前创建的套接字（Socket）绑定，从而让服务器开始在网络上“定点营业”。
    memset(&servaddr, 0, sizeof(servaddr));
    //struct sockaddr_in servaddr
    //servaddr 通常指的是 struct sockaddr_in 类型的一个变量，用于存储服务器的网络地址信息。
    /*
    struct sockaddr_in {
    short            sin_family;   // 地址族 (Address Family)
    unsigned short   sin_port;     // 端口号
    struct in_addr   sin_addr;     // IP 地址
    char             sin_zero[8];  // 填充字节，通常置为0 };
    */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));//atoi将字符串转换成整数; htons将主机字节序转换成网络字节序
    //将用户输入的IP地址字符串转换成网络二进制格式,并进行错误检查的关键步骤。 inet_pton(地址族,源,目的地) 返回值为0(格式错误) 为-1(系统错误)
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
    {
        perror("Invalid IP address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //====4.绑定套接字到服务器地址====
    //将之前创建好的套接字(Socket),牢牢地绑定在配置好的服务器地址上。
    //bind(套接字的"代号" ; 指定要绑定到哪个地址 ; 地址结构体的大小)
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) 
    {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    //告知用户服务器已在哪个IP地址和 端口上开始 "监听"
    printf("UDP server is listening on %s:%d...\n", argv[1], atoi(argv[2]));// ./udp_server 192.168.3.20 9092

    //scanf从标准输入(通常是键盘)读取，而sscanf则从指定的字符串中读取。argv[1]:源字符串; "d%.d%.d%.d%"解析格式模板 
    //&ipfrg[0]...:这些是目标变量的地址。这行代码成功将一个完整的IP地址字符串拆分成了四个独立的整数。
    sscanf(argv[1], "%d.%d.%d.%d", &ipfrg[0], &ipfrg[1], &ipfrg[2], &ipfrg[3]);
    //调试确认步骤,将刚刚存入 ipfrg 数组的四个整数，再用 . 连接起来打印到屏幕上。
    printf("ip=%d.%d.%d.%d\n", ipfrg[0], ipfrg[1], ipfrg[2], ipfrg[3]);
    //调用自定义函数，生成一个唯一的，包含完整路径的日志文件名。
    get_cur_system_time_filename(logfilename, sizeof(logfilename), ipfrg[2]);
    //在上述准备工作就绪后，最终打开(或创建)这个日志文件
    log_file = fopen(logfilename, "a");

    //无限循环，每一次循环都代表着处理一个完整的数据包请求的流程
    /*步骤：1.接收数据：耐心等待，直到接收到一个来自任意客户端的数据包。
            2.记录与诊断：对接收事件进行详细的日志记录，并在屏幕上打印样本数据以供调试。
            3.回声（Echo）：将刚刚收到的数据原封不动地再发送回给客户端。*/
    while (1) 
    {
        // 1.接收客户端数据
        len = sizeof(cliaddr);//cliaddr地址结构体大小
        nread = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,(struct sockaddr*)&cliaddr, &len);
        //接收UDP数据的核心函数recvfrom,阻塞函数。程序会卡在这里进入等待状态，直到有数据包到达这个套接字（sockfd）。
        /*参数:sockfd:从哪个套接字接收 ; buffer:接收到的数据存放在哪里  ; BUFFER_SIZE:最多接收 多少字节
        (struct sockaddr*)&cliaddr:这是UDP编程的精髓所在。是一个输出参数。当recvfrom成功接收到数据后，
        它会自动将发送方（客户端）的IP地址和端口号等信息填充到 cliaddr 这个结构体中。这样，服务器就知道了“这封信是谁寄来的”。*/
        get_cur_system_time(timbuffer, sizeof(timbuffer));
        //在recvfrom返回后，立即调用这个函数获取当前时间。着确保了日志中的事件戳能够尽可能精确地反映出数据包到达的时刻

        //2.处理接收成果
        //接收失败
        if (nread == -1)
        {
            //在屏幕和日志中记录失败的事件
            printf("[%s]recvfrom failed frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);//frameCnt 帧计数器
            fprintf(log_file, "[%s]recvfrom failed frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);
            //实时性; 强制将缓冲区中所有未写入的数据,立刻,马上写入到磁盘文件中。
            fflush(log_file);
            ////保证数据持久化
            fsync(fileno(log_file));
            //fileno(log_file):这个函数的作用是将FILE*类型的文件指针log_file转换成操作系统内核识别的整数 文件描述符(fd).
			//fflush只保证数据从程序的内存缓冲区写入到操作系统的内核缓冲区，但数据可能仍然在操作系统的缓存里，还没真正写入到物理硬盘上。
			//fsync 则会请求操作系统将内核缓冲区的数据也立刻写入到物理存储设备（如硬盘）中。提供最高级别的数据持久化保证。
            continue;
        }
        
        //接收成功
        //在屏幕和日志中记录成功接收的事件，并将帧计数器加1
        printf("[%s]frameCnt = %d nread = %ld\n", timbuffer, frameCnt++, nread);
        fprintf(log_file, "[%s]frameCnt = %d nread = %ld\n", timbuffer, frameCnt, nread);
        fflush(log_file);
        fsync(fileno(log_file));

        //数据样本的诊断性打印
        //打印接收到的数据包的头部16个字节
        for (i = 0;i < 16;i++)
        {
            printf("%02x ", buffer[i]);
        }

        printf("...");
        //打印接收到数据包的尾部16个字节
        for (i = nread - 16;i < nread;i++)
        {
            printf("%02x ", buffer[i]);
        }
        printf("\n\n");

        // 将接收到的数据回显给客户端
        //sendto专门用于无连接的套接字(如UDP)上发送数据的系统调用。允许在发送数据的同时,指定一个明确的目标地址。返回值成功为：发送出去的字节数 失败：-1
        /*参数详解:1.sockfd: 从哪个套接字（网络窗口）发送出去。
        2.buffer: 要发送的数据内容。这里直接使用了刚刚从 recvfrom 接收并存放在 buffer 里的数据，实现了“原样返回”。
        3.nread:要发送的字节数。这里也直接使用 recvfrom 返回的 nread，确保了发送的数据不多不少，正好是客户端发来的那么多。
        5.(struct sockaddr*)&cliaddr;这个参数是目标地址。我们在这里直接使用上一步 recvfrom 函数为我们自动填充好的、
            包含了客户端地址信息的 cliaddr 结构体。sendto 会解析这个地址，并将数据准确地发送回给它。*/
        nwrite = sendto(sockfd, buffer, nread, 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr));
        if (nwrite == -1)
        {
            perror("sendto failed\n");
            fprintf(log_file, "[%s]sendto failed\n", timbuffer);
        }
    }
    fclose(log_file);
    close(sockfd);
    return 0;
}

/*
小结：
`udp_server` 和 `udp_client` 这两个程序之间的业务逻辑，它们是如何协同工作的。

您可以把这两个程序想象成一个**“乒乓球”**的测试游戏，目的是为了看看在有“狂风”（电磁干扰）的情况下，球能不能稳定地打回来。
### **`udp_server` 的业务逻辑：做一个可靠的“墙壁”**
`udp_server` 在这个测试中扮演的角色非常简单、被动，但至关重要。它的业务逻辑可以概括为：**做一个忠实可靠的“回声墙”**。
#### 它的工作流程是：

1.  **“占好位置” (初始化)**
    * 程序启动时，你必须告诉它要在哪个**IP地址**和**端口号**上“营业”。
    * 它会创建一个网络通道（Socket），并把自己牢牢地绑定在这个地址上，就像墙壁固定在球场的一端。
    * 同时，它会准备好一个日志本，记录下所有发生的事情。

2.  **“无限等待” (主循环)**
    * 它会进入一个无限循环，调用 `recvfrom` 函数，然后就**耐心等待**。程序会“卡”在这里，不消耗CPU，直到有“球”（数据包）打过来。

3.  **“反射！” (回声)**
    * 一旦有任何客户端（比如 `udp_client`）发来一个数据包，`recvfrom` 函数就会立刻被激活。
    * 服务器会做的**唯一一件事**就是：调用 `sendto` 函数，把这个**刚刚收到的、一模一样的、未经任何修改的数据包**，再原封不动地**发回**给刚才那个客户端。
    * 它通过 `recvfrom` 自动获取了客户端的地址，所以它知道该把球“反射”回哪里去。
    * 完成反射后，它会在日志本上记一笔：“某时某刻，我收到了一个球，并把它弹回去了。”

4.  **回到等待**
    * 完成一次反射后，它立刻回到第二步，继续等待下一个球的到来。

**`udp_server` 的核心特点**：
* **被动**：它从不主动发起通信。
* **无状态**：它不关心数据包的内容是什么，也不记得到底收了多少个包，它的任务就是机械地、忠实地反射。
* **可靠**：它是整个测试的基准，我们假定在没有干扰的情况下，它总能正确地完成回声任务。

---

### **`udp_client` 的业务逻辑：一个严格的“发球手”和“裁判员”**

`udp_client` 是整个测试的**“主角”**。它既负责主动发起测试，又负责判断测试结果的好坏。

#### 它的工作流程是：

1.  **“赛前准备” (初始化)**
    * 启动时，你不仅要告诉它**目标服务器的地址**，还要告诉它**自己应该从哪个本地地址**发球。这在有多网卡的设备上很重要。
    * 它会创建自己的网络通道（Socket）并绑定到本地地址。
    * **最关键的一步**：它会给自己的网络通道设置一个 **0.3秒的接收超时“闹钟”**。
    * 它也会准备好自己的日志本，以及一个用于“亮红牌”的**LED告警灯**。

2.  **“发球 -> 等球 -> 判罚” (主循环)**
    * 它会进入一个无限循环，周而复始地执行以下动作：
        * **发球 (Send)**：构造一个测试数据包，通过 `sendto` 函数，朝着目标服务器的地址把球打出去。并记录：“我发出了一号球”。
        * **等球 (Wait)**：打出球后，它会立刻调用 `recvfrom` 函数，尝试接收服务器弹回来的球。但由于设置了“闹钟”，它最多只等 **0.3秒**。
        * **判罚 (Verify)**：0.3秒后，会出现两种结果：
            * **情况A：球回来了 (`recvfrom` 成功)**：裁判员很高兴，在日志里记一笔：“一号球成功回来了！”
            * **情况B：球没回来 (`recvfrom` 因超时而失败)**：裁判员会判定“丢球”了！它会在日志里记一笔：“二号球超时了，失败一次！”，然后立刻**闪烁一下LED告警灯**，就像裁判亮出了一张红牌，给现场测试人员一个直观的物理警报。
    * **控制节奏**：每次发球后，它会固定休息一小段时间（`usleep`），以稳定的频率进行测试。

**`udp_client` 的核心特点**：
* **主动**：是它发起了整个通信过程。
* **有状态**：它内部有计数器，精确地记录了自己发了多少球，成功收回了多少球，失败了多少次。
* **是裁判**：测试的**通过/失败标准**（即0.3秒超时）是由客户端来定义的。只有它才能判断一次通信来回是否成功。

### **总结**

`udp_server` 和 `udp_client` 共同构成了一个**完整的闭环测试系统**：

* **`udp_client` (客户端)** 不断地向 **`udp_server` (服务器)** 发送数据包。
* **`udp_server` (服务器)** 忠实地将这些数据包再反射回 **`udp_client` (客户端)**。
* **`udp_client` (客户端)** 负责验证这个“一来一回”的过程是否在指定的时间内完成。如果未完成，就判定为一次失败，并发出告警。

通过观察客户端的输出日志和LED灯，测试人员就能非常直观地判断出网络链路在电磁干扰下的稳定性和可靠性。
*/
