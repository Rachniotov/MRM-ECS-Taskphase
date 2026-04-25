#include "stm32g0xx.h"
#include <string.h>

#define SRAMCAN_FLS_OFF   0x0000U   /* 11-bit standard filter list */
#define SRAMCAN_FLE_OFF   0x0070U   /* 29-bit extended filter list */
#define SRAMCAN_RF0_OFF   0x00B0U   /* Rx FIFO 0                   */
#define SRAMCAN_RF1_OFF   0x0188U   /* Rx FIFO 1                   */
#define SRAMCAN_TEF_OFF   0x0260U   /* Tx event FIFO               */
#define SRAMCAN_TXB_OFF   0x0278U   /* Tx buffers / FIFO / Queue   */

#define SRAMCAN_FLS  ((volatile uint32_t *)(SRAMCAN_BASE + SRAMCAN_FLS_OFF))
#define SRAMCAN_RF0  ((volatile uint32_t *)(SRAMCAN_BASE + SRAMCAN_RF0_OFF))
#define SRAMCAN_TXB  ((volatile uint32_t *)(SRAMCAN_BASE + SRAMCAN_TXB_OFF))
 
#define ELEM_STDID_Pos  18U
#define ELEM_STDID_Msk  (0x7FFU << ELEM_STDID_Pos)
#define ELEM_DLC_Pos    16U
#define ELEM_DLC_Msk    (0xFU   << ELEM_DLC_Pos)


volatile uint8_t flag = 0;
volatile uint8_t data = 1;
volatile uint8_t dataRec = 0;
volatile uint8_t timeoutCount = 0;


volatile uint32_t msTicks = 0;
void SysTick_Handler (void) {
    msTicks++;
}
void delay (uint32_t ms) {
    uint32_t start = msTicks;
    while (msTicks - start < ms) {
        __WFI();
    }
}


void CAN_send (uint32_t id, uint32_t data) {
    SRAMCAN_TXB[0] = (id << ELEM_STDID_Pos);
    SRAMCAN_TXB[1] = (1 << ELEM_DLC_Pos);
    SRAMCAN_TXB[2] = data;

    FDCAN1->TXBAR = 1; // Request transmission of buffer 0
}

// char* CAN_receive (void) {
//     uint32_t rxf0s = FDCAN1->RXF0S;

//     if (rxf0s & FDCAN_RXF0S_F0FL_Msk) { // Check if there are messages in RX FIFO 0
//         uint32_t id = (rxf0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;
//         volatile uint32_t *elem = &SRAMCAN_RF0[id * 4]; // Each element is 4 words (ID/DLC + data)
//         uint8_t dlc = (uint8_t)((elem[1] >> ELEM_DLC_Pos) & ELEM_DLC_Msk);

//         uint8_t chunk[8];
//         uint32_t d0 = elem[2];
//         uint32_t d1 = elem[3];

//         chunk[0] = (uint8_t)(d0);
//         chunk[1] = (uint8_t)(d0 >> 8);
//         chunk[2] = (uint8_t)(d0 >> 16);
//         chunk[3] = (uint8_t)(d0 >> 24);
//         chunk[4] = (uint8_t)(d1);
//         chunk[5] = (uint8_t)(d1 >> 8);
//         chunk[6] = (uint8_t)(d1 >> 16);
//         chunk[7] = (uint8_t)(d1 >> 24);
//         chunk[dlc] = '\0';

//         char *str = (char *)chunk;


//         FDCAN1->RXF0A = id & FDCAN_RXF0A_F0AI_Msk; // Release the FIFO entry
//         return str;
//     } else {
//         return 0; // No message received
//     }
// }

void TIM16_FDCAN_IT0_IRQHandler (void) {
    if (FDCAN1->IR & FDCAN_IR_RF0N) { // Check if RX FIFO 0 new message interrupt
        flag = 1;
        FDCAN1->IR = FDCAN_IR_RF0N; // Clear the interrupt flag
    }
}


