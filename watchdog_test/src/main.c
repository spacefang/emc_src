#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>


// 守护进程初始化函数
void daemon_init() {
	pid_t pid;

	// 步骤1：fork子进程，父进程退出
	if ((pid = fork()) < 0) {
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if (pid > 0) {
		exit(EXIT_SUCCESS); // 父进程退出
	}

	// 步骤2：创建新会话
	if (setsid() < 0) {
		perror("setsid error");
		exit(EXIT_FAILURE);
	}

	// 步骤3：改变工作目录到根目录
	if (chdir("/") < 0) {
		perror("chdir error");
		exit(EXIT_FAILURE);
	}

	// 步骤4：设置文件权限掩码
	umask(0);

	// 步骤5：关闭所有文件描述符
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		perror("getrlimit error");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < rl.rlim_max; i++) {
		close(i);
	}

	// 步骤6：重定向标准输入输出到/dev/null
	int fd = open("/dev/null", O_RDWR);
	if (fd < 0) {
		perror("open /dev/null error");
		exit(EXIT_FAILURE);
	}
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
}

/*硬件看门狗
可以把硬件看门狗想象成一个安装在主板上的、独立的**“定时炸弹”**。它的工作规则是：
   *一旦你启动它，它就开始一个比如说10秒的倒计时。
   *你的主程序必须在10秒内去“喂”它一下（向它写入数据），这个动作就像是“拆除炸弹”并重置倒计时。
   *如果你的主程序运行正常，它会周而复始地在倒计时结束前去“喂狗”。
   *但是，如果你的主程序因为任何原因卡死了、崩溃了（比如在严重的电磁干扰下），它就无法再去“喂狗”。
   *当10秒倒计时结束而无人来“喂”时，“炸弹”就会引爆。这个“引爆”的动作不是破坏硬件，而是会强制重启整个系统，就像你按下电脑的重启按钮一样。

因此，watchdog_test 程序的业务逻辑就是：“启动并持续地‘喂狗’，以验证在EMC干扰下，即使主程序本身没有崩溃，看门狗的‘喂狗’机制和硬件重启功能是否依然可靠。
” 同时，它也作为一个测试样本，如果这个程序本身被干扰卡死了，我们就可以观察整台机器是否能如预期般被看门狗重启。*/

