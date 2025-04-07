//
// Created by warnie on 25-4-7.
//

#include "ws28122.h"
#include "spi.h"
#include "dma.h"

// 常用的颜色，亮度调的比较低
const RGBColor_TypeDef RED      = {90 ,0  ,  0};
const RGBColor_TypeDef GREEN    = {0  , 90,  0};
const RGBColor_TypeDef BLUE     = {0  ,  0, 90};
const RGBColor_TypeDef YELLOW   = { 90, 90,  0};
const RGBColor_TypeDef MAGENTA  = { 30,  0, 30};
const RGBColor_TypeDef BLACK    = {  0,  0,  0};
const RGBColor_TypeDef WHITE    = { 30, 30, 30};

//模拟bit码:0xC0 为 0,0xF8 为 1
const uint8_t code[]={0xE0,0xF8};
//灯颜色缓存区
RGBColor_TypeDef RGB_DAT[RGB_NUM];

//SPI底层发送接口，一次发24个字节，相当于1个灯
extern DMA_HandleTypeDef hdma_spi3_tx;
static void SPI_Send(uint8_t *SPI_RGB_BUFFER)
{
    /* 判断上次DMA有没有传输完成 */
    while(HAL_DMA_GetState(&hdma_spi3_tx) != HAL_DMA_STATE_READY);
    /* 发送一个(24bit)的 RGB 数据到 2812 */
    HAL_SPI_Transmit_DMA(&hspi3,SPI_RGB_BUFFER,24);
}
//颜色设置函数，传入 ID 和 颜色，进而设置缓存区
void RGB_Set_Color(uint8_t LedId, RGBColor_TypeDef Color)
{
    if(LedId < RGB_NUM)
    {
        RGB_DAT[LedId].G = Color.G;
        RGB_DAT[LedId].R = Color.R;
        RGB_DAT[LedId].B = Color.B;
    }
}
//刷新函数，将颜色缓存区刷新到WS2812，输入参数是指定的刷新长度
void RGB_Reflash(uint8_t reflash_num)
{
    static uint8_t RGB_BUFFER[24]={0};
    uint8_t dat_b,dat_r,dat_g;
    //将数组颜色转化为 24 个要发送的字节数据
    if(reflash_num>0 && reflash_num<=RGB_NUM)
    {
        for(int i=0;i<reflash_num;i++)
        {
            dat_g = RGB_DAT[i].G;
            dat_r = RGB_DAT[i].R;
            dat_b = RGB_DAT[i].B;
            for(int j=0;j<8;j++)
            {
                RGB_BUFFER[7-j] =code[dat_g & 0x01];
                RGB_BUFFER[15-j]=code[dat_r & 0x01];
                RGB_BUFFER[23-j]=code[dat_b & 0x01];
                dat_g >>=1;
                dat_r >>=1;
                dat_b >>=1;
            }
            SPI_Send(RGB_BUFFER);
        }
    }
}
//复位函数
void RGB_RST(void)
{
    uint8_t dat[100] = {0};
    /* 判断上次DMA有没有传输完成 */
    while(HAL_DMA_GetState(&hdma_spi3_tx) != HAL_DMA_STATE_READY);
    /* RGB RESET */
    HAL_SPI_Transmit_DMA(&hspi3,dat,100);
    HAL_Delay(10);
}

//红色点亮测试
void RGB_RED(uint16_t RGB_LEN)     //RGB_LEN是指定灯珠数量;RGB_NUM是总灯珠数量;
{
    uint8_t i;
    for(i=0;i<RGB_LEN;i++)
        RGB_Set_Color(i,RED);
    RGB_Reflash(RGB_LEN);
}
// 绿色点亮测试
void RGB_GREEN(uint16_t RGB_LEN)
{
    uint8_t i;
    for(i=0;i<RGB_LEN;i++)
        RGB_Set_Color(i,GREEN);
    RGB_Reflash(RGB_LEN);
}
//黑色点亮测试
void RGB_BLACK(uint16_t RGB_LEN)
{
    uint8_t i;
    for(i=0;i<RGB_LEN;i++)
        RGB_Set_Color(i,BLACK);
    RGB_Reflash(RGB_LEN);
}
//红色闪烁2S，绿色闪烁2S,关闭灯珠;
void test_ws2812()
{
    RGB_RED(24);
    HAL_Delay(2000);
    RGB_GREEN(24);
    HAL_Delay(2000);
    RGB_BLACK(24);
}
//.........其他颜色按需求添加.........//

