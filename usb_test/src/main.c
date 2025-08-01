

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define POLYNOMIAL 0xEDB88320
#define BUFFER_SIZE 4096

#define FILE_SIZE 1024*256
#define FILE_SIZE_STEP 64
unsigned int filebuf[FILE_SIZE];
unsigned int crc32_table[256];


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

void get_cur_system_time_filename(char* pNameBuf, int len, int usbno)
{
    struct timeval tv;
    struct tm tm_info;
    char timbuffer[64] = "";
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    strftime(timbuffer, len, "%Y-%m-%d_%H-%M-%S", &tm_info);
    sprintf(pNameBuf, "./usb_log/usb%d_%s.log", usbno, timbuffer);
}


void init_crc32_table()
{
    unsigned int crc;
    for (unsigned int i = 0; i < 256; i++)
    {
        crc = i;
        for (unsigned int j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ POLYNOMIAL;
            }
            else
            {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
}

unsigned int crc32(const unsigned char* data, size_t length)
{
    unsigned int crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        unsigned char table_index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[table_index];
    }
    return crc ^ 0xFFFFFFFF;
}

//核心任务:构造一个包含已知数据和校验码的大数据块，并将其完整地写入到指定的usb设备中
// filename: 要写入的目标设备文件名（例如 "/dev/sda"）。
// pbuf: 指向内存缓冲区的指针，函数将在这里构造数据，然后写入设备。
// len: 要处理的数据块的总大小（以unsigned int为单位）。
int write_file(const char* filename, unsigned int* pbuf, int len)
{
    // 1. 打开目标设备文件
    // "wb" 表示以“写入二进制(write binary)”模式打开文件。
    // 如果文件不存在，它会被创建；如果已存在，其内容将被清空。
    FILE* file = fopen(filename, "wb");
    int i = 0;
    int file_size = len;// 将传入的长度赋给本地变量

    // 2. 检查文件是否成功打开
    if (!file)// 如果fopen返回NULL，表示打开失败
    {
        // 打印标准的系统错误信息
        perror("Failed to open file");
        return -1;
    }

    // 3. 构造数据内容
    // 这个for循环填充了缓冲区除最后一个位置外的所有空间
    for (i = 0; i < file_size - 1; i++)
    {
        // 填充一个递增的数字序列
        pbuf[i] = i;
    }

    // 4. 计算并填充CRC32校验码
  /*a. (const unsigned char*)pbuf: 将unsigned int指针强制转换为unsigned char指针，因为CRC计算是按字节进行的。
    b. (file_size - 1) * 4: 计算要校验的数据的总字节数。因为数组是unsigned int类型（通常占4字节）所以要校验的元素数量(file_size - 1)需要乘以4。
    c. crc32(...): 调用crc32函数计算出校验码。
    d. pbuf[file_size - 1] = ...: 将计算出的32位校验码，存放在缓冲区的最后一个unsigned int位置。*/
    pbuf[file_size - 1] = crc32((const unsigned char*)pbuf, (file_size - 1) * 4);

    // 5. 将整个缓冲区的数据写入文件
    // fwrite是标准的文件写入函数。
    // - pbuf: 要写入的数据源。
    // - sizeof(unsigned int): 每个数据项的大小（通常是4字节）。
    // - file_size: 总共有多少个数据项。
    // - file: 要写入的目标文件。
    // 这行代码会将整个256KB的缓冲区内容一次性写入到U盘。
    fwrite(pbuf, sizeof(unsigned int), file_size, file);
    fclose(file);

    return 0;
}


// filename: 要读取的目标设备文件名（例如 "/dev/sda"）。
// pbuf: 指向内存缓冲区的指针，函数会将从设备读取的数据存放在这里。
// len: 要读取的数据块的总大小（以unsigned int为单位）。
int read_file(const char* filename, unsigned int* pbuf, int len)
{
     // 1. 打开目标设备文件
    // "rb" 表示以“读取二进制(read binary)”模式打开文件。
    FILE* file = fopen(filename, "rb");
    int i = 0;
    int file_size = len;// 将传入的长度赋给本地变量

     // 2. 检查文件是否成功打开
    if (!file)//如果fopen返回是NULL,表示打开失败
    {
        perror("Failed to open file");
        return -1;
    }

    //3.部分清空内存缓冲区
    //这个for循环的目的是为了绕过操作系统的文件缓存，确保数据是从物理U盘读取的。
    for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)
    {
        // FILE_SIZE_STEP 的值是64，所以这个循环会每隔64个整数，
        // 就将缓冲区中的一个元素清零。
        // 这就“弄脏”了内存中的数据，使得操作系统不能直接从缓存中提供数据，
        // 而是必须去访问物理设备来获取完整、正确的数据。
        pbuf[i] = 0;
    }

    // 4. 从文件中读取整个数据块
    // fread是标准的文件读取函数。- pbuf: 存放读取数据的目的地缓冲区。- sizeof(unsigned int): 每个数据项的大小（通常是4字节）。
    // - file_size: 总共要读取多少个数据项。 - file: 从哪个文件读取。
    // 这行代码会从U盘一次性读取256KB的数据，并覆盖写入到pbuf缓冲区中，从而恢复那些在第3步中被清零的数据。
    fread(pbuf, sizeof(unsigned int), file_size, file);
    fclose(file);
    return 0;
}