int main (void) {
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    RCC->AHBENR |= RCC_AHBENR_FLASHEN;
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2; 

    
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | (0 << RCC_PLLCFGR_PLLM_Pos) | (16 << RCC_PLLCFGR_PLLN_Pos) | (2 << RCC_PLLCFGR_PLLP_Pos) | (1 << RCC_PLLCFGR_PLLR_Pos); 
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN; // Enable PLLR output
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    
    
    RCC->CFGR |= RCC_CFGR_SW_PLLRCLK;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLLRCLK);
    
    SysTick_Config(64000000 / 1000);
    
    RCC->IOPENR |= RCC_IOPENR_GPIODEN; // Enable GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    GPIOD->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1); // Clear mode for PA11 and PA12
    GPIOD->MODER |= (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1); // Set PA11 and PA12 to Alternate Function mode
    GPIOD->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) | (3 << GPIO_OSPEEDR_OSPEED1_Pos)); // Set high speed for PA11 and PA12
    GPIOD->AFR[0] |= (3 << GPIO_AFRL_AFSEL0_Pos) | (3 << GPIO_AFRL_AFSEL1_Pos); // Set AF3 for PA11 and PA12

    GPIOB->MODER &= ~GPIO_MODER_MODE1; // Clear mode for PB2 and PB3
    GPIOB->MODER |= 1 << GPIO_MODER_MODE1_Pos; // Set PB2 to Alternate Function mode

    
    RCC->APBENR1 |= RCC_APBENR1_FDCANEN; // Enable FDCAN clock
    FDCAN1->CCCR |= FDCAN_CCCR_INIT; // Enter initialization mode
    while(!(FDCAN1->CCCR & FDCAN_CCCR_INIT)); // Wait for initialization mode
    FDCAN1->CCCR |= FDCAN_CCCR_CCE; // Enable configuration changes

    FDCAN1->NBTP = (3 << FDCAN_NBTP_NBRP_Pos) | (26 << FDCAN_NBTP_NTSEG1_Pos) | (3 << FDCAN_NBTP_NTSEG2_Pos) | (3 << FDCAN_NBTP_NSJW_Pos); // Set bit timing for 500 kbps
    
    FDCAN1->CCCR &= ~(FDCAN_CCCR_BRSE | FDCAN_CCCR_FDOE);
    // FDCAN1->CCCR |= FDCAN_CCCR_TEST; // Enable test mode
    // FDCAN1->TEST |= FDCAN_TEST_LBCK; // Normal operation mode

    // FDCAN1->TXBC &= ~FDCAN_TXBC_TFQM; // Use dedicated transmit buffers
    FDCAN1->RXGFC = (0 << FDCAN_RXGFC_ANFS_Pos) 
                    | (1 << FDCAN_RXGFC_LSS_Pos)
                    | (0 << FDCAN_RXGFC_ANFE_Pos);

    SRAMCAN_FLS[0] = (2 << 30) | (0x1 << 27) | (0x0 << 16) | (0x0 << 0);


    FDCAN1->IE |= FDCAN_IE_RF0NE; // Enable RX FIFO 0 new message interrupt
    FDCAN1->ILS = 0;
    FDCAN1->ILE |= FDCAN_ILE_EINT0; // Enable interrupt line 0
    NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);

    FDCAN1->CCCR &= ~FDCAN_CCCR_INIT; // Exit initialization mode
    while (FDCAN1->CCCR & FDCAN_CCCR_INIT); // Wait for normal mode


    while (1) {
        if (data == 1) {
            // GPIOB->BSRR = GPIO_BSRR_BS1;
            // delay(100);
            data = 2;
            
        } else {
            // GPIOB->BSRR = GPIO_BSRR_BS1;
            // delay(100);
            data = 1;
        }

        CAN_send(0x123, data);
        delay(100);
        
        // flag = 0;
        
        // int timeout = 1000; // 1 second timeout
        // uint32_t start = msTicks;
        // while(!flag || (msTicks - start < timeout)) {
        //     __WFI();
        // }
        // if (!flag) {
        //     timeoutCount++;
        // }
        
    
        // dataRec = CAN_receive();

        

    }

}