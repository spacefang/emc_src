#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

// CRC8生成多项式
#define POLYNOMIAL 0x07
// x ^ 8 + x ^ 2 + x ^ 1 + x ^ 0

// 初始化CRC8查找表
uint8_t crc8_table[256];

void init_crc8_table(void)
{
    uint32_t i, j;
    for (i = 0; i < 256; i++)
    {
        uint8_t crc = i;
        for (j = 8; j > 0; j--)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc <<= 1;
        }
        crc8_table[i] = crc;
    }
}

uint8_t crc8(const uint8_t* data, size_t len)
{
    const uint8_t* byte = data;
    uint8_t crc = 0x00;

    for (; len > 0; len--)
    {
        crc = crc8_table[(crc ^ *byte++) & 0xFF];
    }

    return crc;
}


