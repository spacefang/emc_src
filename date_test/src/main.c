#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char *argv[]) {
    struct tm time_tm;
    struct timeval time_tv;

    // 健壮性检查：确保收到了正确数量的命令行参数
    if (argc != 7) {
        fprintf(stderr, "Usage: %s YYYY MM DD HH mm SS\n", argv[0]);
        fprintf(stderr, "Error: Incorrect number of arguments provided.\n");
        return 1; // 返回错误码
    }

    // 将字符串参数转换为整数
    memset(&time_tm, 0, sizeof(struct tm));
    time_tm.tm_year = atoi(argv[1]) - 1900; // 年份需要减去1900
    time_tm.tm_mon  = atoi(argv[2]) - 1;    // 月份范围是0-11
    time_tm.tm_mday = atoi(argv[3]);
    time_tm.tm_hour = atoi(argv[4]);
    time_tm.tm_min  = atoi(argv[5]);
    time_tm.tm_sec  = atoi(argv[6]);
    time_tm.tm_isdst = -1; // 让系统自动判断夏令时

    printf("Attempting to set time to: %s-%s-%s %s:%s:%s\n",
           argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

    // 将tm结构体转换为time_t时间戳
    time_t t = mktime(&time_tm);
    if (t == -1) {
        fprintf(stderr, "Error: mktime failed. The provided time might be invalid.\n");
        return 1;
    }

    time_tv.tv_sec = t;
    time_tv.tv_usec = 0;

    // 执行最终的时间设置
    if (settimeofday(&time_tv, NULL) != 0) {
        perror("settimeofday failed");
        return 1;
    }

    printf("System time successfully set.\n");
    system("date"); // 打印设置后的当前时间以供验证

    return 0;
}