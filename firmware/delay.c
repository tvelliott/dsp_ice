
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

/////////////////////////////////////////////
// delay for 3 clock cycles * ulCount
/////////////////////////////////////////////
void __attribute__((naked)) DelayClk3(unsigned long ulCount)
{
  __asm("    subs    r0, #1\n"
        "    bne     DelayClk3\n"
        "    bx      lr");
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_ms_maintasks(int ms)
{
  int i;
  for(i=0; i<ms; i++) {
    delay_us(1000/8);
    do_main_tasks(0);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  delay for 0.365 * us
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void delay_no_block(uint32_t us)
{
  clear_timer4();

  uint32_t tim = get_timer4();
  while(tim<us) {
    //handle_ethernet(0);
    //check_term_timer();
    tim  = get_timer4();
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_ms(int ms)
{
  delay_us(1000/8 * ms);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_us(int us)
{
  DelayClk3( us * 500/8 );
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_qus(int us)
{
  DelayClk3( us * 125/8 );
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_eus(int us)
{
  DelayClk3( us * 60/8 );
}
void do_get_packet()
{
}