int main(int argc, char* argv[])
{
    const char* filename = "/dev/sda";
    //常量字符串指针，它硬编码了要进行测试的目标设备。/dev/sda 通常是第一个被系统识别的块存储设备，这个程序的所有读写操作都将针对这个设备文件进行。
   
    int no = atoi(argv[1]);//提取出一个数字编号，用于构成一个唯一的日志文件名
    //./usb_test 1  ./usb_test 2
    char logfilename[64] = "";//字符数组，用来存放最终生成的，包含完整路径的日志文件
    FILE* log_file = NULL;//文件指针
    int i = 0;
    int cnt = 0;
    int loopcnt = 0; //主测试循环的计数器，记录总共执行了多少次“写入-读出-校验”的完整流程。
    unsigned int crc = 0;
    bool bfail = 0;//布尔类型标志位，作用是在数据对比的for循环中，标记当前这次测试是否失败。如果检测到任何一个字节不匹配，就可以将 bfail 设置为 true (或 1)。
    int file_size = FILE_SIZE;//这个变量定义了每次读写测试的数据块大小。即256KB
    char timbuffer[64] = "";//临时的字符串缓冲区，专门用来存放由get_cur_system_time()函数生成的,带格式的时间戳，用于日志记录。

    //核心任务是获取当前的系统时间，并结合用户传入的编号 (no)，生成一个唯一的、带完整路径的日志文件名。
    get_cur_system_time_filename(logfilename, sizeof(logfilename), no);
    log_file = fopen(logfilename, "a");//打开文件
    init_crc32_table();//初始化高效的CRC校验工具
    while (1)
    {
        bfail = 0;
        /*在每次循环开始时，都将失败标志位 bfail 重置为0 (false)。这确保了每一次测试都是从一个“假设成功”的状态开始的，
        只有在后续的校验中发现问题，这个标志才会被设置为1。*/
        loopcnt++;//主循环计数器加一。这用于在日志中追踪总共执行了多少次测试。
        //get_cur_system_time(timbuffer, sizeof(timbuffer));
        //printf("[%s] start:loopcnt = %d\n", timbuffer, loopcnt);
        write_file(filename, filebuf, FILE_SIZE);
        /*用递增的数字序列 (0, 1, 2, ...) 填充传入的内存缓冲区 filebuf 的前 FILE_SIZE - 1 个位置。调用crc32()函数，对刚刚填充的数据计算出一个CRC32校验码。
            放入filebuf的最后一个位置，最后,通过fopen和fwrite,将这个包含了数据和校验码，完整的filebuf(256KB),一次性写入到filename中*/

        //部分清空内存缓冲区，目的是为了绕过操作系统的文件缓存机制
        for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)
        {
            filebuf[i] = 0;
            //工作原理:这个 for 循环并没有清空整个 filebuf。它以 FILE_SIZE_STEP (64) 为步长，每隔64个整数就将其中一个清零。
            //filebuf[i] = 0;: 通过故意“弄脏”内存中 filebuf 的一部分数据，程序确保了当它下一次读取文件时，内存中的数据是不完整的、被破坏的。这会迫使操作系统必须去访问物理U盘
        }

        //从filename(即/dev/sda)中读取 FILE_SIZE (256KB) 的数据，覆盖写回到内存中的 filebuf 缓冲区里。
        read_file(filename, filebuf, FILE_SIZE);

        //每进行10次循环，就执行一次花括号里的代码。
        if (loopcnt % 10 == 0)
        {
            //filebuf[loopcnt% FILE_SIZE] = 0;//被注释掉的代码，为人为地制造错误将读回的数据中的某个字节修改为0.后续的CRC校验和内容比对就必然会失败。
        }

        //第一重保险——CRC32完整性校验
        //将都会来的filebuf缓冲区的前file_size - 1 个整数进行计算。得到crc校验码
        crc = crc32((const unsigned char*)filebuf, (file_size - 1) * 4);

        //它将刚刚计算出的crc值进行比较
        if (crc != filebuf[file_size - 1])
        {
            bfail = 1;//如果校验失败，则将失败标志位bfail设置为1(true),表明本次测试已经失败了。
        }

        //第二重保险——抽样内容详查
        if (bfail == 0)//确保了只有在第一重CRC校验成功通过时，才会进行这第二重检查。
        {
            for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)//抽样检查的循环。以 FILE_SIZE_STEP (64) 为步长，跳跃式地检查。
            {

                if (filebuf[i] != i)//检查缓冲区在 i 位置的值，是否还等于当初写入的那个递增的数字 i。
                //因为 write_file 函数当初就是用 pbuf[i] = i; 的方式填充的数据，所以如果读写正常，这里的等式 filebuf[i] == i 必须成立。
                {
                    bfail = 1;
                    break;
                }
            }

        }

        //失败处理模块
        if (bfail == 1)
        {
            cnt++;// 失败计数器加一
            get_cur_system_time(timbuffer, sizeof(timbuffer)); // 获取当前精确时间
            // 在屏幕上打印错误日志
            printf("[%s]crc bad cnt = %d\n", timbuffer, cnt);
            //  将同样的错误日志写入日志文件
            fprintf(log_file, "[%s]crc bad cnt = %d\n", timbuffer, cnt);
            //  强制将日志缓冲区内容刷入磁盘，确保关键错误信息不丢失
            fflush(log_file);
            fsync(fileno(log_file));

        }
        //get_cur_system_time(timbuffer, sizeof(timbuffer));
        //printf("[%s] end:loopcnt = %d\n", timbuffer, loopcnt);
        usleep(1000000);
    }

    fclose(log_file);
    return 0;
}

