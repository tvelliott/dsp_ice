
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
#include <stdarg.h>
#include <stdio.h>
#include "std_io.h"
#include "global.h"

uint8_t net_buffer[MAX_NET_BUFFER];
extern int telnet_session_count;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int32_t tn_scanf( uint8_t *outbuffer, const char * format, ...)
{
  uint16_t ret=0;

  va_list args;

  va_start(args,format);

  outbuffer[MAX_PRINTF_BUFFER-1]=0;

  ret = vsscanf(outbuffer, format, args);

  va_end(args);

  return ret; //length sent to device
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int get_print_buffer_left()
{
  return MAX_NET_BUFFER-strlen(net_buffer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int printf( const char * format, ...)
{

  uint16_t ret=0;


  va_list args;

  uint8_t printf_buf[MAX_PRINTF_BUFFER];

  va_start(args,format);

  ret = vsnprintf(printf_buf, sizeof(printf_buf)-1, format, args);
  printf_buf[MAX_PRINTF_BUFFER-1] = 0;

  int len1 = strlen(net_buffer);
  int len2 = strlen(printf_buf);


  if(len1+len2 < MAX_NET_BUFFER) {
    net_buffer[len1+len2]=0x00;
    strcat(net_buffer, printf_buf);
  }

  va_end(args);

  return ret; //length sent to device
}
