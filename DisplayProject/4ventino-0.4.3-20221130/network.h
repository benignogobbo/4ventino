// +---------------------------------------------------+
// | The 4ventino firmware. Network class              |
// | Benigno Gobbo                                     |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+
#ifndef NETWORK_H
#define NETWORK_H

#define USE_DS3231_RTC      // RTC options
//#define USE_DS1302_RTC
//#define USE_NEXTION_RTC

#include <TimeLib.h>        // From https://github.com/PaulStoffregen/Time
#include <Ethernet.h>       // IMPORTANT!: this if from https://github.com/PaulStoffregen/Ethernet
#include <EthernetUdp.h>    // Needed by NTP

#ifdef USE_DS3231_RTC
#  include <DS1307RTC.h>    // From https://github.com/PaulStoffregen/DS1307RTC
#else
#  ifdef USE_DS1302_RTC
#    include <DS1302RTC.h>  // See: https://playground.arduino.cc/Main/DS1302RTC
#  endif
#endif

const int           TCPIPCommunicationPort = 57777;
const int           UDPCommunicationPort   = 8888;
//const char          NTPServer[]            = "it.pool.ntp.org";
//const char          NTPServer[]            = "192.168.69.1";
char                NTPServer[32];
const int           NTP_PACKET_SIZE        = 48;           // NTP time is in the first 48 bytes of message
const unsigned long MCMDCC_MCM             = 2208988800UL; // 1900-01-01T00:00/1970-01-01T00:00
const long          TIME_ZONE              = +1;           // CET

// Ethernet Setup, TPCIP
EthernetServer server = EthernetServer( TCPIPCommunicationPort );
// NTP
EthernetUDP udp;

#ifdef USE_DS1302_RTC
// RTC. Pins: CE, IO, SCLK
DS1302RTC RTC( 19, 20, 21 );
#endif 
#ifdef USE_NEXTION_RTC
NexRtc NXDY_rtc;
uint32_t time_data[7];
char  time_buf[30] = {0}; 
#endif

// +---------------------------+
// | NTP time adjustment class |
// +---------------------------+
class NetworkTime {
  public:
    NetworkTime() {};
    String timeString( void );
    String twoDigits( int );
    time_t getNtpTime( void );
    void sendNTPpacket( const char* address );
    bool syncTime( void );
    void updateTime( void );
  private:
    byte packetBuffer[NTP_PACKET_SIZE];
};

NetworkTime nt;

String NetworkTime::timeString() {
  String timedate = String( year() ) + "-";
  timedate += twoDigits( month() ) + "-";
  timedate += twoDigits( day() ) + " ";
  timedate += twoDigits( hour() ) + ":";
  timedate += twoDigits( minute() ) + ":";
  timedate += twoDigits( second() ); 
  return( timedate );
}

// +--------------------------+
// | number (<99) in 2 digits |
// +--------------------------+
String NetworkTime::twoDigits( int number ) {
  String s;
  if( number < 10 ) {
    s = '0' + String( number );
  }
  else {
    s = String( number );
  }
  return( s );
}

// +--------------------+
// | NTP sychronization |
// +--------------------+
time_t NetworkTime::getNtpTime( void ) {
  while( udp.parsePacket() > 0 ); // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  strcpy(  NTPServer, eepromData.eepromContent.ntp );
  sendNTPpacket( NTPServer );
  uint32_t beginWait = millis();
  while( millis() - beginWait < 1500 ) {
    int size = udp.parsePacket();
    if( size >= NTP_PACKET_SIZE ) {
      //Serial.println("Receive NTP Response");
      udp.read( packetBuffer, NTP_PACKET_SIZE );  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.print( "Since EPOCH: " ); Serial.println( secsSince1900 - MCMDCC_MCM + NTPTimeZone * SECS_PER_HOUR );
      return( secsSince1900 - MCMDCC_MCM + TIME_ZONE * SECS_PER_HOUR );
    }
  }
  //Serial.println( "No NTP Response" );
  return 0; // return 0 if unable to get the time
}

// +----------------------------+
// | NTP request to time server |
// +----------------------------+
void NetworkTime::sendNTPpacket( const char* address ) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  udp.beginPacket( address, 123 ); //NTP requests are to port 123
  udp.write( packetBuffer, NTP_PACKET_SIZE );
  udp.endPacket();
}

// +------------------+
// | Synchronize time |
// +------------------+
bool NetworkTime::syncTime( void ) {
  time_t thisMoment = 0;
  delay( random( 500 ) );
  const time_t s20100101 = 1262300400UL;
  int count = 0;
  while( thisMoment < s20100101 && count++ < 5 ) { 
    thisMoment = getNtpTime();
    if( thisMoment < s20100101 ) delay( random( 100 ) + 100 ); // random wait 0.1 - 0.2 s 
  }
  if( thisMoment < s20100101 ) return( false );
  //Serial.println( String( thisMoment ) + ", year: " + String( year( thisMoment ) ) );
# ifdef USE_NEXTION_RTC
  time_data[0] = year( thisMoment );
  time_data[1] = month( thisMoment );
  time_data[2] = day( thisMoment );
  time_data[3] = hour( thisMoment );
  time_data[4] = minute( thisMoment );
  time_data[5] = second( thisMoment );
  NXDY_rtc.write_rtc_time( time_data );
# else
  setSyncProvider( RTC.get );
  RTC.set( thisMoment );
# endif

  return true;
}

// +-------------+
// | Update time |
// +-------------+
void NetworkTime::updateTime( void ) {
# ifdef USE_NEXTION_RTC
  NXDY_rtc.read_rtc_time( time_buf, 30 );
  time_buf[4]  = '-'; time_buf[7] = '-';
  time_buf[10] = ' '; time_buf[19] = '\0';
  NXDY_tmd.setText( time_buf );
#else
  if( timeStatus() != timeNotSet ) {
    NXDY_tmd.setText( timeString().c_str() );
  }
  else {
    NXDY_tmd.setText( "-------------------" );
  }
# endif
}

#endif //NETWORK_H
