// +----------------------------------------+
// | Ethernet class for the 4ventino client |
// | Benigno Gobbo - INFN Trieste           |
// | V1.1 20191218                          |
// +----------------------------------------+

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <exception>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "qventinoEthernet.h"

// <><><><><><> initialize ethernet socket
bool QventinoEthernet::ethernetConnect( std::string hostAddress, int portNo ) {

  _port = portNo;
  
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if( _socketFd < 0 ) {
    std::stringstream ss; ss << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: opening socket failure.\033[0m";
    throw( ss.str() );
    return( false );
  }
 
  _server = gethostbyname( hostAddress.c_str() );
  if( _server == NULL ) {
    std::stringstream ss; ss << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: host not found.\033[0m";
    throw( ss.str() );
    return( false );
  }

  bzero( (char *) &_serverAddress, sizeof( _serverAddress ) );
  _serverAddress.sin_family = AF_INET;
  bcopy( (char *)_server->h_addr, (char *)&_serverAddress.sin_addr.s_addr, _server->h_length );
  _serverAddress.sin_port = htons( _port);

  if( connect( _socketFd, (struct sockaddr *) &_serverAddress, sizeof(_serverAddress) ) < 0 ) {
    std:: stringstream ss; ss << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: connection failed.\033[0m";
    throw( ss.str() );
    return( false );
  }
  //std:: cout << "\033[1;33m[" << __FUNCTION__ << "] Connected to the net....\033[0m" << std::endl;
  return( true );
}

// <><><><><><> get from net
std::string QventinoEthernet::ethernetRead( void ) {
  bzero( _buff, 256 );
  int n = read( _socketFd, _buff, 255 );
  if( n < 0 ) {
    std::stringstream ss; ss << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: read from socket failure.\033[0m";
    throw( ss.str() );
    return( std::string( "" ) );
  }
  std::string s = _buff;
  s += "\n\r";
  s[s.length()-1] = '\0';
  return( s ); 
}

// <><><><><><> send to net
int QventinoEthernet::ethernetWrite( std::string s ) {
  int n = write( _socketFd, s.c_str(), s.size() );
  if( n < 0 ) {
    std::stringstream ss; ss << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: write to socket failure.\033[0m";
    throw( ss.str() );
  }
  return n;
}

// <><><><><><> close the socket
void QventinoEthernet::ethernetDisconnect( void ) {
  if( close( _socketFd ) < 0 ) {
    throw ( std::string( strerror( errno ) ) );
  }
  return;
}

// <><><><><><> reconnect to ethernet
bool QventinoEthernet::ethernetReconnect( std::string hostAddress, int portNo ) {
  ethernetDisconnect();
  return( ethernetConnect( hostAddress, portNo ) );
}
