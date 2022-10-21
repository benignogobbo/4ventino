// +--------------------------------------------------------+
// | Console for 4ventinoEEPROM                             |
// | Benigno Gobbo                                          |
// | © INFN Sezione di Trieste                              |
// | V1.0                                                   |
// | 2013-10-25                                             |
// +--------------------------------------------------------+

#include <cstdlib>
#include <pthread.h>
#include "console.h"

std::string consoleText = "";

ConsoleStream::ConsoleStream( std::ostream& stream ) : _stream( stream ) {
  _oldBuf = stream.rdbuf();
  stream.rdbuf( this );
}


ConsoleStream::~ConsoleStream( void ) {
  _stream.rdbuf( _oldBuf );
}


int ConsoleStream::overflow( int_type v ) {
  if (v == '\n') {
    consoleText.append( _infoStr );
    _infoStr.clear();
  } else _infoStr.push_back(v);
  return v;
}


std::streamsize ConsoleStream::xsputn( const char* p, std::streamsize n ) {
  _infoStr.append(p, p + n);
  size_t pos = 0;
  while (pos != std::string::npos) {
    pos = _infoStr.find('\n');
    if (pos != std::string::npos) {
      std::string tmp(_infoStr.begin(), _infoStr.begin() + pos);
      consoleText.append( tmp );
      _infoStr.erase(_infoStr.begin(), _infoStr.begin() + pos + 1);
    }
  }
  return n;
}
     
void Console::startConsoleStream( void ) {
  pthread_t consoleThread;
  if( pthread_create( &consoleThread, NULL, &Console::consoleStreamHelper, this ) ) {
    std::cout << "Error creating Console Stream thread" << std::endl;
    exit(1);
  }
}
