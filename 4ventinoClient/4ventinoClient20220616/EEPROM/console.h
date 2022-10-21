#ifndef Console_h
#define Console_h
// +--------------------------------------------------------+
// | Console for 4ventinoEEPROM                             |
// | Benigno Gobbo                                          |
// | © INFN Sezione di Trieste                              |
// | V1.0                                                   |
// | 2013-10-25                                             |
// +--------------------------------------------------------+

#include <iostream>
#include <streambuf>
#include <string>

extern std::string consoleText;

class ConsoleStream : public std::basic_streambuf< char, std::char_traits< char > > {

  typedef std::basic_streambuf< char, std::char_traits< char > >::int_type int_type;
  typedef std::char_traits< char > traits_t;

 public:

  ConsoleStream( std::ostream& );

  ~ConsoleStream( void );

  void startConsoleStream( void );

 protected:

  virtual int_type overflow( int_type );
  virtual std::streamsize xsputn( const char*, std::streamsize );

 private:

  std::ostream&   _stream;
  std::streambuf* _oldBuf;
  std::string     _infoStr;

};

class Console {

 public:

  Console( void ) {}
  ~Console( void ) {}

  void startConsoleStream( void );

 private:

  ConsoleStream*      _consoleStream;

  void *theConsoleStream( void ) { _consoleStream = new ConsoleStream( std::cout ); return NULL; }
  static void *consoleStreamHelper( void *c ) { return( ( (Console*)c )->theConsoleStream() ); }

};

#endif // Console_h
