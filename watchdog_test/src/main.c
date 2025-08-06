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

int daemon_work() {
	int dog_fd_, ret;
	int log_fd = open("/var/log/watchdog_test.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	char timbuffer[256];
	char log_msg[512];
	time_t now = time(NULL);
	if (log_fd < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "open watchdog_test.log failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		exit(EXIT_FAILURE);
	}
	dog_fd_ = open("/dev/watchdog", O_WRONLY);
	if (dog_fd_ < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "open watchdog failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		return -1;
	}

	int action = WDIOS_DISABLECARD;
	ret = ioctl(dog_fd_, WDIOC_SETOPTIONS, &action);
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETOPTIONS action:WDIOS_DISABLECARD failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	int timeout_ = 10;
	ret = ioctl(dog_fd_, WDIOC_SETTIMEOUT, &timeout_);
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETTIMEOUT failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	ret = ioctl(dog_fd_, WDIOC_GETTIMEOUT, &timeout_);
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_GETTIMEOUT failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}
	action = WDIOS_ENABLECARD;
	ret = ioctl(dog_fd_, WDIOC_SETOPTIONS, &action);
	if (ret < 0) {
		strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S\n", localtime(&now));
		sprintf(log_msg, "[%s]%s", timbuffer, "ioctl WDIOC_SETOPTIONS action:WDIOS_ENABLECARD failed\n");
		write(log_fd, log_msg, strlen(log_msg));
		close(dog_fd_);
		dog_fd_ = -1;
		return -1;
	}

	strftime(timbuffer, sizeof(timbuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
	sprintf(log_msg, "[%s]%s", timbuffer, "feed dog 5s,dog timeout 10s\n");
	write(log_fd, log_msg, strlen(log_msg));

	while (1) {
		sleep(5);
		ret = ioctl(dog_fd_, WDIOC_KEEPALIVE, NULL);
		time_t now = time(NULL);
		strftime(log_msg, sizeof(log_msg), "running current time %Y-%m-%d %H:%M:%S\n", localtime(&now));
		write(log_fd, log_msg, strlen(log_msg));
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