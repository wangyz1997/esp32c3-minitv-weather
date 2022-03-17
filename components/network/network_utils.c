#include "network_utils.h"

/**
 * @brief 打印缓冲区内的数据
 * 
 * @param buffer 要打印的缓冲区
 * @param length 缓冲区长度
 * @param width 打印时每行显示的字节数
 */
void print_content(char *buffer, size_t length, uint8_t width)
{
    size_t row = 0;
    while(row*width < length) {
        printf("%04X  ", row * width);
        for(size_t col = 0; col < width; col ++) { //打印16进制
            if(col + row*width >= length) {
                for(size_t cnt = col; cnt < width; cnt ++) {
                    printf("   "); //补齐剩余空格
                }
                break;
            }
            printf("%02X ", buffer[col + row*width]);
        }
        printf(" ");
        for(size_t col = 0; col < width; col ++) { //打印字符
            if(col + row*width >= length) {
                printf("\n");
                return;
            }
            char c = buffer[col + row*width];
            if(c > '~' || c < ' ') {
                c = '.';
            }
            printf("%c", c);
        }
        printf("\n");
        row ++;
    }
}
