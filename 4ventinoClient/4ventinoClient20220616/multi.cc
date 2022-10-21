// +--------------------------------------------------------+
// | A multiple access test program for the 4ventino client |
// | Benigno Gobbo - INFN Trieste                           |
// | V1.1 20190604                                          |
// | V1.1.1 20190903 - Bug fix                              |
// +--------------------------------------------------------+

#include <iostream>
#include <exception>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include "qventino.h"

const std::string command   =  "data";
const std::string IPaddr[7] = { "192.168.69.11", "192.168.69.12", "192.168.69.13", "192.168.69.14", "192.168.69.15", "192.168.69.16", "192.168.69.17" };
const std::string IPport    = "57777";
const std::string UARTpn    = "FT232R USB UART";
const std::string UARTsn[7] = { "AH06GM8C", "A106QUOF", "A9078H15", "AH06GKUP", "A9078G9H", "AH06GLG2", "AH06GM2D" };
const int         toRead = 5;

// <><><><><><> help...
void usage( char *cp ) {
  std::cout << std::endl
	    << "usage: " << cp << " -e"<< std::endl
	    << "       " << cp << " --ethernet"<< std::endl
	    << "       " << cp << " -s" << std::endl
	    << "       " << cp << " --serial"  << std::endl
	    << "    -e, --ethernet         comunicate via ethernet" << std::endl
	    << "    -s, --serial           comunicate via serial port" << std::endl
    	    << "    -h, --help             this help" << std::endl
	    << std::endl;
}

// <><><><><><> Main...
int main( int argc, char *argv[] ) {

  int mode = 0;
  std::string param[2] = { "", "" };
  const int sleepSecs = 5;
  
  static struct option longOptions[] = {
    { "ethernet",       no_argument,       0, 'e' },
    { "serial",         no_argument,       0, 's' },
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
    while( ( rc = getopt_long( argc, argv, "esh", longOptions, &longIndex ) ) != EOF ) {
      switch( rc ) {
        case 'e': mode = 2;
	  break;
        case 's': mode = 1;
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

  QventinoClient client[6];

    
  std::cout << "\033c";
  
  try {
    
    for( int i=0; i<toRead; i++ ) {
      if( mode == 1 ) {
	param[0] = UARTpn;
	param[1] = UARTsn[i];
	client[i].clientConnect( mode, param[0], param[1] );
      }
      else if( mode == 2 ) {
	param[0] = IPaddr[i];
	param[1] = IPport;
	client[i].clientConnect( mode, param[0], param[1] );
      }
      else {
	usage( cp );
	return 1;
      }
    }
    
    while( true ) {
      for( int i=0; i<toRead; i++ ) {
	bool status = false; int count = 0;
	while( !status && count++ < 5 ) {
	  std::cout << "Status: " << status << ", count: " << count << std::endl;
	  client[i].clientWrite( command );
	  usleep( 300000 );
	  std::string s = client[i].clientRead();
	  if( mode == 1 ) {
	    std::cout << std::endl << "\033[1;34mSerial test. UART s/n " << UARTsn[i] << "\033[0m" << std::endl;
	  }
	  else {
	    std::cout << std::endl << "\033[1;34mEthernet test. IP address " << IPaddr[i] << "\033[0m" << std::endl;
	  }
	  status = client[i].dataDecode( s );
	  if( status ) {
	    client[i].dataDump();
	  }
	  else {
	    usleep( 500000 );
	  }
	}
	if( status == false ) {
	  std::cout << "\033[31mError: no correct serial data collected after five attempts.\033[0m";
	  return 1;
	}
      }
      QventinoUtils::sleep( sleepSecs );
    }
  } catch( std::string error ) {
    std::cout << "\033[31mError: " << error << "\033[0m" << std::endl;
    return 1;
  }

  return 0;
  
}
