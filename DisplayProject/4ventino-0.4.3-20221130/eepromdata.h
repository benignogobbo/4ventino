// +---------------------------------------------------+
// | The 4ventino firmware. EEPROM class               |
// | Benigno Gobbo                                     |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+
#ifndef EEPROMDATA_H
#define EEPROMDATA_H

#include <EEPROM.h>  // ATmega1284P has 4kB eeprom storage

// +--------------------------+
// | What is stored in eeprom |
// +--------------------------+
struct EEPROMContent {
  char init[5];          // char string to flag initialisation
  int  line;             // gas line number
  byte mac[6];           // mac address
  char passkey[5];       // setup pass key 
  byte ip[4];            // IP address
  byte dns[4];           // DNS 
  byte gw[4];            // gateway
  byte snm[4];           // subnet mac
  char ntp[32];          // NTP server
  float slope[5];        // scaling straight line slope
  float inter[5];        // scaling straight line intercept
  byte  sleep;           // sleep flag
};

// +-----------------------------+
// | eeprom store/retrieve class |
// +-----------------------------+
class EEPROMData {
  public:  
    EEPROMData( void ) {};
    static const byte macBase[6];
    static EEPROMContent eepromContent;
    bool loadFromEEPROM( void );
    bool saveToEEPROM( void );
    bool clear( void );
    void dump( void );
  private:
    static const byte _startAddress;
};

const byte EEPROMData::_startAddress = 0;
const byte EEPROMData::macBase[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0xD0 };  
EEPROMContent EEPROMData::eepromContent = {
  "", 0, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "",
  { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
  "", { 1.0, 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0, 0.0, 0.0 }, 0
};

// +----------------------+
// | Retrieve eeprom data |
// +----------------------+
bool EEPROMData::loadFromEEPROM( void ) {
  for( unsigned int i=0; i<sizeof(eepromContent); i++ ) {
    *( (byte*)&eepromContent + i ) = EEPROM.read( _startAddress + i );
  }
  return true;
}

// +---------------------+
// | Save data on eeprom |
// +---------------------+
bool EEPROMData::saveToEEPROM( void ) {
  for( unsigned int i=0; i<sizeof(eepromContent); i++ ) {
    EEPROM.write( _startAddress + i, *( (byte*)&eepromContent + i ));
    if( EEPROM.read( _startAddress + i ) != *( (byte*)&eepromContent + i ) ) {
      return false;
    }
  }
  return true;
}

// +--------------+
// | Clear eeprom |
// +--------------+
bool EEPROMData::clear( void ) {
  for( int i = 0 ; i<EEPROM.length() ; i++ ) {
    EEPROM.write( i, 0 );
    if( EEPROM.read( i ) != 0 ) {
      return false;
    }
  }
  return true;
}

// +---------------------+
// | Dump eeprom content |
// +---------------------+
void EEPROMData::dump( void ) {
  Serial.println( eepromContent.init );
  Serial.println( eepromContent.line );
  for( int i=0; i<6; i++ ) { 
    Serial.print( eepromContent.mac[i], HEX );
    if( i<5 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  Serial.println( eepromContent.passkey );
  for( int i=0; i<4; i++ ) {
    Serial.print( eepromContent.ip[i] );
    if( i<3 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  for( int i=0; i<4; i++ ) {
    Serial.print( eepromContent.dns[i] );
    if( i<3 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  for( int i=0; i<4; i++ ) {
    Serial.print( eepromContent.gw[i] );
    if( i<3 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  for( int i=0; i<4; i++ ) {
    Serial.print( eepromContent.snm[i] );
    if( i<3 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  Serial.println( eepromContent.ntp );
  for( int i=0; i<5; i++ ) {
    Serial.print( eepromContent.slope[i] );
    if( i<4 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  for( int i=0; i<5; i++ ) {
    Serial.print( eepromContent.inter[i] );
    if( i<4 ) Serial.print( " " );
    else Serial.println( "" ); 
  }
  Serial.println( eepromContent.sleep );
}

#endif //EEPROMDATA_H
