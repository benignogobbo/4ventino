// +--------------------------------------+
// | Serial class for the 4ventino client |
// | Benigno Gobbo - INFN Trieste         |
// | V1.1 20191218                        |
// +--------------------------------------+

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <exception>

#include <unistd.h>
#include <sys/types.h>
// serial...
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>

// local stuff
#include "qventinoSerial.h"

// <><><><><><> Scale readout timeout
const float _readTimeout0 = 0.2;
void QventinoSerial::scaleReadTimeout( float scale ) {
  _readTimeout = _readTimeout0 * scale;
  return;
}

// <><><><><><> Reset readout timeout
void QventinoSerial::resetReadTimeout( void ) {
  _readTimeout = _readTimeout0;
  return;
}

// <><><><><><> try to write some characters to serial
void QventinoSerial::serialWrite( std::string data ) {
  int status = write( _fd, data.c_str(), data.size() );
  if( status != data.size() ) {
    throw( std::string( strerror( errno ) ) );
    return;
  }
}

// <><><><><><> try to read something from seriar (slow but safer...)
std::string QventinoSerial::serialRead() {

  fd_set read_fds, write_fds, except_fds;
  FD_ZERO( &read_fds );
  FD_ZERO( &write_fds );
  FD_ZERO( &except_fds );
  FD_SET( _fd, &read_fds );
  struct timeval timeout;

  timeout.tv_usec = modf( _readTimeout, &_readTimeout ) * 1000000; 
  timeout.tv_sec  = _readTimeout;
  resetReadTimeout();
  
  std::string s = "";
  char buff[512];
  int status = 0;
  int rv = 0;
  do {
    if( ( rv = select( _fd+1, &read_fds, &write_fds, &except_fds, &timeout )) ==1 ) {
      status = read( _fd, buff, 1 );
      if( status == 1 ) {
	s += buff[0];
      }
    }
  } while( status > 0 && rv == 1 );

  s[s.length()-1] = '\0';
  return(s);
}

// <><><><><><> try to read something from serial, fast but unsafe
std::string QventinoSerial::serialFastRead() {
  char buff[512];
  int status = read( _fd, buff, sizeof(buff) );
  if( status < 0 ) {
    throw( std::string( strerror( errno ) ) );
    return( std::string( "" ) );
  }
  else {
    return( std::string( buff ) );
  }
}

// <><><><><><> Connect to Serial
void QventinoSerial::serialConnect( std::string prodName, std::string prodSN  ) {

  _devName = prodName;
  _devSerial = prodSN;

  std::cout << "\033c\033[4mLooking for " << _devName << " (s/n "
	    << _devSerial << ") module... \033[0m" << std::endl;
  char buff[256];
  std::string prod = "\"" + _devName + "\"";
  std::string seri = "\"" + _devSerial + "\"";
  std::string comm1 = "udevadm info -a -n ";
  std::string comm2 = " | xargs ";
  std::string comm3 = " | grep ";

  // Look for device:
  std::vector<std::string> files;
  DIR *dp = opendir( "/dev" );
  struct dirent *dirp;
  while( (dirp = readdir(dp))  != NULL ) {
    files.push_back( std::string( dirp->d_name ) );
  }
  std::string s = "ttyUSB";
  std::vector<std::string> devices;

  for( unsigned int i=0; i<files.size(); i++ ) {
    if( files[i].substr(0,6) == s ) {
      devices.push_back( "/dev/" + files[i] );
    }
  }

  std::vector<std::string> foundDev;
  for( unsigned int i=0; i<devices.size(); i++ ) {
    bool found = false;
    std::string command = comm1 + devices[i] + comm2 + comm3 + prod + comm3 + seri;
    FILE* f = popen( command.c_str(), "r" );
    if( f ) {
      while( !feof( f ) ) {
        if( fgets( buff, 256, f ) != NULL && !found ) {
	  found = true;
          foundDev.push_back( devices[i] );
        }
      }
      pclose( f );
    }
  }
  
  if( foundDev.size() != 1 ) {
    std::cout << "\033[31m Found too few or many devices. I need just one. Exit...\033[0m" << std::endl;
    exit(1);
  }
  
  _fd = open( foundDev[0].c_str(), O_RDWR | O_NOCTTY );
  if( _fd < 0 ) {
    throw( std::string( strerror( errno ) ) );
    return;
  }

  tcgetattr( _fd, &_oldtio );
  _newtio = _oldtio;

  cfsetispeed( &_newtio, (speed_t)B9600 );
  cfsetospeed( &_newtio, (speed_t)B9600 );
  //cfmakeraw(   &_newtio );

  _newtio.c_cflag &= ~PARENB;
  _newtio.c_cflag &= ~CSTOPB;
  _newtio.c_cflag &= ~CSIZE;
  _newtio.c_cflag |= CS8;
  _newtio.c_lflag |= ICANON;
  
  tcsetattr( _fd, TCSANOW, &_newtio );
  tcflush( _fd, TCIOFLUSH );

  std::cout << "Wait for Arduino initialisation..." << std::endl;
  int count = 0; bool gotIt = false;
  while( !gotIt && count < 20 ) {
    std::cout << "\033[2K\033[33m" << std::fixed << std::setprecision(1) << float(count)/2. << " seconds\033[0m" << std::endl;
    std::cout << std::resetiosflags( std::ios::fixed ) << std::setprecision(6);
    count+=1;
    std::string s = serialRead();
    if( s.substr(0,9) == "EndOfInit" ) {
      gotIt = true;
    }
    usleep(500000);
    std::cout << "\033[2A" << std::endl;
  }
  if( gotIt ) {
    std::cout << "Got connection..." << std::endl;
    usleep(100000);
  }
  else {
    std::cout << "\033[31mError: Arduino does not comunicate...\033[0m"  << std::endl;      
    exit(1);
  }  
  
  return;
}

// <><><><><><> Restore default Serial Settings and close it
void QventinoSerial::serialDisconnect( void ) {

  tcsetattr( _fd, TCSANOW, &_oldtio ); /* restore old port settings */
  tcflush( _fd, TCIOFLUSH );
  if( close( _fd ) < 0 ) {
    throw ( std::string( strerror( errno ) ) );
  } 
  return;
}

void QventinoSerial::serialReconnect( std::string prodName, std::string prodSN ) {
  serialDisconnect();
  serialConnect(  prodName, prodSN );
  return;
}
