// +---------------------------------------------------+
// | The 4ventino firmware                             |
// | Benigno Gobbo                                     |
// | 2022-10-17                                         |
// | V0.4.2                                            |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+

#define VERSION_MAJOR 0
#define VERSION_MINOR 4
#define VERSION_REV   2

#include "eepromdata.h"
#include "nxdydisplay.h"
#include "utilities.h"
#include "network.h"

// I/O Setup
const int           sensorPins[8] = { A0, A1, A2, A3, A4, A5, A6, A7 };    // 8x Sensor 4-20mA input pins (only 5 of them are used)
const int           relay[6] = { 12, 13, 14, 15, 22, 23 };                 // Digital alarms (only the 1st is used)
const float         resistor = 100.0;                                      // 100 ohm 
const float         adcMax = 1023.0;                                       // 10 bit ADC
const float         vRef = 2.522;                                          // Vref = 2.522V
const String        units[5] = { " °C\n", " °C\n", " l/min\n", " hPa\n", " hPa\n" };
const int           resolutions[5] = { 1, 1, 2, 0, 0 };        // Data output precision 
const unsigned long dataUpdatePeriod = 250UL;                  // Data updates every 250ms
const unsigned long ntpUpdatePeriod  = 60000UL;                // NTP updates every minute
const int           TPAlarmThreshold = -30;                    // Dew Point alarm threshold in °C
const int           TPDeltaThreshold = 10;                     // Max Dew Point discrepancy in °C

float  sensor[5] = {0}; // DewPoint1, DewPoint2, Flux, Pressure in line, Pressure in reference line.

unsigned long tm0 = 0, tm1 = 0;

char buff[256];

// ++------------------------------------------------------++
// ++------------------------------------------------------++
// || Define here the Dew Point Sensors alarm algorithm... ||
// ++------------------------------------------------------++
// ++------------------------------------------------------++
void checkTp( void ) {
  float TPLowerEdge[2] = { 0 };
  // If measured frost-point is below transmitter lower edge send an alarm
  for( int i = 0; i < 2; i++ ) {
    TPLowerEdge[i] = eepromData.eepromContent.slope[i] * 4 + eepromData.eepromContent.inter[i];
    if( sensor[i] < TPLowerEdge[i] ) {
      digitalWrite( relay[0], HIGH );
      return;
    }
  }
  // If TP1 is over threshold and TP2 is over threshold plus "delta" send the alarm
  //if( ( sensor[0] > TPAlarmThreshold ) && ( sensor[1] > ( TPAlarmThreshold + TPDeltaThreshold ) ) ) {
  // If TP1 is over theshold send the alarm
  if( sensor[0] > TPAlarmThreshold ) {
    digitalWrite( relay[0], HIGH );
    return;
  }
  digitalWrite( relay[0], LOW );
}

// +---------------+
// | Display Error |
// +---------------+
void onErrorAction( void ) {
  int answer = NXDY_pollDisplay( &NXDY_gir );
  if( answer == 1 ) {
    NXDY_page5.show();
    while(true);
  }
  else if( answer ==  2 ) {
    NXDY_inSetup = true;
    NXDY_page4.show();
    while( NXDY_inSetup ) {
      nexLoop( nex_listen_list );
    }
  }
  else if( answer ==  3 ) {
    resetAVR();
  }
}

