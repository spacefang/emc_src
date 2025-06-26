

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

int write_file(const char* filename, unsigned int* pbuf, int len)
{
    FILE* file = fopen(filename, "wb");
    int i = 0;
    int file_size = len;
    if (!file)
    {
        perror("Failed to open file");
        return -1;
    }

    for (i = 0; i < file_size - 1; i++)
    {

        pbuf[i] = i;
    }

    pbuf[file_size - 1] = crc32((const unsigned char*)pbuf, (file_size - 1) * 4);

    fwrite(pbuf, sizeof(unsigned int), file_size, file);
    fclose(file);

    return 0;
}

int read_file(const char* filename, unsigned int* pbuf, int len)
{
    FILE* file = fopen(filename, "rb");
    int i = 0;
    int file_size = len;
    if (!file)
    {
        perror("Failed to open file");
        return -1;
    }
    for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)
    {
        pbuf[i] = 0;
    }

    fread(pbuf, sizeof(unsigned int), file_size, file);
    fclose(file);
    return 0;
}



int main(int argc, char* argv[])
{
    const char* filename = "/dev/sda";
   
    int no = atoi(argv[1]);
    char logfilename[64] = "";
    FILE* log_file = NULL;
    int i = 0;
    int cnt = 0;
    int loopcnt = 0;
    unsigned int crc = 0;
    bool bfail = 0;
    int file_size = FILE_SIZE;
    char timbuffer[64] = "";

    get_cur_system_time_filename(logfilename, sizeof(logfilename), no);
    log_file = fopen(logfilename, "a");
    init_crc32_table();
    while (1)
    {
        bfail = 0;
        loopcnt++;
        //get_cur_system_time(timbuffer, sizeof(timbuffer));
        //printf("[%s] start:loopcnt = %d\n", timbuffer, loopcnt);
        write_file(filename, filebuf, FILE_SIZE);

        for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)
        {
            filebuf[i] = 0;
        }

        read_file(filename, filebuf, FILE_SIZE);

        if (loopcnt % 10 == 0)
        {
            //filebuf[loopcnt% FILE_SIZE] = 0;
        }
        crc = crc32((const unsigned char*)filebuf, (file_size - 1) * 4);

        if (crc != filebuf[file_size - 1])
        {
            bfail = 1;
        }

        if (bfail == 0)
        {
            for (i = 0; i < file_size - 1; i = i + FILE_SIZE_STEP)
            {

                if (filebuf[i] != i)
                {
                    bfail = 1;
                    break;
                }
            }

        }

        if (bfail == 1)
        {
            cnt++;
            get_cur_system_time(timbuffer, sizeof(timbuffer));
            printf("[%s]crc bad cnt = %d\n", timbuffer, cnt);
            fprintf(log_file, "[%s]crc bad cnt = %d\n", timbuffer, cnt);
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