#include <mcs51/8051.h>
#include "lcd1602.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#define SPEAKER      P1_5
#define LED7SEG_DATA P0
#define TH0_50ms     0x4C
#define TL0_50ms     0x00
#define TH1_ms       0xDB // 10ms
#define TL1_ms       0xFF

unsigned long map         = 0;
unsigned long carPosition = 0b01;
int timeCount             = 0;
int time1Count            = 0;
int score                 = 0;
char soundFreq            = 5;
char soundEnable          = 1;
char isPlaying            = 1;

long randomNewObstacle()
{
    // return 0b1000000000000000000000000000000;
    unsigned char lastMap = map >> 30;
    if (lastMap == 0) {
        unsigned long _2ndLastMap = ((map << 2) & 0b11000000000000000000000000000000) ^ 0b11000000000000000000000000000000;
        return _2ndLastMap;
    }

    int r = rand() & 1;
    if (r) {
        return 0;
    } else {
        return map & 0b11000000000000000000000000000000;
    }
}

char handleImpact()
{
    return map & carPosition;
}

void displayFailure()
{
    isPlaying = 0;
    soundFreq = 1;
    TR0       = 0;
    LCD_Send_Command(0x80);
    LCD_Write_String("FAILURE        ");
    LCD_Send_Command(0xC0);
    char scoreStr[16];
    sprintf(scoreStr, "Score: %d", score);
    int i = 0;
    while (scoreStr[i] != '\0') {
        i++;
    }

    for (; i < 16; i++) {
        scoreStr[i] = ' ';
    }

    LCD_Write_String(scoreStr);
}

char moveMapForward()
{
    unsigned long newMap = randomNewObstacle();
    map >>= 2;
    if (handleImpact()) {
        displayFailure();
        return 0;
    }
    map |= newMap;
    score++;
    return 1;
}

void initMap()
{
    map = 0b10000000000000000000000000000000;
    for (int i = 0; i < 15; i++) {
        unsigned long newMap = randomNewObstacle();
        map >>= 2;
        map |= newMap;
    }
}

void renderMap()
{
    unsigned char line[16];
    unsigned char isBlock = 0;
    LCD_Send_Command(0x80);
    for (int i = 0; i < 16; i++) {
        isBlock = (map >> (i * 2)) & 0b1;
        if (isBlock) {
            line[i] = 0xFF;
        } else {
            line[i] = ' ';
        }
    }
    if (carPosition == 0b01) {
        line[0] = '>';
    }
    LCD_Write_String(line);
    LCD_Send_Command(0xC0);
    for (int i = 0; i < 16; i++) {
        isBlock = (map >> (i * 2 + 1)) & 0b1;
        if (isBlock) {
            line[i] = 0xFF;
        } else {
            line[i] = ' ';
        }
    }
    if (carPosition == 0b10) {
        line[0] = '>';
    }
    LCD_Write_String(line);
}

int main()
{
    // Tắt tất cả các LED 7 đoạn
    LED7SEG_DATA = 0x00;

    // Đặt thanh ghi TMOD để chọn mode 1 cho timer 0
    TMOD = 0x11; // timer 0 mode 1

    // Đặt thanh ghi TH0 và TL0 để ngắt xảy ra sau 50ms
    // Khi nào TH0 = FF và TL0 = FF thì sẽ bật flag TF0
    TH0 = TH0_50ms;
    TL0 = TL0_50ms;

    TH1 = TH1_ms;
    TL1 = TL1_ms;

    // Đặt thanh ghi IE để cho phép ngắt timer 0 và ngắt ngoài
    // IE bit 7: EA (Enable All interrupt)
    // IE bit 6, 5: Reserved bit
    // IE bit 4: ES (Enable Serial interrupt)
    // IE bit 3: ET1 (Enable Timer 1 interrupt)
    // IE bit 2: EX1 (Enable External interrupt 1)
    // IE bit 1: ET0 (Enable Timer 0 interrupt)
    // IE bit 0: EX0 (Enable External interrupt 0)
    IE = 0b10001010;

    // Đặt thanh ghi TF0 để xóa cờ ngắt timer 0
    TF0 = 0;
    TF1 = 0;

    // Khởi tạo LCD
    LCD_init();

    // displayFailure();

    // // Khởi tạo map
    initMap();
    renderMap();

    // Chờ 3s
    Delay_ms(300);

    // // Đặt thanh ghi TR0 để bắt đầu chạy timer 0
    TR0 = 1;
    TR1 = 1;

    while (1) {
        if (isPlaying) {
            // Nhấn phím S1 để rẽ trái
            // Nhấn phím S4 để rẽ phải
            // P1_7 = 1;
            // P1_6 = 0;
            // P1_5 = 1;
            P1_4 = 1;

            if (P1_3 == 0) {
                carPosition = 0b01;
                if (handleImpact()) {
                    carPosition = 0b10;
                }
            }
            if (P1_0 == 0) {
                carPosition = 0b10;
                if (handleImpact()) {
                    carPosition = 0b01;
                }
            }

            // if (handleImpact()) {
            //     displayFailure();
            // } else
            renderMap();

            Delay_us(30000);
        }
    }
    // return 0;
}

// Ngắt xảy ra sau 50ms
void TIMER0_ISR() __interrupt TF0_VECTOR
{
    // Đặt lại giá trị cho thanh ghi TH0 và TL0 để ngắt xảy ra sau 50ms
    TH0 = TH0_50ms;
    TL0 = TL0_50ms;

    timeCount++;
    if (timeCount == 20) {
        timeCount = 0;

        // Bật tắt LED 1 sau mỗi 0.5
        if (moveMapForward()) {
            renderMap();
        }
    }
}

// Ngắt xảy ra sau 50ms
void TIMER1_ISR() __interrupt TF1_VECTOR
{
    // Đặt lại giá trị cho thanh ghi TH0 và TL0 để ngắt xảy ra sau 50ms
    TH1 = TH1_ms;
    TL1 = TL1_ms;

    time1Count++;
    if (time1Count >= soundFreq) {
        time1Count = 0;

        if (soundEnable) {
            SPEAKER = !SPEAKER;
        }
    }
}