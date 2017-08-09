/**************************************************************************
*
* Playtune: An Arduino Tune Generator
*
* Plays a polyphonic musical score
* For documentation, see the Playtune.cpp source code file
*
*   (C) Copyright 2011,2016 Len Shustek
*
**************************************************************************/
/*
* Change log
*  19 January 2011, L.Shustek, V1.0: Initial release.
*  10 June 2013, L. Shustek, V1.3
*     - change for compatibility with Arduino IDE version 1.0.5
*   6 April 2015, L. Shustek, V1.4
*     - change for compatibility with Arduino IDE version 1.6.x
*  28 May 2016, T. Wasiluk
*     - added support for ATmega32U4
*  10 July 2016, Nick Shvelidze
*     - Fixed include file names for Arduino 1.6 on Linux.
*/

#ifndef Playtune_h
#define Playtune_h

typedef struct __attribute__((packed))
{
  int8_t id1;     // 'P'
  int8_t id2;     // 't'
  uint8_t hdr_length; // length of whole file header
  uint8_t f1;         // flag byte 1
  uint8_t f2;         // flag byte 2
  uint8_t num_tgens;  // how many tone generators are used by this score
}
file_hdr_t;


#define PROTO(x) x
#include "protos/Playtune_proto.h"

#endif
