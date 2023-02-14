// +---------------------------------------------------+
// | The 4ventino firmware. Nextion display class      |
// | Benigo Gobbo                                      |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+
#ifndef NXDYDISPLAY_H
#define NXDYDISPLAY_H

#include <Nextion.h> // For ATmega1284 define nexSerial as Serial1 in NexConfig.h. Comment out DEBUG output too!
#include "eepromdata.h"
#include "utilities.h"
EEPROMData eepromData;

// Nextion Display 

// NB. Nextion colors: (red>>3)<<11 + (green>>2)<<5 + blue>>3
struct NXDYColorsDefs { uint32_t         red,    lime,    blue,  yellow, magenta,    cyan,   black,   white,  orange,  drkred,  drkora,  drkyel, drkgray; };
const  NXDYColorsDefs NXDYColors = {   63488,    2016,      31,   65504,   63519,    2047,       0,   65535,   64800,   38912,   39520,   40128,   25356 }; 
// RGB:                               FF0000,  00FF00,  0000FF,  FFFF00,  FF00FF,  00FFFF,  000000,  FFFFFF,  FFA500,  990000,  994C00,  999900,  606060
     
NexPage     NXDY_page0 = NexPage( 0,  0, "page0" );

NexPage     NXDY_page1 = NexPage( 1,  0, "page1" );
NexText     NXDY_sn[5] = { NexText( 1,  1, "t0" ), NexText( 1,  3, "t2" ), NexText( 1,  8, "t7" ), NexText( 1, 14, "t13" ), NexText( 1, 11, "t10" ) };
NexText     NXDY_tmd   = NexText( 1, 16, "t15" );
NexText     NXDY_dln   = NexText( 1, 18, "t16" );
NexButton   NXDY_sup   = NexButton( 1, 17, "b0" );
NexVariable NXDY_sus   = NexVariable( 1, 19, "va0" );

NexPage     NXDY_page2 = NexPage( 2, 0, "page2" );
NexNumber   NXDY_ip[4] = { NexNumber( 2,  6, "n0" ),  NexNumber( 2,  7, "n1" ),  NexNumber( 2,  8, "n2" ),  NexNumber( 2,  9, "n3" )  };
NexNumber   NXDY_dn[4] = { NexNumber( 2, 10, "n4" ),  NexNumber( 2, 11, "n5" ),  NexNumber( 2, 12, "n6" ),  NexNumber( 2, 13, "n7" )  };
NexNumber   NXDY_gw[4] = { NexNumber( 2, 14, "n8" ),  NexNumber( 2, 15, "n9" ),  NexNumber( 2, 16, "n10" ), NexNumber( 2, 17, "n11" ) };
NexNumber   NXDY_nm[4] = { NexNumber( 2, 18, "n12" ), NexNumber( 2, 19, "n13" ), NexNumber( 2, 20, "n14" ), NexNumber( 2, 21, "n15" ) };
NexText     NXDY_ntp =   NexText( 2, 70, "t33" );
NexNumber   NXDY_lin   = NexNumber( 2, 61, "n16" );
NexText     NXDY_lo[5] = { NexText( 2, 28, "t11" ), NexText( 2, 29, "t12" ), NexText( 2, 30, "t13" ), NexText( 2, 31, "t14" ), NexText( 2, 32, "t15" ) };
NexText     NXDY_hi[5] = { NexText( 2, 38, "t21" ), NexText( 2, 39, "t22" ), NexText( 2, 40, "t23" ), NexText( 2, 41, "t24" ), NexText( 2, 42, "t25" ) };
NexText     NXDY_psk   = NexText( 2, 65, "t30" );
NexButton   NXDY_can   = NexButton( 2, 56, "b12" );
NexButton   NXDY_sav   = NexButton( 2, 57, "b13" );
NexVariable NXDY_scs   = NexVariable( 2, 66, "va1" );
NexCheckbox NXDY_sleep = NexCheckbox( 2, 67, "c0" );

