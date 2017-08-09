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

import java.net.*;
import java.util.*;
import java.io.*;


class compress_fpga_image
{

  static int EOF = -100000;

  public static void main( String args[] ) throws Exception
  {
    new compress_fpga_image().init(args);
  }

  ////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////
  public void init( String args[] ) throws Exception
  {

    byte[] pgm_buffer = new byte[(512*1024)+128];
    byte[] comp_buffer = new byte[(512*1024)+128];

    int file_len=0;
    int crc=0;
    int rle_len=0;

    //read input file
    try {
      FileInputStream fis = new FileInputStream(args[0]);
      file_len = fis.read( pgm_buffer, 0, pgm_buffer.length);
      System.out.println("//filename: "+args[0]);
      System.out.println("//input file length: "+file_len);

      rle_len = rle_encode(pgm_buffer, comp_buffer, file_len); 
      System.out.println("//compressed len: "+rle_len);


      System.out.println("const uint8_t rle_compressed_fpga_image[] = {");
      for(int i=0;i<rle_len;i++) {
        System.out.print( String.format("0x%02x", comp_buffer[i]) );
        if(i!=rle_len-1) System.out.print(",");
        if(i>0 && i%32==0) System.out.println("");
      }

      System.out.print(" };");


    } catch(Exception e) {
      e.printStackTrace();
      System.exit(0);
    }

  }

  public int rle_encode(byte[] buffer, byte[] outbuffer, int len)
  {

    int out_index=0;
    try {

      int currChar, prevChar;             

      prevChar = EOF;     
      int count = 0;

      int index=0;

      while(index<len) {

        currChar = (int) buffer[index++];

        outbuffer[out_index++] = (byte)(currChar&0xff);

        if (currChar == prevChar) {
          count = 0;

          while (index<len) {

            currChar = (int)buffer[index++];
            if (currChar == prevChar) {
              count++;

              if (count == 255) {
                outbuffer[out_index++] = (byte)(count&0xff);


                prevChar = EOF;
                break;
              }
            } else {
              outbuffer[out_index++] = (byte)(count&0xff);
              outbuffer[out_index++] = (byte)(currChar&0xff);
              prevChar = currChar;
              break;
            }
          }
        } else {
          prevChar = currChar;
        }

        if (index==len+1) {
          outbuffer[out_index++] = (byte)(count&0xff);
          break;
        }
      }

    } catch(Exception e) {
      e.printStackTrace();
    }

    return out_index;
  }
}
