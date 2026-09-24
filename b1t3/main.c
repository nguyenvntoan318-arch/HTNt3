#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

#define DS1307_ADDR 0xD0 // Ä�á»‹a chá»‰ I2C 7-bit (0x68) dá»‹ch trÃ¡i 1 bit

/* --- HÃ€M Táº O TRá»„ (DELAY TÆ¯Æ NG Ä�á»�I) --- */
void Delay_ms(uint32_t ms) {
    // Vá»›i tháº¡ch anh thÃ´ng thÆ°á»�ng, vÃ²ng láº·p nÃ y táº¡o trá»… tÆ°Æ¡ng Ä‘á»‘i 1ms
    uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 7200; j++); 
    }
}

/* --- Cáº¤U HÃŒNH USART1 Ä�á»‚ Gá»¬I Dá»® LIá»†U LÃŠN MÃ�Y TÃ�NH --- */
void UART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 1. Cáº¥p xung nhá»‹p cho USART1 vÃ  GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 2. Cáº¥u hÃ¬nh chÃ¢n TX (PA9) - Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. Cáº¥u hÃ¬nh chÃ¢n RX (PA10) - Input Floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. Cáº¥u hÃ¬nh thÃ´ng sá»‘ USART: 115200, 8-bit, 1 Stop, No Parity
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    // 5. Cho phÃ©p USART1 hoáº¡t Ä‘á»™ng
    USART_Cmd(USART1, ENABLE);
}

// HÃ m gá»­i chuá»—i kÃ½ tá»± qua UART an toÃ n (thay tháº¿ cho printf trong Makefile)
void UART_SendString(char* str) {
    while (*str) {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Ä�á»£i gá»­i xong
    }
}

/* --- Cáº¤U HÃŒNH I2C1 Ä�á»‚ GIAO TIáº¾P Vá»šI DS1307 --- */
void I2C1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 1. Cáº¥p xung cho I2C1 vÃ  GPIOB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 2. Cáº¥u hÃ¬nh chÃ¢n SCL (PB6) vÃ  SDA (PB7) - Alternate Function Open-Drain
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. Cáº¥u hÃ¬nh thÃ´ng sá»‘ I2C1
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100kHz
    I2C_Init(I2C1, &I2C_InitStructure);

    // 4. Cho phÃ©p I2C hoáº¡t Ä‘á»™ng
    I2C_Cmd(I2C1, ENABLE);
}

/* --- HÃ€M CHUYá»‚N Ä�á»”I BCD VÃ€ DECIMAL --- */
uint8_t dec2bcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

uint8_t bcd2dec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

/* --- CÃ�C HÃ€M GIAO TIáº¾P Vá»šI DS1307 --- */

// CÃ i Ä‘áº·t thá»�i gian cho DS1307
void DS1307_SetTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t date, uint8_t month, uint8_t year) {
    uint8_t data[7] = {dec2bcd(sec), dec2bcd(min), dec2bcd(hour), dec2bcd(day), dec2bcd(date), dec2bcd(month), dec2bcd(year)};
    int i;

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, DS1307_ADDR, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, 0x00); // Trá»� con trá»� vÃ o thanh ghi 0x00 (GiÃ¢y)
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    for(i = 0; i < 7; i++) {
        I2C_SendData(I2C1, data[i]);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
}

// Ä�á»�c thá»�i gian tá»« DS1307
void DS1307_GetTime(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year) {
    uint8_t data[7];
    int i;

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    // BÆ°á»›c 1: BÃ¡o cho DS1307 muá»‘n Ä‘á»�c tá»« thanh ghi 0x00
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, DS1307_ADDR, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    I2C_SendData(I2C1, 0x00);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    // BÆ°á»›c 2: Báº¯t Ä‘áº§u quÃ¡ trÃ¬nh nháº­n dá»¯ liá»‡u
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, DS1307_ADDR, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    for(i = 0; i < 7; i++) {
        if(i == 6) {
            // Byte cuá»‘i cÃ¹ng thÃ¬ ngáº¯t ACK vÃ  gá»­i lá»‡nh STOP
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        } else {
            // CÃ¡c byte trÆ°á»›c Ä‘Ã³ thÃ¬ cho phÃ©p ACK
            I2C_AcknowledgeConfig(I2C1, ENABLE);
        }
        
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
        data[i] = I2C_ReceiveData(I2C1);
    }

    // Chuyá»ƒn Ä‘á»•i dá»¯ liá»‡u vÃ  gáº¯n vÃ o biáº¿n
    *sec   = bcd2dec(data[0] & 0x7F);
    *min   = bcd2dec(data[1]);
    *hour  = bcd2dec(data[2]);
    *day   = bcd2dec(data[3]);
    *date  = bcd2dec(data[4]);
    *month = bcd2dec(data[5]);
    *year  = bcd2dec(data[6]);
}


/* --- CHÆ¯Æ NG TRÃŒNH CHÃ�NH --- */
int main(void) {
    uint8_t sec, min, hour, day, date, month, year;
    char buffer[100];

    // Khá»Ÿi táº¡o cÃ¡c ngoáº¡i vi
    UART1_Init();
    I2C1_Init();

    UART_SendString("\r\n--- HE THONG BAT DAU ---\r\n");

    // ========================================================
    // Náº¾U Máº CH Láº¦N Ä�áº¦U Sá»¬ Dá»¤NG, Bá»Ž COMMENT DÃ’NG DÆ¯á»šI Ä�á»‚ CÃ€I GIá»œ.
    // Sau khi náº¡p code láº§n 1, comment láº¡i vÃ  náº¡p code láº§n 2
    // CÃ i Ä‘áº·t: 09:32:34 Thá»© 7 (NgÃ y 7 trong tuáº§n), NgÃ y 19/09/26
    // ========================================================
    //DS1307_SetTime(00, 30, 10, 7, 19, 9, 26);

    while (1) {
        // Ä�á»�c dá»¯ liá»‡u
        DS1307_GetTime(&sec, &min, &hour, &day, &date, &month, &year);

        // GhÃ©p thÃ nh chuá»—i vÃ  in lÃªn mÃ n hÃ¬nh
        sprintf(buffer, "Thoi gian: %02d:%02d:%02d - Ngay: %02d/%02d/20%02d\r", 
                hour, min, sec, date, month, year);
        UART_SendString(buffer);

        // Nghá»‰ 1 giÃ¢y
        Delay_ms(1000);
    }
}
