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
#define CAR_ICON     0x4
#define CAR_ICON_HIGH_SPEED 0x5
#define CAR_ICON_LOW_SPEED 0x6
#define CAR_ICON_CROSS_WALL 0x7
#define LOW_SPEED_ICON 0x1
#define HIGH_SPEED_ICON 0x2
#define CROSS_WALL_ICON 0x3

unsigned long map         = 0;
unsigned long carPosition = 0b01;    
unsigned long typeOb1     = 0;   // xuyên tường
unsigned long typeOb2     = 0;   // chậm
unsigned long typeOb3     = 0 ;  // tăng tốc                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         

char time1Count           = 0;
char timeCount            = 0;
char score                = 0;
char soundFreq            = 5;
char soundEnable          = 1;
char isPlaying            = 1;
char speed                = 20; // tốc độ chơi game
char timeSpeed            = 0;
char carIcon              = CAR_ICON;
// char carIconHighSpeed     = 0x5;
// char carIconLowSpeed      = 0x6;
// char carIconCrossWall     = 0x7;


void printInt(int num, char* dest, int destIndex)
{
    int cIndex = destIndex;
    while(num && cIndex < 16) {
        dest[cIndex] = num % 10 + '0';
        num /= 10;
        cIndex++;
    }
    char tmp;
    while(destIndex < cIndex) {
        tmp = dest[destIndex];
        dest[destIndex] = dest[cIndex];
        dest[cIndex] = tmp;
        destIndex++;
        cIndex--;
    }
}

long randomNewObstacle()
{
    // return 0b1000000000000000000000000000000;
    // cứ 2 bit cho 1 cột, tính từ bên phải sang (0 -> 31)
    // Số thứ tự cột thì từ trái sang phải (1 -> 16)
    // số thứ tự hàng từ trên xuống dưới (1 -> 2)
    unsigned char lastMap = map >> 30;
    if (lastMap == 0) {
        unsigned long _2ndLastMap = ((map << 2) & 0b11000000000000000000000000000000) ^ 0b11000000000000000000000000000000;
        return _2ndLastMap; 
        // sinh ra 1 chướng ngại vật chéo với cái gần nhất
    }

    char r = rand() & 1;
    if (r) {
        return 0; // sinh ra cột trống -> tiếp theo sinh cột chéo
    } else {
        return map & 0b11000000000000000000000000000000; // sinh ra chướng ngại vật thẳng vs cái gần nhất
    }
}

char handleImpact()
{
    return map & carPosition;
}

char handleImpactTypeOb1()
{
    return typeOb1 & carPosition;
}

char handleImpactTypeOb2()
{
    return typeOb2 & carPosition;
}

char handleImpactTypeOb3()
{
    return typeOb3 & carPosition;
}

void displayFailure()
{
    isPlaying = 0;
    soundFreq = 1;
    TR0       = 0;
    LCD_Send_Command(0x80);
    LCD_Write_String("FAILURE         ");
    LCD_Send_Command(0xC0);
    char scoreStr[] = "Score:          ";
    // sprintf(scoreStr, "Score: %d", score);
    printInt(score, scoreStr, 7);

    LCD_Write_String(scoreStr);
}

char moveMapForward()
{
    unsigned long newMap = randomNewObstacle();
    char type= rand()%6+1;
    map >>= 2;
    typeOb1>>=2;
    typeOb2>>=2;
    typeOb3>>=2;
    if (newMap!=0){
        if (type==1){
            typeOb1|=newMap;
        }
        else if (type==2){
            typeOb2|=newMap;
        }
        else if (type==3){
            typeOb3|=newMap;
        }
    }


    if  (handleImpactTypeOb1()){
        if (carPosition==0b01){
            for (char i=2; i<16; i+=2)
            {
                if (map>>i & 0b1){
                    map &= ~(1UL << i);
                    // typeOb1 &= ~(1UL << i);
                    break;
                }
            }
        }
        else{
            for (char i=3; i<16; i+=2)
            {
                if (map>>i & 0b1){
                    map &= ~(1UL << i);
                    // typeOb1 &= ~(1UL << i);
                    break;
                }
            }
        }
    }
    else if (handleImpactTypeOb2())
    {
        speed=25;
        timeSpeed=12; // thời gian chậm là 12 block
        carIcon= 'v';

    }
    else if (handleImpactTypeOb3())
    {
        speed=15;
        timeSpeed=12; // thời gian tăng tốc là 12 block
        carIcon= '^';

    }
    
    else if (handleImpact()) {
        displayFailure();
        return 0;
    }
    map |= newMap;
    if (timeSpeed!=0){
        timeSpeed--;
    }
    else{
        speed=20;
        carIcon=CAR_ICON;
    }

    score++;
    return 1;
}

void initMap()
{
    map = 0b10000000000000000000000000000000;
    for (char i = 0; i < 15; i++) {
        unsigned long newMap = randomNewObstacle();
        char type= rand()%6+1;
        typeOb1>>=2;
        typeOb2>>=2;
        typeOb3>>=2;
        map >>= 2;
        if (type==1 && newMap!=0){
            typeOb1|=newMap;
        }
        if (type==2 && newMap!=0){
            typeOb2|=newMap;
        }
        if (type==3 && newMap!=0){

            typeOb3|=newMap;
        }
        map |= newMap;

    }
}

