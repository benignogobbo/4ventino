// +--------------------------------------+
// | Serial class for the 4ventino client |
// | Benigno Gobbo - INFN Trieste         |
// | V1.1 20191218                        |
// +--------------------------------------+

#ifndef QVENTINOSERIAL_H
#define QVENTINOSERIAL_H

#include <string>
#include <termios.h>

class QventinoSerial {

 public:
  QventinoSerial( void ) {};
  ~QventinoSerial( void ) {}
  void        serialConnect( std::string prodName, std::string prodSN );
  void        serialDisconnect( void );
  void        serialReconnect( std::string prodName, std::string prodSN );
  void        serialWrite( std::string ); 
  std::string serialRead( void );
  std::string serialFastRead( void );
  void        scaleReadTimeout( float );
  void        resetReadTimeout( void );
  bool        handleReceivedData( std::string );
  
 private:
  std::string    _devName;
  std::string    _devSerial;
  int            _fd;
  double         _readTimeout;
  char           _buff[256];
  struct termios _oldtio;
  struct termios _newtio;
  };

#endif //QVENTINOSERIAL_H