NexPage     NXDY_page3 = NexPage( 3, 0, "page3" );
NexText     NXDY_err   = NexText( 3, 2, "t0" );
NexButton   NXDY_git   = NexButton( 3, 1, "b0" );
NexButton   NXDY_rst   = NexButton( 3, 4, "b1" );
NexButton   NXDY_sup2  = NexButton( 3, 5, "b2" );
NexVariable NXDY_gir   = NexVariable( 3, 3, "va0" );

NexPage     NXDY_page4 = NexPage( 4, 0, "page4" );
NexText     NXDY_pat   = NexText( 4, 2, "t1" );
NexButton   NXDY_okp   = NexButton( 4, 14, "b11" );
NexVariable NXDY_pws   = NexVariable( 4, 15, "va0" );

NexPage     NXDY_page5 = NexPage( 5, 0, "page5" );

//NexTouch *nex_listen_list[] = { &NXDY_sup, &NXDY_sav, &NXDY_can, &NXDY_git, &NXDY_rst, &NXDY_okp, NULL };
NexTouch *nex_listen_list[] = { &NXDY_sup, &NXDY_sav, &NXDY_can, &NXDY_okp, &NXDY_sup2, NULL };

bool NXDY_inSetup = false;

// +-----------------------------------+
// | Go to setup page passwork request |
// +-----------------------------------+
void NXDY_displaySetupPw( void *ptr ) {
  NXDY_inSetup = true;
  NXDY_page4.show();
}

// +----------------------+
// | Go to the setup page |
// +----------------------+
void NXDY_displaySetupPage( void ) {
  NXDY_inSetup = true;
  NXDY_page2.show();
  NXDY_lin.setValue( eepromData.eepromContent.line );
  NXDY_psk.setText( eepromData.eepromContent.passkey );
  for( int i=0; i<4; i++ ) {
    NXDY_ip[i].setValue( eepromData.eepromContent.ip[i] );
    NXDY_dn[i].setValue( eepromData.eepromContent.dns[i] );
    NXDY_gw[i].setValue( eepromData.eepromContent.gw[i] );
    NXDY_nm[i].setValue( eepromData.eepromContent.snm[i] );
  }
  NXDY_ntp.setText( eepromData.eepromContent.ntp );
  for( int i=0; i<5; i++ ) {
    NXDY_lo[i].setText( String(  4.0*eepromData.eepromContent.slope[i] + eepromData.eepromContent.inter[i] ).c_str() );
    NXDY_hi[i].setText( String( 20.0*eepromData.eepromContent.slope[i] + eepromData.eepromContent.inter[i] ).c_str() );
  }
  NXDY_sleep.setValue(  eepromData.eepromContent.sleep );
}

// +------------------------------+
// | Verify Setup access apssword |
// +------------------------------+
void NXDY_displaySetupCheckPw( void *ptr ) {
  char buff[5];
  int len = NXDY_pws.getText( buff, 5 );
  if( strncmp( buff, eepromData.eepromContent.passkey, 4 ) == 0 ) {
    NXDY_displaySetupPage();
  }
  else {
    NXDY_inSetup = false;
    NXDY_page1.show();
    NXDY_dln.setText( (String( "Line " ) + String( eepromData.eepromContent.line )).c_str() );
  }
}

