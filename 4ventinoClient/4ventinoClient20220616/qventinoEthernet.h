// +----------------------------------------+
// | Ethernet class for the 4ventino client |
// | Benigno Gobbo - INFN Trieste           |
// | V1.1 20191218                          |
// +----------------------------------------+

#ifndef QVENTINOETHERNET_H
#define QVENTINOETHERNET_H

#include <string>
#include <netinet/in.h>
#include <netdb.h>

class QventinoEthernet {

 public:
  QventinoEthernet() {};
  bool        ethernetConnect( std::string hostAddress , int portNo );
  std::string ethernetRead( void );
  int         ethernetWrite( std::string s );
  void        ethernetDisconnect( void );
  bool        ethernetReconnect( std::string hostAddress, int portNo ); 
  
 private:
  int                _port;
  int                _socketFd;
  struct sockaddr_in _serverAddress;
  struct hostent*    _server;
  char               _buff[256];
  };

#endif //QVENTINOETHERNET_H
