// +---------------------------------------+
// | Client class for the 4ventino         |
// | Benigno Gobbo - INFN Trieste          |
// | V1.4 20200217                         |
// +---------------------------------------+


#ifndef QVENTINOCLIENT_H
#define QVENTINOCLIENT_H

#include <string>
#include <ctime>

class QventinoSerial;
class QventinoEthernet;

class QventinoClient {
 public:
  QventinoClient() {}
  void               clientConnect( int mode, std::string param1 = "", std::string param2 = "" );
  void               clientDisconnect( void );
  void               clientReconnect( int mode, std::string param1 = "", std::string param2 = "" );
  void               clientWrite( std::string s );
  std::string        clientRead( void );
  bool               dataDecode( std::string );
  void               dataDump( void );
  inline int         getYear( void )   { return( _year ); }
  inline int         getMonth( void )  { return( _month ); }
  inline int         getDay( void )    { return( _day ); }
  inline int         getHour( void )   { return( _hour ); }
  inline int         getMinute( void ) { return( _minute ); }
  inline int         getSecond( void ) { return( _second ); }
  inline std::time_t getSecondsSinceEpoch( void ) { return( _sinceEpoch ); }
  float              getData( int i );
  QventinoSerial*    getSerialClient( void )   { if( _mode ==  1 ) return( &_serial );   else return( NULL ); }
  QventinoEthernet*  getEthernetClient( void ) { if( _mode ==  2 ) return( &_ethernet ); else return( NULL ); } 
  
 private:
  int              _mode;
  std::string      _param[2];
  QventinoSerial   _serial;
  QventinoEthernet _ethernet;
  int              _year;
  int              _month;
  int              _day;
  int              _hour;
  int              _minute;
  int              _second;
  std::time_t      _sinceEpoch;   
  float            _data[5];
  
};

#endif //QVENTINOCLIENT_H
