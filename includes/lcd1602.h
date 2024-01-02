#ifndef LCD1602_H_INCLUDED
#define LCD1602_H_INCLUDED

//LCD in 8 bit interface mode
#define LCD_DATA    P0
#define LCD_EN          P2_7
#define LCD_RS          P2_6
#define LCD_RW          P2_5

void LCD_init();
void Wait_For_LCD();
void LCD_Send_Command(unsigned char x);
void LCD_Write_One_Char(unsigned char c);
void LCD_Write_String(unsigned char *s);
void LCD_GotoXY(char row, char col);            //row: line number (1 or 2)
void Delay_ms(int interval);
void Delay_us(int interval);

#endif // LCD1602_H_INCLUDED
