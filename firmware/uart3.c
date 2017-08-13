
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "std_io.h"
#include "uart3.h"

#define MAX_STRLEN 256
static volatile int cmd_out_index;
static volatile uint8_t cmd_buffer[256];
static int8_t did_init;

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void uart3_init(int32_t br)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_PinAFConfig( GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
  GPIO_PinAFConfig( GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

  USART_DeInit(USART3);

  USART_InitStructure.USART_BaudRate = br;

  USART_InitStructure.USART_StopBits = USART_StopBits_1;

  USART_InitStructure.USART_Parity = USART_Parity_No;

  USART_InitStructure.USART_WordLength = USART_WordLength_8b;

  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART3, &USART_InitStructure);

  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // enable the USART3 receive interrupt
  USART_ITConfig(USART3, USART_IT_TC, ENABLE); // enable the USART6 transmit empty interrupt

  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;    // we want to configure the USART3 interrupts
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART3 interrupts
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;     // this sets the subpriority inside the group
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      // the USART3 interrupts are globally enabled
  NVIC_Init(&NVIC_InitStructure);

  /* Enable USART3 */
  USART_Cmd(USART3, ENABLE);

  did_init=1;

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void uart3_putc(uint16_t c)
{

  if(!did_init) return;

  /* Send some test data */
  while( !USART_GetFlagStatus(USART3, USART_FLAG_TXE) );
  USART_SendData(USART3, c);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void USART3_IRQHandler()
{
uint8_t buff[2];
  buff[1] = 0;

  //rx
  while( USART_GetITStatus(USART3, USART_IT_RXNE) ) {
    buff[0] = USART3->DR; 
    printf("%s",buff); //send from serial console to telnet console
  }

  //tx
  if( USART_GetITStatus(USART3, USART_IT_TC) ) {
    USART_ClearFlag(USART3, USART_FLAG_TC);

      if(cmd_buffer[cmd_out_index]!=0) {
        uart3_putc(cmd_buffer[cmd_out_index++]);
      } else {
        cmd_out_index=0;
      }
  }
}
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void puts_u3(uint8_t *buffer, int len)
{
  memcpy(cmd_buffer,buffer,len);
  cmd_buffer[len]=0; //terminate
  uart3_putc(cmd_buffer[cmd_out_index++]);
}
