#include <stdio.h>//标准的输入输出 perror打印错误信息,用于printf fflush等函数
#include <stdlib.h>//例如内存分配，字符串转换，本例中调用atoi函数(将传入的字符串转换成整数)和system(执行shell命令)函数
#include <time.h>// C语言的时间库，用于处理时间结构，例如 struct tm, mktime 函数。
#include <sys/time.h>//系统时间库，定义了struct timeval和settimeofday函数
#include <unistd.h>// 提供对 POSIX 操作系统 API 的访问，settimeofday 实际上也常在这里被包含


//前向声明;
void TimeSet(int year, int month, int day, int hour, int min, int sec);


int main(int argc, char* argv[])
{
	//---变量定义
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;
	//1.打印设置前的时间
	printf("before time set");
	fflush(stdout);//保证程序正确的执行顺序
	/*拓展：I/O缓冲区
		计算机中直接的I/O操作(如写入磁盘在屏幕上显示字符)相对于CPU的计算速度来说比较慢。为提高效率,操作系统和C语言引入了缓冲区(BUffer)机制
		例如超市购物:无缓冲:每拿起一件商品，就立刻去收银台结账一次。
					有缓冲：把所有想买的商品都放进购物车（这就是缓冲区），直到购物车满了，或者你全部买完了，才去收银台一次性结账。
		三种缓冲类型:全缓冲:在缓冲区完全填满后，才会执行实际的 I/O 操作。通常用于对文件的读写。
					行缓冲 (Line Buffering): 当遇到换行符 \n 时，或者当缓冲区满时，才执行 I/O 操作。标准输出 stdout 在连接到终端时，默认就是行缓冲。
					无缓冲 (No Buffering): 数据被立即写入，不经过任何缓冲。标准错误流 stderr 通常是无缓冲的，这样可以确保错误信息能被立即看到。
		fflush 的作用:强制将指定流（stream）的缓冲区中所有未写入的数据，立即发送到其最终目的地（例如屏幕、文件等），并清空该缓冲区。*/
	system("date");//调用shell命令显示当前时间

	// 2. 从命令行参数获取时间值
    // argc 是参数个数，argv 是一个字符串数组，存放着所有参数
    // argv[0] 是程序名本身
    // argv[1] 是第一个参数，以此类推
	//atoi("ASCII to integer") 函数将这些字符串参数转换为整数
	year = atoi(argv[1]);
	month = atoi(argv[2]);
	day = atoi(argv[3]);
	hour = atoi(argv[4]);
	min = atoi(argv[5]);
	sec = atoi(argv[6]);

	// 3. 调用核心函数设置时间
	TimeSet(year, month, day, hour, min, sec);

	// 4. 打印设置后的时间，以验证结果
	printf("after time set");
	fflush(stdout);
	system("date");
	return 0;
}


void TimeSet(int year, int month, int day, int hour, int min, int sec)
{
	struct tm tptr;
	//struct tm 是C标准库中用于表示“分解时间”或“日历时间”的结构体。例如月份tm_mon 范围是0到11 年份tm_year是从1900年开始经过的年数。表示2025，就是125
	struct timeval tv;
	//表示时间戳的结构体。time_t tv_sec; //秒(来自1970年1月1日 00::00::00的总秒数)   suseconds_t tv_usec;//微妙

	tptr.tm_year = year - 1900;
	tptr.tm_mon = month - 1;
	tptr.tm_mday = day;
	tptr.tm_hour = hour;
	tptr.tm_min = min;
	tptr.tm_sec = sec;

	tptr.tm_isdst = 0;//夏令时必须设置，不设置mktime大概率返回-1;等于0就是标准时间

	//系统无法直接使用struct tm,所以需要转换成一个标准的time_t时间戳。转换由mktime()函数完成。
	tv.tv_sec = mktime(&tptr);//接收一个指向struct tm(日历时间)的指针
	tv.tv_usec = 0;//目标精确到秒，所以直接设置为0
	settimeofday(&tv, NULL);//执行系统时间修改的系统调用（必须以root用户身份）

}

/*代码的整体逻辑于用途：
核心用途:通过命令行来设置测控一体机的系统时间。需要保证：
1.日志记录准确：所有测试过程中产生的事件和日志都能被赋予正确的时间，便于后续分析问题。
2.事件同步：如果有多台设备协同测试，统一的时间基准是保证事件顺序和同步性的前提。

整体步骤：
1.接收参数：程序启动时，会检查用户是否在命令行中输入了7个参数（程序名 + 年、月、日、时、分、秒）。如果参数数量不对，它会打印一条错误提示并退出。
2.转换参数：它将用户输入的字符串格式的年月日等参数（例如 "2025"）转换成整数格式。
3.打包时间：将这些独立的整数（年、月、日、时、分、秒）填充到一个专门的时间结构体 (struct tm) 中。
4.格式化时间：调用系统函数将这个结构体转换成一个标准的、操作系统能够理解的“时间戳”格式（从1970年1月1日至今的总秒数）。
5.设置系统时间：使用这个生成的时间戳来更新系统的时钟。
6.反馈结果：打印出设置成功后的时间，告知用户操作已完成。
*/