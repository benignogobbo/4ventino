// +-----------------------------------------------+
// | A single test program for the 4ventino client |
// | Benigno Gobbo - INFN Trieste                  |
// | V1.1 20190604                                 |
// +-----------------------------------------------+

#include <iostream>
#include <exception>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include "qventinoUtils.h"
#include "qventino.h"

// <><><><><><> help...
void usage( char *cp ) {
  std::cout << std::endl
	    << "usage: " << cp << " -e  -a address -p port" << std::endl
	    << "       " << cp << " --ethernet  --address address --port port" << std::endl
	    << "       " << cp << " -s -t product_type -n serial_number " << std::endl
	    << "       " << cp << " --serial --serial-producs product_type --serial-number serial_number"  << std::endl
	    << "    -e, --ethernet         comunicate via ethernet" << std::endl
	    << "    -a, --address          ethernet server address" << std::endl
	    << "    -p, --port             ethernet comunication port" << std::endl
	    << "    -s, --serial           comunicate via serial port" << std::endl
    	    << "    -t, --serial-product   serial product name" << std::endl
    	    << "    -n, --serial-number    product serial number" << std::endl
	    << std::endl;
}

// <><><><><><> Main...
int main( int argc, char *argv[] ) {

  int mode = 0;
  std::string param[2] = { "", "" };
  const int sleepSecs = 5;
  
  static struct option longOptions[] = {
    { "ethernet",       no_argument,       0, 'e' },
    { "address",        required_argument, 0, 'a' },
    { "port",           required_argument, 0, 'p' },
    { "serial",         no_argument,       0, 's' },
    { "serial-product", required_argument, 0, 't' },
    { "serial-number",  required_argument, 0, 'n' },
    { "help",           no_argument,       0, 'h' }
  };

  int   rc = 0;
  char* cp;
  if( ( cp = (char*) strrchr( argv[0], '/' )) != NULL ) {
    ++cp;
  }
  else {
    cp = argv[0];
  }

  if( argc > 1 ) {
    int longIndex = 0;
    while( ( rc = getopt_long( argc, argv, "ea:p:st:n:h", longOptions, &longIndex ) ) != EOF ) {
      switch( rc ) {
        case 'e': mode = 2;
	  break;
        case 'a': param[0] = optarg;
	  break;
        case 'p': param[1] = optarg;
	  break;
        case 's': mode = 1;
	  break;
        case 't': param[0] = optarg;
  	  break;
        case 'n': param[1] = optarg;
	  break;
        case 'h': usage( cp );
	  return 0;
	  break;
        default: usage( cp );
	  return 1;
	  break;
      }
    }
  }
  else {
    usage( cp );
    return 1;
  }

  if( mode < 1 || mode > 2 ) {
    usage( cp );
    return 1;
  }

  if( param[0] == "" || param[1] == "" ) {
    usage( cp );
    return 1;
  }

  QventinoClient client;

  std::string command =  "vers";
    
  std::cout << "\033c";
  
  try {
    
    client.clientConnect( mode, param[0], param[1] );
    usleep( 500000 );
    client.clientWrite( command );
    usleep( 500000 );
    std::string vers = client.clientRead();
    std::cout << std::endl << "\033[1;32mFirmware Version: \033[1;33m" << vers << "\033[0m" << std::endl;
    sleep(2);
    std::cout << "\033c";

    command = "data";
    
    while( true ) {

      bool status = false; int count = 0;

      while( !status && count++ < 5 ) { 

	client.clientWrite( command );
	usleep( 300000 );
	std::string s = client.clientRead();
	status = client.dataDecode( s );
	if( status ) {
	  client.dataDump();
	}
	else {
	  usleep( 500000 );
	}
      }
      if( status == false ) {
	std::cout << "\033[31mError: no correct serial data collected after five attempts.\033[0m";
	return 1;
      }
      QventinoUtils::sleep( sleepSecs );
    }
  } catch( std::string error ) {
    std::cout << "\033[31mError: " << error << "\033[0m" << std::endl;
    return 1;
  }
      
  return 0;
  
}
