
#include <stdio.h>
#include <stdint.h>
#include "crc.h"
#include "flash_sst.h"

volatile static uint16_t crc;
volatile int8_t do_crc=0;

//crc32 holding register,  should be initialized before call to crc32_range
//
//        typical usage for compatibility with Linux crc calculations for example:
//        ------------------
//        crc32_val = ~0L;
//        uint32_t eth_crc = crc32_range( &eth_frame_buffer[2], frame_len );
//        eth_crc = ~eth_crc;
uint32_t crc32_val = 0;
uint32_t flash_crc32_val = 0;
static uint16_t df1_crc;

#define CRC32_POLYNOMIAL 0xEDB88320L  //common poly used in 32-bit crc calculations


static unsigned short crc_table [256] = {

  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
  0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
  0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
  0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
  0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
  0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,
  0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
  0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
  0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
  0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,
  0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
  0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
  0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
  0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
  0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6,
  0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
  0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
  0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,
  0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
  0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
  0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
  0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
  0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
  0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2,
  0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
  0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
  0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
  0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,
  0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
  0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
  0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
  0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
  0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
unsigned short CRCCCITT(uint8_t *data, int16_t length, uint16_t seed, uint16_t final)
{

  int16_t count;
  unsigned int crc = seed;
  unsigned int temp;

  for (count = 0; count < length; ++count) {
    temp = (*data++ ^ (crc >> 8)) & 0xff;
    crc = crc_table[temp] ^ (crc << 8);
  }

  return (unsigned short)(crc ^ final);

}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
static uint32_t CRC32Value(uint16_t i)
{
  int16_t j;
  uint32_t ulCRC;

  ulCRC = i;
  for ( j = 8 ; j > 0; j-- ) {
    if ( ulCRC & 1 )
      ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
    else
      ulCRC >>= 1;
  }
  return ulCRC;
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
uint32_t crc32_range( uint8_t *ucBuffer, int32_t ulCount)
{
  uint32_t ulTemp1;
  uint32_t ulTemp2;

  while ( ulCount-- != 0 ) {
    ulTemp1 = ( crc32_val >> 8 ) & 0x00FFFFFFL;
    ulTemp2 = CRC32Value( ((uint16_t) crc32_val ^ *ucBuffer++ ) & 0xff );
    crc32_val = ulTemp1 ^ ulTemp2;
  }
  return crc32_val;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
uint32_t flash_crc32_range( uint8_t *ucBuffer, int32_t ulCount)
{
  uint32_t ulTemp1;
  uint32_t ulTemp2;

  while ( ulCount-- != 0 ) {
    ulTemp1 = ( flash_crc32_val >> 8 ) & 0x00FFFFFFL;
    ulTemp2 = CRC32Value( ((uint16_t) flash_crc32_val ^ *ucBuffer++ ) & 0xff );
    flash_crc32_val = ulTemp1 ^ ulTemp2;
  }
  return flash_crc32_val;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void crc_test(void)
{
  uint8_t testdata[] = {0x25, 0x3b, 0x05, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00 };

  uint16_t crc = CRCCCITT( testdata, sizeof(testdata), 0xffff, 0xffff);
  printf("crc test: %04x", crc);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
int8_t check_crc16( uint8_t *buffer)
{
  uint16_t crc = CRCCCITT( buffer, 10, 0xffff, 0xffff);
  uint8_t crch = buffer[10];
  uint8_t crcl = buffer[11];
  if( crc>>8 == crch  && (crc&0xff)==crcl) return 0;
  else return -1;
}
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
int8_t update_crc16( uint8_t *buffer)
{
  uint16_t crc = CRCCCITT( buffer, 10, 0xffff, 0xffff);
  buffer[10] = crc>>8;
  buffer[11] = crc&0xff;
}
////////////////////////////////////
////////////////////////////////////
void clear_crc()
{
  crc=0xffff;
}
////////////////////////////////////
////////////////////////////////////
void clear_crc2()
{
  crc=0xffff;
}

////////////////////////////////////
//
////////////////////////////////////
uint16_t get_crc()
{
  return crc;
}

////////////////////////////////////
////////////////////////////////////
uint8_t get_crc_h()
{
  return (crc>>8)&0xff;
}

////////////////////////////////////
////////////////////////////////////
uint8_t get_crc_l()
{
  return crc&0xff;
}

////////////////////////////////////////////////////////////////////////////////////////
//     Update the CRC for transmitted and received data using
//     the CCITT 16bit polynomial (X^16 + X^12 + X^5 + 1).
//
////////////////////////////////////////////////////////////////////////////////////////

uint16_t do_crc16(uint8_t data)
{
  crc  = (uint8_t)(crc >> 8) | (crc << 8);
  crc ^= data;
  crc ^= (uint8_t)(crc & 0xff) >> 4;
  crc ^= (crc << 8) << 4;
  crc ^= ((crc & 0xff) << 4) << 1;

  return crc;
}

////////////////////////////////////////////////////////////////////////////////////
// assumes storage for 2 bytes crc at end of struct
////////////////////////////////////////////////////////////////////////////////////

void update_struct_crc(uint8_t *sp, int16_t size)
{
  int16_t i;
  uint8_t *ptr = sp;

  clear_crc();
  for (i=0; i<size-2; i++) {
    do_crc16(*ptr++);
  }
  *ptr++ = get_crc_h();
  *ptr++ = get_crc_l();
}

////////////////////////////////////////////////////////////////////////////////////
// assumes storage for 2 bytes crc at end of struct
////////////////////////////////////////////////////////////////////////////////////

uint8_t check_struct_crc(uint8_t *sp, int16_t size)
{
  int16_t i;
  uint8_t *ptr = sp;

  clear_crc();
  for (i=0; i<size-2; i++) {
    do_crc16(*ptr++);
  }
  if (*ptr++ != get_crc_h())
    return 1;
  ;
  if (*ptr++ != get_crc_l())
    return 1;
  ;

  return 0;
}
