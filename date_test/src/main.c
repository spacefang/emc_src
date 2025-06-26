#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

void TimeSet(int year, int month, int day, int hour, int min, int sec);


int main(int argc, char* argv[])
{
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;
	printf("before time set");
	fflush(stdout);
	system("date");

	year = atoi(argv[1]);
	month = atoi(argv[2]);
	day = atoi(argv[3]);
	hour = atoi(argv[4]);
	min = atoi(argv[5]);
	sec = atoi(argv[6]);

	TimeSet(year, month, day, hour, min, sec);

	printf("after time set");
	fflush(stdout);
	system("date");
	return 0;
}


void TimeSet(int year, int month, int day, int hour, int min, int sec)
{
	struct tm tptr;
	struct timeval tv;

	tptr.tm_year = year - 1900;
	tptr.tm_mon = month - 1;
	tptr.tm_mday = day;
	tptr.tm_hour = hour;
	tptr.tm_min = min;
	tptr.tm_sec = sec;

	tptr.tm_isdst = 0;//必须设置，不设置mktime大概率返回-1

	tv.tv_sec = mktime(&tptr);
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);

}