/*
小结:
第一阶段：准备工具 (初始化)
在开始正式检测之前，机器人需要准备好它所有的工具和记录本。这个过程在 main 函数进入 while(1) 循环前完成。
1.接收任务编号 (int no = atoi(argv[1]);)
    业务逻辑：用户在启动程序时，必须给机器人分配一个任务编号（例如 ./usb_test 1）。
                这个编号就像是贴在本次测试报告上的一个标签，用来区分不同的测试任务。

2.准备日志本 (get_cur_system_time_filename(...) 和 fopen(...))
    业务逻辑：机器人会根据任务编号和当前的精确时间，创建一个独一无二的日志文件（例如 usb_log/usb1_2025-07-28_15-10-25.log）。
                所有后续的检测结果都会被记录在这个专用的“日志本”里。

3.学习“速算技巧” (init_crc32_table();)
    业务逻辑：机器人需要一种快速判断数据是否完好的方法，这个方法叫CRC32校验。为了提高效率，
                它不会每次都从头计算，而是在开始工作前，调用 init_crc32_table()  函数，预先生成一本“
                CRC32速查表”。这就像人提前背好了乘法口诀表一样，后续的校验速度会快非常多。

当所有这些准备工作完成后，机器人就进入了它的核心工作模式。

第二阶段：无限循环检测 (while(1) 主循环)
机器人会永远地、不知疲倦地重复执行一套严格的“存入 -> 取出 -> 检验”流程。每一次循环，都代表一次完整的读写可靠性测试。

步骤 1: 存入标准货物 (写入)
write_file(filename, filebuf, FILE_SIZE);
    a.业务逻辑：机器人首先调用 write_file  函数，在内存中精心准备一批“标准货物”。这批货物是一个256KB的大数据块，内容是固定的、从0开始递增的数字序列。
    b.贴上“防伪标签”：在准备好货物后，机器人会用它的“速算技巧”（CRC32）为这批货物计算出一个唯一的“指纹”（校验码），并把这个指纹贴在货物的末尾。
    c.存入仓库：最后，它将这批贴好了防伪标签的、完完整整的256KB标准货物，一次性地存入到“仓库”（也就是U盘 /dev/sda）中。

步骤 2: 取出货物 (读取)
read_file(filename, filebuf, FILE_SIZE);
    a.业务逻辑：机器人接着调用 read_file  函数，尝试从刚才存货的U盘位置，把那批256KB的货物再取出来，放回内存中。
    b.一个聪明的技巧（绕过缓存）：在调用 read_file 之前，代码故意执行了一个 for 循环，把内存中刚刚写入的部分数据弄脏（清零）。
                                这就像是故意撕掉了几张货物清单。这么做的目的是强制操作系统必须去访问物理U盘来获取完整的数据，
                                而不是图省事直接从系统的高速缓存里拿数据。这保证了测试的是真正的物理读写能力。

步骤 3: 严格检验 (验证)
这是整个流程最关键的一步，机器人会对取出的货物进行双重检验。
    a.第一重检验：检查“防伪标签” (CRC校验)
                crc = crc32(...) 
                if (crc != filebuf[file_size - 1]) 
    业务逻辑：机器人首先对取出的货物（除末尾的标签外）用同样的“速算技巧”重新计算一遍“指纹”。
                然后，它对比新算出的指纹和货物上自带的原始防伪标签。如果两者不一致，说明货物在存放或取出的过程中已经损坏了。

第二重检验：抽查“货物内容” (内容比对)
    if (bfail == 0) 
    for (...) { if (filebuf[i] != i) ... } 
    业务逻辑：只有在第一重防伪标签检验通过后，机器人还会进行一次更仔细的抽样检查。
             它会跳跃式地检查货物中的几个关键位置，看看里面的内容是否还是当初存入的那个递增的数字。
             这可以作为CRC校验的一个补充，确保数据内容没有发生更诡异的变化。

步骤 4: 记录结果与控制节奏
    if (bfail == 1) 
    业务逻辑：在两重检验中，只要有任何一关失败，机器人就会举起一个叫 bfail 的“失败”旗帜。在这个最终判断的环节，如果旗帜被举起了，机器人就会：
            1.在失败计数器 cnt 上加一。
            2.获取当前精确时间。
            3.在屏幕和日志本上，同时记录一条详细的错误信息：“报告！又坏了一个！累计已坏 N 个！”
            4.强制将日志写入硬盘 (fflush 和 fsync)，确保即使下一秒系统因为干扰而崩溃，这条宝贵的错误记录也不会丢失。


usleep(1000000); 

业务逻辑：在完成一整套“存入-取出-检验”流程后，无论成功还是失败，机器人都会固定休息1秒钟。这确保了测试是以一个稳定的、可持续的频率进行的，既保证了测试强度，也避免了对系统造成过度的冲击。

休息1秒后，机器人会回到 while(1) 的开头，开始下一次一模一样的检测流程，周而复始，永不停止。
*/