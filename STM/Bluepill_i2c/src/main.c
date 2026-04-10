#include <stdint.h>
#include "stm32f1xx.h"
#include <math.h>

volatile int buttonState = 0;
volatile uint32_t dutyCycle = 0;
volatile int16_t AX, AY, AZ, Tmp, GX, GY, GZ;
volatile float AX_f, AY_f, AZ_f, Tmp_f, GX_f, GY_f, GZ_f;
volatile float roll;

volatile uint32_t msTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
}
void delay(uint32_t ms) {
    uint32_t start = msTicks;
    while (msTicks - start < ms) {
        __WFI();
    }
}

void I2C_start (uint8_t address, uint8_t isRead) {
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));


    if (isRead) {
        I2C1->DR = (address << 1) | 1;
    } else {
        I2C1->DR = address << 1;
    }

    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2;

}

void I2C_write(uint8_t data) {
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;
    while(!(I2C1->SR1 & I2C_SR1_BTF));

}

uint16_t I2C_read() {
    uint8_t data;

    I2C1->CR1 |= I2C_CR1_ACK;

    while(!(I2C1->SR1 & I2C_SR1_RXNE));
    data = I2C1->DR;

    return data;
}

uint8_t I2C_read_nack() {
    uint8_t data;

    I2C1->CR1 &= ~I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_STOP;

    while(!(I2C1->SR1 & I2C_SR1_RXNE));
    data = I2C1->DR;

    return data;
}

uint8_t I2C_stop(void) {
    I2C1->CR1 |= I2C_CR1_STOP;
    while(!(I2C1->SR2 & I2C_SR2_MSL));
    return 0;
}

int main (void) {
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    RCC->CFGR &= ~(RCC_CFGR_PLLSRC_Msk | RCC_CFGR_PLLXTPRE_Msk | RCC_CFGR_PLLMULL_Msk);
    RCC->CFGR |= 1 << 16 | RCC_CFGR_PLLMULL9;
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL);

    SysTick_Config(72000000 / 1000);

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->CRL &= ~(GPIO_CRL_MODE6_Msk | GPIO_CRL_CNF6_Msk | GPIO_CRL_MODE7_Msk | GPIO_CRL_CNF7_Msk);
    GPIOB->CRL |= (3 << GPIO_CRL_CNF6_Pos) | (3 << GPIO_CRL_CNF7_Pos) | (2 << GPIO_CRL_MODE6_Pos) | (2 << GPIO_CRL_MODE7_Pos);

    GPIOC->CRH &= ~(GPIO_CRH_MODE13_Msk | GPIO_CRH_CNF13_Msk);
    GPIOC->CRH |= 1 << GPIO_CRH_MODE13_Pos;

    
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 |= 36 << I2C_CR2_FREQ_Pos;
    I2C1->CCR |= 180 << I2C_CCR_CCR_Pos;
    I2C1->TRISE |= (36 + 1) << I2C_TRISE_TRISE_Pos;

    I2C1->CR1 |= I2C_CR1_PE;


    I2C_start(0x68, 0);
    I2C_write(27);
    I2C_write(0b00010000);
    I2C_stop();

    I2C_start(0x68, 0);
    I2C_write(28);
    I2C_write(0b00010000);
    I2C_stop();


    while (1) {

        I2C_start(0x68, 0);
        I2C_write(59);
        I2C_stop();
        I2C_start(0x68, 1);

        AX = I2C_read(0x68) << 8 | I2C_read(0x68);
        AY = I2C_read(0x68) << 8 | I2C_read(0x68);
        AZ = I2C_read(0x68) << 8 | I2C_read(0x68);
        Tmp = I2C_read(0x68) << 8 | I2C_read(0x68);
        GX = I2C_read(0x68) << 8 | I2C_read(0x68);
        GY = I2C_read(0x68) << 8 | I2C_read(0x68);
        GZ = I2C_read(0x68) << 8 | I2C_read_nack(0x68);


        AX_f = AX / 4096.0;
        AY_f = AY / 4096.0;
        AZ_f = AZ / 4096.0;
        Tmp_f = (Tmp / 340.00) + 36.53;
        GX_f = GX / 32.8;
        GY_f = GY / 32.8;
        GZ_f = GZ / 32.8;

        roll = atan2(AX_f, AZ_f) * 180 / 3.1415;


        delay(100);

    }
}