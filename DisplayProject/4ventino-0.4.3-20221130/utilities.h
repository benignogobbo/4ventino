// +---------------------------------------------------+
// | The 4ventino firmware. Utilities                  |
// | Benigno Gobbo                                     |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+
#ifndef UTILITIES_H
#define UTILITIES_H

#include <avr/io.h>
#include <avr/wdt.h>
#include <ctype.h>

// +--------------------+
// | Software AVR reset |
// +--------------------+
void resetAVR( void ) {
  wdt_enable( WDTO_30MS ); 
  while(true){};
}

// Reset Function
//void( *hardReset )(void) = 0; // 

// +--------------------------------+
// | return a two hex digits string |
// +--------------------------------+
String twoHex( byte number ) {
  String s = "";
  if( number < 16 ) {
    s = "0" + String( number, HEX );  
  }
  else {
    s = String( number, HEX ); 
  }
  return s;
}

// +------------------------------------+
// | Check if a char array "is" a float |
// +------------------------------------+
bool isFloat( char s[] ) {
  if( strlen( s ) < 1 ) return false;
  if( !isDigit( s[0] ) && s[0] != '+' && s[0] != '-' && s[0] != '.' ) return false;  
  for( byte i = 1; i < strlen( s ); i++ ) {
    if( !isDigit( s[i] ) && s[i] != '.' ) return false; 
  }
  return true;
}

// +-------------------------------------+
// | Check if a 2 char array "is" a byte |
// +-------------------------------------+
bool isByte( char s[] ) {
  if( sizeof( s ) < 2 ) return false;
  if( isHexadecimalDigit( s[0] ) && isHexadecimalDigit( s[1] ) ) return true;
  return false;
}

// +------------------------------------+
// | Convert a 2 char array into a byte |
// +------------------------------------+
byte toByte( char s[] ) {
  if( sizeof( s ) < 2 ) return 0;
  uint16_t v = 0;
  for( int i=0; i<2; i++ ) {
    if( s[i] < 58 ) {
      v += s[i]-48; 
    }
    else if( s[i] < 71 ) {
      v += s[i]-55; //-65+10
    }
    else {
      v += s[i]-87; //-97+10
    }
    v = v * 16;
  }
  if( v != 0 ) v = v /16;
  return( v&0xFF );
}

// +-----------------------------------------------+
// | search for the 1st 'E' or 'F' in a char array |
// +-----------------------------------------------+
int findEF( const char s[] ) {
  for( int i = 0; i < strlen(s); i++ ) {
    if( s[i] == 'F' || s[i] == 'E' ) {
      return( i );
    }
  }
  return -1;
}

#endif //UTILITIES_H