int daemon_work() {
	int dog_fd_, ret;//dog_fd_是后续代码中用来存放打开看门狗设备(/dev/watchdog)后返回的文件描述符。ret 用来存放函数调用的返回值，以便进行错误检查
	int log_fd = open("/var/log/watchdog_test.log", O_WRONLY | O_CREAT | O_APPEND, 0644);//尝试打开或创建一个日志文件，将其文件描述符存入log_fd变量
	char timbuffer[256];//用于存放格式化后的时间戳字符串
	char log_msg[512];//用于拼接最终要写入日志的完整消息
	time_t now = time(NULL);//调用time()函数获取当前的日历时间(从1970年1月1日到现在的总秒数)，并将其存放在now变量中
	//错误处理模块
	if (log_fd < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));//strftime将时间日期信息格式化成一个可自定义的字符串
		//将之前获取的now时间，格式化成一个人类可读的字符串(如 "2025-07-29 02:15:30\n")，并存入 timbuffer。
		sprintf(log_msg, "[%s]%s", timbuffer, "open watchdog_test.log failed\n");//sprintf将格式化的数据写入一个字符串中
		//将时间戳和错误信息 "open watchdog_test.log failed\n" 拼接成一条完整的日志消息，存入 log_msg
		//write(log_fd, log_msg, strlen(log_msg));//log_fd无效,
		fprintf(stderr, "%s", log_msg);
		exit(EXIT_FAILURE);
	}

	//尝试打开并获取对硬件看门狗设备的控制权，并对可能发生的“打开失败”这一错误进行处理。
	dog_fd_ = open("/dev/watchdog", O_WRONLY);
	if (dog_fd_ < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "open watchdog failed\n");
		//write(log_fd, log_msg, strlen(log_msg));
		fprintf(stderr, "%s", log_msg);
		return -1;
	}

	//核心任务:向看门狗硬件发送一个特殊指令，告诉它：如果因为超时而触发重启，请不要在程序关闭文件描述符时自动禁用自己。
	int action = WDIOS_DISABLECARD;
	//WDIOS_DISABLECARD:这是一个在看门狗驱动头文件中定义的常量宏。
	// 当关闭看门狗设备文件时（即调用close(dog_fd_)）,不要自动停止（disable）看门狗的计时
	//默认行为:在很多看门狗驱动的默认设置下，一旦你 close 了 /dev/watchdog 文件，看门狗就会认为你的程序已经正常结束，于是它就会自动停止计时，不再有重启系统的能力。
	ret = ioctl(dog_fd_, WDIOC_SETOPTIONS, &action);//参数1：要控制的设备(看门狗),参数2：要设置一个选项 , 参数3：要设置的具体选项
	//ioctl:Input/Output Control 通用的，用于向设备驱动程序发送控制命令的系统调用
	if (ret < 0) {//操作失败
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETOPTIONS action:WDIOS_DISABLECARD failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;//表示这个句柄已经不再有效
		return -1;//终止当前函数执行
	}

	//核心任务:向看门狗硬件发送一个指令，明确地告诉它：你的‘定时炸弹’倒计时应该是10秒。
	int timeout_ = 10;//定义超时值
	ret = ioctl(dog_fd_, WDIOC_SETTIMEOUT, &timeout_);//发送设置指令:通过 ioctl 系统调用，将这个10秒的超时值发送给看门狗设备。
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETTIMEOUT failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	//核心任务:向看门狗硬件反向查询：‘请告诉我，你当前的超时时间到底被设置成了多少秒？’，然后对查询结果进行确认。
	//1.发送查询指令并接收结果
	ret = ioctl(dog_fd_, WDIOC_GETTIMEOUT, &timeout_);
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_GETTIMEOUT failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	//核心任务:向看门狗硬件发送一个‘启用’指令，正式开启看门狗的倒计时功能。
	//1.准备"启用"指令
	action = WDIOS_ENABLECARD;//常量宏，其作用就是激活并启动看门狗的硬件定时器。在执行这条指令之前，看门狗硬件处于待机或禁用状态。
	ret = ioctl(dog_fd_, WDIOC_SETOPTIONS, &action);//发送指令并检查结果
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETOPTIONS action:WDIOS_ENABLECARD failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	//循环前的启动日志
	strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));//将日期信息格式化成字符串
	sprintf(log_msg, "[%s]%s", timbuffer, "feed dog 5s,dog timeout 10s\n");//将格式化的数据写入一个字符串当中
	//工作模式：我将每5秒喂一次狗，而狗的重启超时时间是10秒
	write(log_fd, log_msg, strlen(log_msg));//写入日志文件

	//喂狗的主循环
	while (1) {
		//1.固定休眠
		sleep(5);
		//2."喂狗"
		ret = ioctl(dog_fd_, WDIOC_KEEPALIVE, NULL);// WDIOC_KEEPALIVE:每次看门狗硬件收到这个命令，就会将它内部的10秒倒计时，重新复位，再次从10秒开始倒数
		//3.记录存活日志
		time_t now = time(NULL);//每次成功喂狗后，在日志文件中记录当前时间
		strftime(log_msg, sizeof(log_msg), "running current time %Y-%m-%d %H:%M:%S\n", localtime(&now));
		//将当前时间now格式化成一个人类可读的字符串
		write(log_fd, log_msg, strlen(log_msg));
		//调用将 log_msg 中格式化好的日志消息，直接写入到由 log_fd 代表的日志文件中。
		if (ret < 0) {
			strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
			sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETOPTIONS action:WDIOS_DISABLECARD failed\n");
			write(log_fd, log_msg, strlen(log_msg));
			close(dog_fd_);
			dog_fd_ = -1;
			return -1;
		}
	}
	close(dog_fd_);
	return  0;
}

int main() {
	daemon_init(); // 初始化守护进程
	daemon_work(); // 运行主业务逻辑
	return 0;
}

/*
如何判断结果:
1.理想情况（测试通过）：在干扰结束后，设备依然正常运行。你可以查看 watchdog_test 生成的日志文件，会发现它在整个干扰期间，
						依然在以大约5秒一次的频率持续不断地记录“喂狗”日志。这证明了主程序和看门狗的通信机制是稳定的。
2.最坏但符合预期的情况（测试也算通过）：在强干扰下，操作系统或 watchdog_test 程序本身被“卡死”了。这时，程序无法再按时“喂狗”。
						在等待大约10-15秒（取决于硬件设置）后，整台设备会自动重启。重启后系统能恢复正常工作。
						这证明了看门狗的硬件重启功能是有效的，它成功地将系统从“假死”状态中拯救了出来。
3.测试失败：在强干扰下，设备死机了，但看门狗并没有触发重启，设备一直处于卡死状态，需要手动断电才能恢复。这说明看门狗硬件或其驱动在干扰下失效了。

*/