void renderMap() // in màn hình
{
    unsigned char line[16];
    unsigned char isBlock = 0;
    unsigned char isType1 = 0;
    unsigned char isType2 = 0;
    unsigned char isType3 = 0;
    LCD_Send_Command(0x80);
    // các bit ở index 0, 2, 4, 2n,... sẽ được dùng để
    // sinh ra chướng ngại vật cho hàng 1
    for (char i = 0; i < 16; i++) {
        isBlock = (map >> (i * 2)) & 0b1;
        isType1 = (typeOb1 >> (i * 2)) & 0b1;
        isType2 = (typeOb2 >> (i * 2)) & 0b1;
        isType3 = (typeOb3 >> (i * 2)) & 0b1;

        if (isType1){
            line[i] = CROSS_WALL_ICON;
            continue;
        }
        else if (isType2){
            line[i] = LOW_SPEED_ICON;
            continue;
        }
        else if (isType3){
            line[i] = HIGH_SPEED_ICON;
            continue;
        }
        else if (isBlock){
            line[i] = 0xFF;
        }
        
        else {
            line[i] = ' ';
        }
    }

    // SINH ra vị trí xe ở hàng 1
    if (carPosition == 0b01) {
        line[0] = carIcon;
    }
    LCD_Write_String(line);

    // các bit ở index 1, 3, 5, 2n + 1,... sẽ được dùng để
    // sinh ra chướng ngại vật cho hàng 2
    LCD_Send_Command(0xC0);
    for (char i = 0; i < 16; i++) {
        isBlock = (map >> (i * 2 + 1)) & 0b1;
        isType1 = (typeOb1 >> (i * 2 + 1)) & 0b1;
        isType2 = (typeOb2 >> (i * 2 + 1)) & 0b1;
        isType3 = (typeOb3 >> (i * 2 + 1)) & 0b1;
            // line[i] = 0xFF;
        if (isType1){
            line[i] = CROSS_WALL_ICON;
            continue;
        }
        else if (isType2){
            line[i] = LOW_SPEED_ICON;
            continue;
        }
        else if (isType3){
            line[i] = HIGH_SPEED_ICON;
            continue;
        }
        else if (isBlock){
            line[i] = 0xFF;
        }
        else {
            line[i] = ' ';
        }
    }
    // SINH ra vị trí xe ở hàng 1
    if (carPosition == 0b10) {
        line[0] = carIcon;
    }
    LCD_Write_String(line);
}

void initObjectsIcon()
{
    // Khởi tạo icon cho xe
    unsigned char customChar[] = {
        0b00000,
        0b00000,
        0b11100,
        0b11111,
        0b11111,
        0b11100,
        0b00000,
        0b00000
    };
    LCD_Create_Char(CAR_ICON, customChar);

    // Khởi tạo icon cho object xuyên tường
    customChar[0] = 0b00000;
    customChar[1] = 0b10001;
    customChar[1] = 0b10101;
    customChar[3] = 0b11111;
    customChar[4] = 0b11111;
    customChar[5] = 0b10101;
    customChar[6] = 0b10001;
    customChar[7] = 0b00000;
    LCD_Create_Char(CROSS_WALL_ICON, customChar);

    // Khởi tạo icon cho object làm chậm
    customChar[0] = 0b00010;
    customChar[1] = 0b00100;
    customChar[2] = 0b01001;
    customChar[3] = 0b11010;
    customChar[4] = 0b11010;
    customChar[5] = 0b01001;
    customChar[6] = 0b00100;
    customChar[7] = 0b00010;
    LCD_Create_Char(LOW_SPEED_ICON, customChar);

    // Khởi tạo icon cho object tăng tốc
    customChar[0] = 0b01000;
    customChar[1] = 0b00100;
    customChar[2] = 0b10010;
    customChar[3] = 0b01011;
    customChar[4] = 0b01011;
    customChar[5] = 0b10010;
    customChar[6] = 0b00100;
    customChar[7] = 0b01000;
    LCD_Create_Char(HIGH_SPEED_ICON, customChar);

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

    // Khởi tạo icon cho xe và các object
    initObjectsIcon();

    // displayFailure();

    // // Khởi tạo map
    initMap();
    renderMap();
    LCD_Send_Command(LCD_RETURNHOME);

    // Chờ 3s
    Delay_ms(300);

    // // Đặt thanh ghi TR0 để bắt đầu chạy timer 0
    TR0 = 1; // map
    TR1 = 1; // loa



    while (1) {
        if (isPlaying) {
            // Nhấn phím S1 để rẽ trái
            // Nhấn phím S4 để rẽ phải
            // P1_7 = 1;
            P1_6 = 0;
            // P1_5 = 1;
            // P1_4 = 1;

            if (P1_3 == 0) { // bên phải: đi lên
                carPosition = 0b01;
                if (handleImpact()) { // nếu đang đi mà bên trên có vật cản thì ko đi lên đc
                    carPosition = 0b10;
                }
            }
            if (P1_0 == 0) { // bên trái: đi xuống
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
    if (timeCount == speed) {
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