// +----------------------------+
// | Initialize ethernet device |
// +----------------------------+
void ethernetInit( void ) {

  Ethernet.init( 4 );

  int count = 0;
  do {
    Ethernet.begin( eepromData.eepromContent.mac, eepromData.eepromContent.ip, eepromData.eepromContent.dns, eepromData.eepromContent.gw, eepromData.eepromContent.snm );
  } while( count++ <5 && Ethernet.hardwareStatus() != EthernetW5500 && Ethernet.linkStatus() != LinkON );

  if( count >= 5 ) {
    if( Ethernet.hardwareStatus() == EthernetNoHardware ) {
      //Serial.println( String( count ) + ". Error in ethernet initialization" );
      NXDY_page3.show();
      NXDY_err.Set_font_color_pco( 63488 );
      NXDY_err.setText( "ERROR ethernet init failed" );
      onErrorAction();
    }
    else if( Ethernet.hardwareStatus() == EthernetW5500 ) {
      if( Ethernet.linkStatus() == Unknown ) {
        //Serial.println( String( count ) + ". Error ethernet link unknown" );
        NXDY_page3.show();
        NXDY_err.Set_font_color_pco( 63488 );
        NXDY_err.setText( "ERROR ethernet link unknown" );
        onErrorAction();
      }
      else if( Ethernet.linkStatus() == LinkOFF ) {
        //Serial.println( String( count ) + ". Error in ethernet link off" );
        NXDY_page3.show();
        NXDY_err.Set_font_color_pco( 63488 );
        NXDY_err.setText( "ERROR ethernet link off" );
        onErrorAction();
      }
      //else if( Ethernet.linkStatus() == LinkON ) {
      //  Serial.println( String( count ) + ". Ethernet link on" );
      //}
    }
    else {
      //Serial.println( String( count ) + ". Unknown error during ethernet initialization" );
      NXDY_page3.show();
      NXDY_err.Set_font_color_pco( 63488 );
      NXDY_err.setText( "Unknown ERROR during ethernet initialization" );
      onErrorAction();
    }
  }
}

// +----------------------------------------+
// | Send eeprom content to the remote host |
// +----------------------------------------+
void sendEEPROMDataToRemote( int mode ) {
  String eeStr = "S";  // "header"
  eeStr += "B" + twoHex( eepromData.eepromContent.line & 0xFF ) + "B" + twoHex( eepromData.eepromContent.sleep & 0xFF );
  for( int i=0; i<5; i++ ) {
    eeStr += "B" + twoHex( eepromData.eepromContent.passkey[i] );
  }
  for( int i=0; i<4; i++ ) {
    eeStr += "B" + twoHex( eepromData.eepromContent.ip[i] );
  }
  for( int i=0; i<4; i++ ) {
    eeStr += "B" + twoHex( eepromData.eepromContent.dns[i] );
  }
  for( int i=0; i<4; i++ ) {
   eeStr += "B" + twoHex( eepromData.eepromContent.gw[i] );
  }
  for( int i=0; i<4; i++ ) {
    eeStr += "B" + twoHex( eepromData.eepromContent.snm[i] );
  }
  for( int i=0; i<32; i++ ) {
    eeStr += "B" + twoHex( eepromData.eepromContent.ntp[i] );
  }
  for( int i=0; i<5; i++ ) {
    eeStr += "F" + String( 4.0*eepromData.eepromContent.slope[i] + eepromData.eepromContent.inter[i] );
  }
  for( int i=0; i<5; i++ ) {
    eeStr += "F" + String( 20.0*eepromData.eepromContent.slope[i] + eepromData.eepromContent.inter[i] );
  }
  eeStr += "E"; // "trailer"
  if( mode == 1 )  { // Serial
    Serial.write( eeStr.c_str() );
  }
  else if( mode == 2 ) {  // Ethernet
    server.write( eeStr.c_str() );
  } 
}

