//
//Creat By HuangKai
//中景园电子
//
#include "oled.h"
#include "oledfont.h"
#include "main.h"

/*
 * OLED的显存
 * 存放格式如下
 * [0] 0 1 2 3 ... 127
 * [1] 0 1 2 3 ... 127
 * [2] 0 1 2 3 ... 127
 * [3] 0 1 2 3 ... 127
 * [4] 0 1 2 3 ... 127
 * [5] 0 1 2 3 ... 127
 * [6] 0 1 2 3 ... 127
 * [7] 0 1 2 3 ... 127
 * * */

#if OLED_MODE == 1
/* @brief   向SSD1106写入一个字节
 * @param   dat 要写入的数据/命令
 *          cmd 数据/命令标志 0:命令    1:数据
 * @note    四线串行模式写入*/
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    DATAOUT(dat);
    if(cmd)
      OLED_DC_Set();
    else
      OLED_DC_Clr();
    OLED_CS_Clr();
    OLED_WR_Clr();
    OLED_WR_Set();
    OLED_CS_Set();
    OLED_DC_Set();
}
#else

/* @brief   向SSD1106写入一个字节
 * @param   dat 要写入的数据/命令
 *          cmd 数据/命令标志 0:命令    1:数据
 * @note    并行0808模式写入*/
void OLED_WR_Byte(uint8_t dat, uint8_t cmd) {
    uint8_t i;
    if (cmd)
        OLED_DC_Set();
    else
        OLED_DC_Clr();
    OLED_CS_Clr();
    for (i = 0; i < 8; i++) {
        OLED_SCLK_Clr();
        if (dat & 0x80)
            OLED_SDIN_Set();
        else
            OLED_SDIN_Clr();
        OLED_SCLK_Set();
        dat <<= 1;
    }
    OLED_CS_Set();
    OLED_DC_Set();
}

#endif

void OLED_Set_Pos(unsigned char x, unsigned char y) {
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}

/* @brief   开启OLED显示*/
void OLED_Display_On(void) {
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC????
    OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

/* @brief   关闭OLED显示*/
void OLED_Display_Off(void) {
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC????
    OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}

/* @brief   用于清屏
 * @note    使用后整个屏幕是黑的,和没电亮一样*/
void OLED_Clear(void) {
    uint8_t i, n;
    for (i = 0; i < 8; i++) {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);    //??????????0~7??
        OLED_WR_Byte(0x00, OLED_CMD);      //???????λ?á??е???
        OLED_WR_Byte(0x10, OLED_CMD);      //???????λ?á??и???
        for (n = 0; n < 128; n++)OLED_WR_Byte(0, OLED_DATA);
    } //???????
}

/* @brief   在指定位置显示一个字符
 * @param   x   屏幕x坐标
 *          y   屏幕y坐标
 *          chr 要显示的字符
 * @note    SIZE表示显示的字体,有16和12两种字体选择*/
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr) {
    unsigned char c = 0, i = 0;
    c = chr - ' ';//?????????
    if (x > Max_Column - 1) {
        x = 0;
        y = y + 2;
    }
    if (SIZE == 16) {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
    } else {
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 6; i++)
            OLED_WR_Byte(F6x8[c][i], OLED_DATA);

    }
}

/* @brief   幂指数显示m^n*/
uint32_t oled_pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while (n--)result *= m;
    return result;
}

/* @brief   显示数字
 * @param   x       起点位置x坐标
 *          y       起点位置y坐标
 *          num     要显示的数字(最多可显示32位数字   2^32=4294967295)
 *          len     设定屏幕用几位来显示该数字
 *          size    设定字体大小*/
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x + (size / 2) * t, y, ' ');
                continue;
            } else enshow = 1;

        }
        OLED_ShowChar(x + (size / 2) * t, y, temp + '0');
    }
}

/* @brief   显示一个字符串
 * param    x,y 起始位置
 *          chr 字符串的首地址,字符串也是一个带有结束符的字符数组*/
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr) {
    unsigned char j = 0;
    while (chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j]);
        x += 8;
        if (x > 120) {
            x = 0;
            y += 2;
        }
        j++;
    }
}

/* @brief   显示汉字
 * @param   x,y 起始位置
 *          no  要显示的汉字,填入的是要显示的汉字的编号
 * @note    汉字要事先取模后才能显示,本质上就是显示一幅图片*/
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no) {
    uint8_t t, adder = 0;
    OLED_Set_Pos(x, y);
    for (t = 0; t < 16; t++) {
        OLED_WR_Byte(Hzk[2 * no][t], OLED_DATA);
        adder += 1;
    }
    OLED_Set_Pos(x, y + 1);
    for (t = 0; t < 16; t++) {
        OLED_WR_Byte(Hzk[2 * no + 1][t], OLED_DATA);
        adder += 1;
    }
}

/* @brief   显示BMP图片z最大尺寸128*64
 * @param   (x0,y0)图片左上角坐标
 *          (x1,y1)图片右下角坐标
 *          BMP[]图片取模生成的数组*/
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]) {
    unsigned int j = 0;
    unsigned char x, y;

    if (y1 % 8 == 0) y = y1 / 8;
    else y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++) {
            OLED_WR_Byte(BMP[j++], OLED_DATA);
        }
    }
}


/* @brief   SSD1306初始化,也就是初始化OLED
 * @note    只有执行初始化函数后,其他函数才能使用*/
void OLED_Init(void) {

    OLED_RST_Set();
    HAL_Delay(10);
    OLED_RST_Clr();
    HAL_Delay(10);
    OLED_RST_Set();

    OLED_WR_Byte(0xAE, OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00, OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10, OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40, OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81, OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1, OLED_CMD);//--Set SEG/Column Mapping     0xa0??????? 0xa1????
    OLED_WR_Byte(0xC8, OLED_CMD);//Set COM/Row Scan Direction   0xc0???·??? 0xc8????
    OLED_WR_Byte(0xA6, OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8, OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f, OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3, OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00, OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5, OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80, OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9, OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1, OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA, OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40, OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20, OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02, OLED_CMD);//
    OLED_WR_Byte(0x8D, OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14, OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4, OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6, OLED_CMD);// Disable Inverse Display On (0xa6/a7)
    OLED_WR_Byte(0xAF, OLED_CMD);//--turn on oled panel

    OLED_WR_Byte(0xAF, OLED_CMD); /*display ON*/
    OLED_Clear();
    OLED_Set_Pos(0, 0);
}  





























