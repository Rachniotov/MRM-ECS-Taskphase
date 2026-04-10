#include <stdint.h>
#include "stm32f1xx.h"
#include <math.h>

volatile uint32_t dutyCycle = 0;
volatile uint32_t val = 0;

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

    
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    

    GPIOA->CRL &= ~(GPIO_CRL_MODE2_Msk | GPIO_CRL_CNF2_Msk);

    GPIOC->CRH &= ~(GPIO_CRH_MODE13_Msk | GPIO_CRH_CNF13_Msk);
    GPIOC->CRH |= 1 << GPIO_CRH_MODE13_Pos;

    DMA1_Channel1->CPAR = (uint32_t) &ADC1->DR;
    DMA1_Channel1->CMAR = (uint32_t) &val;
    DMA1_Channel1->CNDTR = 1;

    DMA1_Channel1->CCR |= 1 << DMA_CCR_MSIZE_Pos | 1 << DMA_CCR_PSIZE_Pos | DMA_CCR_CIRC | DMA_CCR_EN;

    ADC1->SQR3 = 2;
    ADC1->CR2 |= ADC_CR2_DMA;
    ADC1->SMPR2 |= 7 << ADC_SMPR2_SMP2_Pos;
    ADC1->CR2 |= ADC_CR2_ADON;

    delay(100);
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    ADC1->CR2 |= ADC_CR2_CONT;
    ADC1->CR2 |= ADC_CR2_ADON;



    while (1) {
        
        delay(100);

    }
}