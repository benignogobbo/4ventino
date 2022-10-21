// +--------------------------------------------------------+
// | A multiple access test program for the 4ventino client |
// | with output to file                                    |
// | Benigno Gobbo - INFN Trieste                           |
// | V1.0 20191205                                          |
// +--------------------------------------------------------+

#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <ctime>
#include "qventino.h"

const std::string command   =  "data";
const std::string IPaddr[7] = { "192.168.69.11", "192.168.69.12", "192.168.69.13", "192.168.69.14", "192.168.69.15", "192.168.69.16", "192.168.69.17" };
const std::string IPport    = "57777";
const std::string UARTpn    = "FT232R USB UART";
const std::string UARTsn[7] = { "AH06GM8C", "A106QUOF", "A9078H15", "AH06GKUP", "A9078G9H", "AH06GLG2", "AH06GM2D" };
const int         toRead    = 6;
const int         precision[5] = { 1, 1, 2, 0, 0 };

float data[ toRead ][ 5 ] = {};
std::string dateTime[ toRead ] = {};
std::ofstream outFile;


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

// <><><><><><> write out the table...
void dump( void ) {
  std::cout << dateTime[0] << std::endl;
  outFile << "----------------------------------------------------------------------------------------------------" << std::endl
          << " Line        Date      Time        TP1 (°C)      TP2 (°C)  Flow (l/min)   Pline (hPa)    Pref (hPa) " << std::endl
          << "----------------------------------------------------------------------------------------------------" << std::endl;
  for( int i=0; i<toRead; i++ ) {
    outFile << "  " << i+1 << "       " << dateTime[i];
    for( int j=0; j<5; j++ ) {
      if( data[i][j] > -9999 ) {
	outFile << "        " << std::right << std::setw(6) << std::fixed << std::setprecision( precision[j] ) << data[i][j] << std::left;
      }
      else {
	outFile << "             ";
      }
    }
    outFile << std::endl;   
  }
  outFile << "----------------------------------------------------------------------------------------------------" << std::endl;
}

// <><><><><><> Main...
int main( int argc, char *argv[] ) {

  int mode = 0;
  std::string param[2] = { "", "" };
  const int sleepSecs = 900;
  
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

  QventinoClient client[toRead];

  outFile.open( "tofile.txt" );

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
	  //std::cout << "Status: " << status << ", count: " << count << std::endl;
	  client[i].clientWrite( command );
	  usleep( 300000 );
	  std::string s = client[i].clientRead();
	  status = client[i].dataDecode( s );
	  if( status ) {
	    for( int j=0; j<5; j++ ) data[i][j] = client[i].getData(j);
	    std::time_t sinceEpoch = client[i].getSecondsSinceEpoch();
	    if( sinceEpoch > 0 ) {
	      char timeBuffer[40];	      
	      struct tm * timeDate;
	      timeDate = localtime( &sinceEpoch );
	      strftime( timeBuffer, 40, "%Y/%m/%d %H:%M:%S", timeDate );
	      dateTime[i] = timeBuffer;
	    }
	    else {
	      dateTime[i] = "----/--/-- --:--:--";
	    }
	  }
	  else {
	    usleep( 500000 );
	  }
	}
	if( status == false ) {
	  std::cout << "\033[31mError: no correct serial data collected after five attempts.\033[0m" << std::endl;
	  return 1;
	}
      }
      dump();
      QventinoUtils::sleep( sleepSecs );
    }
  } catch( std::string error ) {
    std::cout << "\033[31mError: " << error << "\033[0m" << std::endl;
    return 1;
  }

  outFile.close();
  
  return 0;
  
}
