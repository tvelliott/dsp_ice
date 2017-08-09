
//MIT License
//
//Copyright (c) 2017 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "misc.h"
#include "global.h"
#include "config.h"
#include "stm_timers.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void init_stm_timers(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


  TIM_TimeBaseStructure.TIM_Prescaler = 1;
  TIM_TimeBaseStructure.TIM_Period = -1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);


  TIM_TimeBaseStructure.TIM_Prescaler = 256;
  TIM_TimeBaseStructure.TIM_Period = -1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* Enable the TIM3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the TIM3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_Init(&NVIC_InitStructure);

  //TIM_PrescalerConfig(TIM3, 1, TIM_PSCReloadMode_Immediate);
  //TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  //TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);


  TIM_Cmd(TIM2 , ENABLE);
  TIM_Cmd(TIM3 , ENABLE);
  TIM_Cmd(TIM4 , ENABLE);

  clear_timer2();
  clear_timer3();

}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t get_timer4()
{
  return TIM_GetCounter(TIM4);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t clear_timer4()
{
  TIM_Cmd(TIM4 , DISABLE);
  TIM_SetCounter(TIM4,0);
  TIM_Cmd(TIM4 , ENABLE);
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t get_timer2()
{
  return TIM_GetCounter(TIM2);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t clear_timer2()
{
  TIM_Cmd(TIM2 , DISABLE);
  TIM_SetCounter(TIM2,0);
  TIM_Cmd(TIM2 , ENABLE);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t set_timer2(uint32_t val)
{
  TIM_Cmd(TIM2 , DISABLE);
  TIM_SetCounter(TIM2,val);
  TIM_Cmd(TIM2 , ENABLE);
}



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t get_timer3()
{
  return TIM_GetCounter(TIM3);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t clear_timer3()
{
  TIM_Cmd(TIM3 , DISABLE);
  TIM_SetCounter(TIM3,0);
  TIM_Cmd(TIM3 , ENABLE);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
uint32_t set_timer3(uint32_t val)
{
  TIM_Cmd(TIM3 , DISABLE);
  TIM_SetCounter(TIM3,val);
  TIM_Cmd(TIM3 , ENABLE);
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void TIM3_IRQHandler()
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
