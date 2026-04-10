#include <stdint.h>
#include "stm32f1xx.h"

volatile int buttonState = 0;
volatile uint32_t dutyCycle = 0;

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

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_CC1IF) {
        GPIOC->BSRR = GPIO_BSRR_BS13;
        
        TIM2->SR = ~TIM_SR_CC1IF;
    }

    if (TIM2->SR & TIM_SR_UIF) {
        GPIOC->BSRR = GPIO_BSRR_BR13;

        TIM2->SR = ~TIM_SR_UIF;
    }
}

void EXTI0_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR0) {
        buttonState = !buttonState;

        GPIOA->ODR |= GPIO_ODR_ODR0;
        EXTI->PR = EXTI_PR_PR0;
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

    GPIOC->CRH &= ~(GPIO_CRH_MODE13_Msk | GPIO_CRH_CNF13_Msk);
    GPIOC->CRH |= 1 << GPIO_CRH_MODE13_Pos;


    while (1) {
        GPIOC->BSRR = GPIO_BSRR_BS13;
        delay(1000);
        GPIOC->BSRR = GPIO_BSRR_BR13;
        delay(1000);

    }
}