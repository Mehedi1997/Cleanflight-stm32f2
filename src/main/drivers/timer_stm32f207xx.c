/*
 * timer_stm32f207xx.c
 *
 *  Created on: Jun 21, 2017
 *      Author: Benzene
 */
#include "platform.h"

#include "common/utils.h"

#include "stm32f2xx.h"
#include "rcc.h"
#include "timer.h"

const timerDef_t timerDefinitions[HARDWARE_TIMER_DEFINITION_COUNT] = {
    { .TIMx = TIM1,  .rcc = RCC_APB2(TIM1),  .inputIrq = TIM1_CC_IRQn},
    { .TIMx = TIM2,  .rcc = RCC_APB1(TIM2),  .inputIrq = TIM2_IRQn},
    { .TIMx = TIM3,  .rcc = RCC_APB1(TIM3),  .inputIrq = TIM3_IRQn},
    { .TIMx = TIM4,  .rcc = RCC_APB1(TIM4),  .inputIrq = TIM4_IRQn},
    { .TIMx = TIM5,  .rcc = RCC_APB1(TIM5),  .inputIrq = TIM5_IRQn},
    { .TIMx = TIM6,  .rcc = RCC_APB1(TIM6),  .inputIrq = 0},
    { .TIMx = TIM7,  .rcc = RCC_APB1(TIM7),  .inputIrq = 0},
    { .TIMx = TIM8,  .rcc = RCC_APB2(TIM8),  .inputIrq = TIM8_CC_IRQn},

    { .TIMx = TIM9,  .rcc = RCC_APB2(TIM9),  .inputIrq = TIM1_BRK_TIM9_IRQn},
    { .TIMx = TIM10, .rcc = RCC_APB2(TIM10), .inputIrq = TIM1_UP_TIM10_IRQn},
    { .TIMx = TIM11, .rcc = RCC_APB2(TIM11), .inputIrq = TIM1_TRG_COM_TIM11_IRQn},

    { .TIMx = TIM12, .rcc = RCC_APB1(TIM12), .inputIrq = TIM8_BRK_TIM12_IRQn},
    { .TIMx = TIM13, .rcc = RCC_APB1(TIM13), .inputIrq = TIM8_UP_TIM13_IRQn},
    { .TIMx = TIM14, .rcc = RCC_APB1(TIM14), .inputIrq = TIM8_TRG_COM_TIM14_IRQn},
};

/*
    need a mapping from dma and timers to pins, and the values should all be set here to the dmaMotors array.
    this mapping could be used for both these motors and for led strip.

    only certain pins have OC output (already used in normal PWM et al) but then
    there are only certain DMA streams/channels available for certain timers and channels.
     *** (this may highlight some hardware limitations on some targets) ***

    DMA1

    Channel Stream0   |  Stream1  |   Stream2 |  Stream3  |  Stream4  |  Stream5  |  Stream6  |  Stream7  |
    _______|__________|___________|___________|___________|___________|___________|___________|___________|
    0	   |		  |			  | 		  |			  |			  |			  |			  |			  |
    1	   |		  |			  | 		  |			  |			  |			  |			  |			  |
    _______|__________|___________|___________|___________|___________|___________|___________|___________|
    2      | TIM4_CH1 |           |           |  TIM4_CH2 |           |           |           |  TIM4_CH3 |
    _______|__________|___________|___________|___________|___________|___________|___________|___________|
    3      |          |  TIM2_CH3 |           |           |           | TIM2_CH1  |  TIM2_CH2 |  TIM2_CH4 |
    	   |		  |			  |			  |			  |			  |			  |			  |  TIM2_CH4 |
    _______|__________|___________|___________|___________|___________|___________|___________|___________|
    4______|_________ |___________|___________|___________|___________|___________|___________|___________|
    5      |          |           |  TIM3_CH4 |           | TIM3_CH1  | TIM3_CH2  |           |  TIM3_CH3 |
    _______|__________|___________|___________|___________|___________|___________|___________|___________|
    6      | TIM5_CH3 |  TIM5_CH4 |  TIM5_CH1 |  TIM5_CH4 | TIM5_CH2  |			  |			  |           |
    7_________________|___________|___________|___________|___________|___________|___________|___________|

    DMA2

    Channel Stream0   |  Stream1   |  Stream2   |  Stream3  |   Stream4   |  Stream5   |  Stream6  |  Stream7 |
    __________________|____________|____________|___________|_____________|____________|___________|__________|
    0                 |            |  TIM8_CH1  |           |             |            |  TIM1_CH1 |		  |
                      |            |  TIM8_CH2  |           |             |            |  TIM1_CH2 |		  |
                      |            |  TIM8_CH3  |           |             |            |  TIM1_CH3 |		  |
     _________________|____________|____________|___________|_____________|____________|___________|__________|
    1_________________________________________________________________________________________________________|
    2_________________________________________________________________________________________________________|
    3_________________________________________________________________________________________________________|
    4_________________________________________________________________________________________________________|
    5_________________________________________________________________________________________________________|
    6                 |  TIM1_CH1  |  TIM1_CH2  |  TIM1_CH1  |  TIM1_CH4  |           |  TIM1_CH3 |			  |
    __________________|____________|____________|____________|____________|___________|___________|___________|
    7                 |  TIM8_UP   |  TIM8_CH1  |  TIM8_CH2  |  TIM8_CH3  |           |           |  TIM8_CH4 |
    __________________|____________|____________|____________|____________|___________|___________|___________|
*/

uint8_t timerClockDivisor(TIM_TypeDef *tim)
{
    if (tim == TIM1 || tim == TIM8 || tim == TIM9 || tim == TIM10 || tim == TIM11) {
        return 1;
    } else {
        return 2;
    }
}

uint32_t timerClock(TIM_TypeDef *tim)
{
    if (tim == TIM1 || tim == TIM8 || tim == TIM9 || tim == TIM10 || tim == TIM11) {
        return SystemCoreClock;
    } else {
        return SystemCoreClock / 2;
    }
}


