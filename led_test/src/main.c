#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <time.h>
#include <string.h>

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

void daemon_work() {
	int log_fd = open("/var/log/led_test.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (log_fd < 0) {
		perror("open log error");
		exit(EXIT_FAILURE);
	}
	system("echo 20 > /sys/class/gpio/export  ");
	while (1) {
		time_t now = time(NULL);
		char log_msg[256];
		strftime(log_msg, sizeof(log_msg), "running current time %Y-%m-%d %H:%M:%S\n", localtime(&now));
		write(log_fd, log_msg, strlen(log_msg));
		system("echo 1 > /sys/class/gpio/gpio20/value");
		sleep(1);
		system("echo 0 > /sys/class/gpio/gpio20/value");
		sleep(1);
	}
	close(log_fd);
}

int main() {
	daemon_init(); // 初始化守护进程
	daemon_work(); // 运行主业务逻辑
	return 0;
}