// +-----------------------------+
// | Save all settings on eeprom |
// +-----------------------------+
void NXDY_saveSettings( void ) {
  strcpy( eepromData.eepromContent.init, "done" );
  eepromData.eepromContent.init[4] = '\0';
  NXDY_lin.getValue( (uint32_t*)&(eepromData.eepromContent.line) );
  memcpy( eepromData.eepromContent.mac, eepromData.macBase, 6 );
  eepromData.eepromContent.mac[5] = eepromData.eepromContent.mac[5] + eepromData.eepromContent.line;
  uint16_t gotLen = NXDY_psk.getText( eepromData.eepromContent.passkey, 5 );
  eepromData.eepromContent.passkey[gotLen] = '\0';
  uint32_t dummy;
  for( int i=0; i<4; i++ ) {
    NXDY_ip[i].getValue( &dummy );
    eepromData.eepromContent.ip[i]  = dummy & 0xff;
    NXDY_dn[i].getValue( &dummy );
    eepromData.eepromContent.dns[i] = dummy & 0xff;
    NXDY_gw[i].getValue( &dummy );
    eepromData.eepromContent.gw[i]  = dummy & 0xff;
    NXDY_nm[i].getValue( &dummy );
    eepromData.eepromContent.snm[i] = dummy & 0xff;
  }
  gotLen = NXDY_ntp.getText( eepromData.eepromContent.ntp, 32 );
  eepromData.eepromContent.ntp[gotLen] = '\0';
  for( int i=0; i<5; i++ ) {
    char buff[12]; String ss; float val[2];
    memset(buff, 0, sizeof(buff));
    gotLen = NXDY_lo[i].getText( buff, 12 );
    buff[gotLen] = '\0';
    ss = buff;
    val[0] = ss.toFloat();
    memset(buff, 0, sizeof(buff));
    gotLen = NXDY_hi[i].getText( buff, 12 );
    buff[gotLen] = '\0';
    ss = buff;
    val[1] = ss.toFloat();
    eepromData.eepromContent.slope[i] = ( val[1] - val[0] ) / 16.0;
    eepromData.eepromContent.inter[i] = ( 5.0 * val[0] - val[1] ) / 4.0;
  }
  NXDY_sleep.getValue( &dummy );
  eepromData.eepromContent.sleep = dummy&0x0F;
}

// +--------------------------------------+
// | If setup "save" button is pressed... |
// +--------------------------------------+
void NXDY_displaySetupSave( void *ptr ) {
  NXDY_inSetup = false;
  NXDY_saveSettings();
  if( !eepromData.saveToEEPROM() ) {
    Serial.println( "Error in writing to EEPROM" );
    NXDY_page3.show();
    NXDY_err.Set_font_color_pco( 63488 );
    NXDY_err.setText( "ERROR writing to EEPROM" );
  }
  resetAVR(); // As things changed a reset wold be better here....  
  //NXDY_page1.show();
  //NXDY_dln.setText( (String( "Line " ) + String( eepromData.eepromContent.line )).c_str() );
}

// +----------------------------------------+
// | If setup "cancel" button is pressed... |
// +----------------------------------------+
void NXDY_displaySetupCancel( void *ptr ) {
  NXDY_inSetup = false;
  NXDY_page1.show();
  NXDY_dln.setText( (String( "Line " ) + String( eepromData.eepromContent.line )).c_str() );
}

// +------------+
// | Initialize |
// +------------+
void NXDY_init( ) {
  nexInit();
  NXDY_sup.attachPop( NXDY_displaySetupPw );
  NXDY_sup2.attachPop( NXDY_displaySetupPw );
  NXDY_okp.attachPop( NXDY_displaySetupCheckPw );
  NXDY_sav.attachPop( NXDY_displaySetupSave );
  NXDY_can.attachPop( NXDY_displaySetupCancel );
  //NXDY_git.attachPop( NXDY_displayGotIt );
  //NXDY_rst.attachPop( NXDY_displayReset );
}

// +--------------------------------------+
// | Display sleeps and waits for a touch |
// +--------------------------------------+
bool NXDY_sleepAndWakeOnTouch( int secs ) {
  if( secs < 3 || secs > 65535 ) {
    return false; 
  }
  String s = "thsp=" + String( secs );  // sleep on no touch
  sendCommand( s.c_str() );
  s = "thup=1";                         // wake on touch
  sendCommand( s.c_str() ); 
}

// +----------------------+
// | Display never sleeps |
// +----------------------+
bool NXDY_neverSleep( void ) {
  String s = "thsp=0";                  // sleep on no touch not active
  sendCommand( s.c_str() );
  s = "thup=0";                         // wake on touch not active
  sendCommand( s.c_str() ); 
}

// +------------------------+
// | Polling for a variable |
// +------------------------+
int NXDY_pollDisplay( NexVariable *var ) {
  uint32_t value = 0;
  do {
    var->getValue( &value );
    delay( 200 );
  } while( value == 0 );
  return( value );
}

#endif //NXDYDISPLAY_H
