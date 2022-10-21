// +-------------------------------------+
// | Sends a reset command to a 4ventino |
// | Benigno Gobbo - INFN Trieste        |
// | V1.0 20190214                       |
// +-------------------------------------+

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

  std::string command =  "reset";
  
  try {    
    client.clientConnect( mode, param[0], param[1] );
    bool status = false; int count = 0;
    if( mode == 1 ) { 
      std::cout << "\033[1;33mTrying to reset 4ventino module with FTDI s/n "
		<< param[1] << "\033[0m" << std::endl;
    }
    else {
      std::cout << "\033[1;33mTrying to reset 4ventino module with address "
		<< param[0] << "\033[0m" << std::endl;
    } 
    client.clientWrite( command );
  } catch( std::string error ) {
    std::cout << "\033[31mError: " << error << "\033[0m" << std::endl;
    return 1;
  }
      
  return 0;
  
}