// +-----------------------------------------+
// | Get eeprom content from the remote host |
// +-----------------------------------------+
bool getEEPROMDataFromRemote( char s[]) {

  char st[3];
  char sf[17];
  st[2] = '\0';
  st[16] = '\0';
  if( s[0] != 'S' && s[strlen(s)-2] != 'E' ) {
    return false;
  }
  s += 1;
  if( s[0] != 'B' ) { return( false ); };
  s += 1;
  strncpy( st, s, 2 ); // line
  if( isByte( st ) ) {
    eepromData.eepromContent.line = toByte( st );
  }
  s += 2;
  if( s[0] != 'B' ) { return( false ); };
  s += 1;
  strncpy( st, s, 2 ); // sleep
  if( isByte( st ) ) {
    eepromData.eepromContent.sleep = toByte( st );
  }
  for( int i=0; i<5; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // passkey*5
    if( isByte( st ) ) { 
      eepromData.eepromContent.passkey[i] = toByte( st );
    }
  }
  eepromData.eepromContent.passkey[4] = '\0';
  for( int i=0; i<4; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // ip*4
    if( isByte( st ) ) {
      eepromData.eepromContent.ip[i] = toByte( st );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // dns*4
    if( isByte( st ) ) {
      eepromData.eepromContent.dns[i] = toByte( st );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // gw*4
    if( isByte( st ) ) {
      eepromData.eepromContent.gw[i] = toByte( st );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // snm*4
    if( isByte( st ) ) {
      eepromData.eepromContent.snm[i] = toByte( st );
    }
  }
  for( int i=0; i<32; i++ ) { 
    s += 2;
    if( s[0] != 'B' ) { return( false ); };
    s += 1;
    strncpy( st, s, 2 ); // ntp*32
    eepromData.eepromContent.ntp[i] = toByte( st );
  }
  s += 2;
  float minv[5] = {0}, maxv[5] = {0};
  for( int i=0; i<5; i++ ) { 
    if( s[0] != 'F' ) { return( false ); };
    s += 1;
    int pos = findEF( s );
    if( pos == -1 ) pos = strlen(s)-1;
    strncpy( sf, s, pos );
    sf[pos] ='\0';
    if( isFloat( sf ) ) {
      minv[i] = atof( sf );
    }
    else {
     minv[i] = -9999.;
    }
    s += pos;
  }
  for( int i=0; i<5; i++ ) {
    if( s[0] != 'F' ) { return( false ); };
    s += 1;
    int pos = findEF( s );
    if( pos == -1 ) pos = strlen(s)-1;
    strncpy( sf, s, pos );
    sf[pos] ='\0';
    if( isFloat( sf ) ) {
      maxv[i] = atof( sf );
    }
    else {
      maxv[i] = -9999.;
    }
    s += pos;
  }
  for( int i=0; i<5; i++ ) { 
    eepromData.eepromContent.slope[i] = ( maxv[i] - minv[i] ) / 16.0;
    eepromData.eepromContent.inter[i] = ( 5.0 * minv[i] - maxv[i] ) / 4.0;
  }
  if( s[0] != 'E' ) {
    return( false ); 
  }
  return( true );
}
  
// +---------------+
// | Arduino Setup |
// +---------------+ 
void setup( void ) {

  delay( 2000 ); // wait a bit for switch and hub boot 

  Serial.begin(9600);
  while (!Serial) {}   // wait for serial
  Serial.setTimeout( 50 );

  // AT settings
  analogReference( EXTERNAL ); // Vref in pin AREF. External 2.5V reference voltage from LTC6655
  for( int i=0; i<6; i++ ) {
    pinMode( relay[i], OUTPUT ); // Output Relays
    digitalWrite( relay[i], LOW );
  }

  // Nextion display initialisation 
  NXDY_init();
  // Load data from EEPROM
  if( !eepromData.loadFromEEPROM() ) {
    //Serial.println( "Error in reading EEPROM" );
    NXDY_page3.show();
    NXDY_err.Set_font_color_pco( 63488 );  // red text
    NXDY_err.setText( "ERROR reading from EEPROM" );
    onErrorAction();
  }
  else {
    if( strncmp( eepromData.eepromContent.init, "done", 4 ) != 0 ) {
      //Serial.println( "Init NOT done" );
      if( !eepromData.clear() ) {   // clear EEPROM if never written
        //Serial.println( "Error during EEPROM clearing" );
        NXDY_page3.show();
        NXDY_err.Set_font_color_pco( 63488 );
        NXDY_err.setText( "ERROR during EEPROM clearing" );
        onErrorAction();
      }
      NXDY_page2.show();
      int answer = NXDY_pollDisplay( &NXDY_scs );
      if( answer == 1 ) NXDY_saveSettings();
      if( !eepromData.saveToEEPROM() ) {
        //Serial.println( "Error in writing to EEPROM" );
        NXDY_page3.show();
        NXDY_err.Set_font_color_pco( 63488 );
        NXDY_err.setText( "ERROR writing to EEPROM" );
        onErrorAction();
      }
      resetAVR(); // Things changed so...   
    }
  }
  if( eepromData.eepromContent.sleep == 1 ) {
    NXDY_sleepAndWakeOnTouch( 180 );  // sleep after 3 minutes, wake on touch
  } 
  else {
    NXDY_neverSleep();                // never sleep
  }

  //eepromData.dump();
  
  // Ethernet...
  ethernetInit();
  
  // TCP/IP server initialisation 
  server.begin();
  delay( 100 );
  
  // NTP
  udp.begin( UDPCommunicationPort );
  //if( !nt.syncTime() ) {
    //Serial.println( "ERROR time NOT synchronized with NTP server." );
    //NXDY_page3.show();
    //NXDY_err.Set_font_color_pco( 63488 );
    //NXDY_err.setText( "ERROR time NOT synchronized with NTP server." );
    //onErrorAction();
    //resetAVR();
  //}
  // 20190528. Try to do twice...
  nt.syncTime();
  delay( 500 );
  nt.syncTime();
  
  tm0 = millis();
  tm1 = millis();
  NXDY_page1.show();
  NXDY_dln.setText( (String( "Line " ) + String( eepromData.eepromContent.line )).c_str() );
  
  // That's it
  Serial.println( "EndOfInit" );
  while( Serial.available()>0) { char r = Serial.read(); }; // clean input serial buffer
  delay( 100 );
}

// +--------------+
// | Arduino Loop |
// +--------------+
void loop() {

  nexLoop( nex_listen_list );

  if( ( millis() - tm1 ) > dataUpdatePeriod && !NXDY_inSetup ) {  // slow down loop frequency
    // Do a readout
    float meanc[5], V[5], I[5];
    String outStr[5];
    String outAll = "D" + String( year() ) + nt.twoDigits( month() ) + nt.twoDigits( day() ) +
             "T" + nt.twoDigits( hour() ) + nt.twoDigits( minute() ) + nt.twoDigits( second() ); 
    for( int j=0; j<5; j++ ) {
      meanc[j] = 0;
      for( int i=0; i<16; i++ ) {
        meanc[j] += analogRead( sensorPins[j] );
      }
      meanc[j] /= 16;
      V[j] = float( meanc[j] ) * vRef / adcMax; // V in Volts
      I[j] = V[j] / resistor * 1000.;           // I in milliAmperes
      if( I[j] > 2.0 ) {
        sensor[j] = eepromData.eepromContent.slope[j] * I[j] + eepromData.eepromContent.inter[j];
        outStr[j] = String( sensor[j], resolutions[j] );
        if( j==0 ) {
          if(      sensor[j] > TPAlarmThreshold )      { NXDY_sn[j].Set_font_color_pco( NXDYColors.red    ); } // Red text if above threshold 
          else if( sensor[j] > TPAlarmThreshold*1.05 ) { NXDY_sn[j].Set_font_color_pco( NXDYColors.orange ); } // Orange text if above threshold -5% (here the threshold is assumed to be below zero)
          else if( sensor[j] > TPAlarmThreshold*1.1 )  { NXDY_sn[j].Set_font_color_pco( NXDYColors.yellow ); } // Yellow test if above threshold -10%
          else                                         { NXDY_sn[j].Set_font_color_pco( NXDYColors.white  ); } // White text if below threshold -10%
        }
        if( j==1 ) {
          if(      sensor[j] > TPAlarmThreshold )      { NXDY_sn[j].Set_font_color_pco( NXDYColors.drkred  ); } // Dark Red text if above threshold 
          else if( sensor[j] > TPAlarmThreshold*1.05 ) { NXDY_sn[j].Set_font_color_pco( NXDYColors.drkora  ); } // Dark Orange text if above threshold -5% (here the threshold is assumed to be below zero)
          else if( sensor[j] > TPAlarmThreshold*1.10 ) { NXDY_sn[j].Set_font_color_pco( NXDYColors.drkyel  ); } // Olive text if above threshold -10%
          else                                         { NXDY_sn[j].Set_font_color_pco( NXDYColors.drkgray ); } // Dark Gray text if below threshold -10%
        }
        NXDY_sn[j].setText( String( sensor[j], resolutions[j] ).c_str() );
      }
      else {
        outStr[j] = "------";
        NXDY_sn[j].setText( "------\n" );
      }
      // Careful! output buffer is 64 bytes...
      outAll += "M" + outStr[j];
    }
    outAll += "E";
    
    checkTp(); // Act on relays
    
    // Check for an IP client request...
    EthernetClient client = server.available();
    if( client ) {
      byte c = 1; byte i = 0;
      //while( c != 255 && i<10 ) {
      while( c != 255 && i<256 ) {
        c = client.read();
        buff[i] = char( c );
        i++;
      }
      buff[i-1] = '\0';
      if( strncmp( buff, "data", 4 ) == 0 ) {
        server.write( outAll.c_str() );
      }
      else if( strncmp( buff, "reset", 5 ) == 0 ) {
        resetAVR();
      }
      else if( strncmp( buff, "vers", 4 ) == 0 ) {
        String version = String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "." + String(VERSION_REV);
        server.write( version.c_str() );
      }
      else if( strncmp( buff, "give", 5 ) == 0 ) {
        sendEEPROMDataToRemote( 2 );
      }
      else if( strncmp( buff, "get", 3 ) == 0 ) {
        bool bstatus = getEEPROMDataFromRemote( buff+3 );
        if( bstatus ) {
          if( !eepromData.saveToEEPROM() ) {
            server.write( "Error writing to eeprom" );
            NXDY_page3.show();
            NXDY_err.Set_font_color_pco( 63488 );
            NXDY_err.setText( "ERROR writing to EEPROM" );
            onErrorAction();
          }
          else {
            server.write( "Done. Now rebooting myself..." );
          }
          resetAVR(); // Things changed so...   
        }
        else {
          server.write( "Error in decoding the parameters string." );
        }
      }
    } 

    // Check for a Serial request...
    if( Serial.available() ) {
      String s = Serial.readString();
      if( s.substring(0,4).compareTo( String( "data" ) ) == 0 ) {
        int availableBytes = Serial.availableForWrite();
        if( availableBytes >= outAll.length() ) {
          Serial.println( outAll.substring(0,availableBytes) );
          while( Serial.available()>0) { char r = Serial.read(); }; // clean input serial buffer
        }
      }
      else if( s.substring(0,5).compareTo( String( "reset" ) ) == 0 ) {
        resetAVR();
      }
      else if( s.substring(0,4).compareTo( String( "vers" ) ) == 0 ) {
        String version = String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "." + String(VERSION_REV);
        int availableBytes = Serial.availableForWrite();
        if( availableBytes >= version.length() ) {
          Serial.println( version.substring(0,availableBytes) );
          while( Serial.available()>0) { char r = Serial.read(); }; // clean input serial buffer
        }
      }
      else if( s.substring(0,5).compareTo( String( "give" ) ) == 0 ) {
        sendEEPROMDataToRemote( 1 );
        Serial.println( "" );
      }
      else if( s.substring(0,3).compareTo( String( "get" ) ) == 0 ) {
        strncpy( buff, s.c_str()+3, s.length()-3 );
        bool bstatus = getEEPROMDataFromRemote( buff );
        if( bstatus ) {
          if( !eepromData.saveToEEPROM() ) {
            Serial.println( "Error writing to eeprom" );
            NXDY_page3.show();
            NXDY_err.Set_font_color_pco( 63488 );
            NXDY_err.setText( "ERROR writing to EEPROM" );
            onErrorAction();
          }
          else {
            Serial.println( "Done. Now rebooting myself..." );
          }
          resetAVR(); // Things changed so...   
        }
        else {
          Serial.println( "Error in decoding the parameters string." );
        } 
      }
    }
    nt.updateTime();
    tm1 = millis();
  }
  if( ( millis() - tm0 ) > ntpUpdatePeriod ) { 
    nt.syncTime();
    tm0 = millis();
  }
}
