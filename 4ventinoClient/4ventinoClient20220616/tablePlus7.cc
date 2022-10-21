// +--------------------------------------------------------+
// | A multiple access test program for the 4ventino client |
// | with tablular formatted output. Uses QventinoInit.     |
// | Benigno Gobbo - INFN Trieste                           |
// | V0.1 20210713                                          |
// +--------------------------------------------------------+

#include <vector>
#include <iostream>
#include <iomanip>
#include <exception>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <ctime>
#include "qventino.h"

const std::string command   =  "data";
const int         precision[5] = { 1, 1, 2, 0, 0 };
const int         precision2[5] = { 1, 1, 1, 2, 0 };

float data[ 7 ][ 5 ] = {};
std::string dataTime[ 7 ] = {};

// <><><><><><> write out the table...
void dump( int toRead ) {
  std::cout << "----------------------------------------------------------------------------------------------------" << std::endl
	    << "\033[1;33m Line        Date      Time        TP1 (°C)      TP2 (°C)  Flow (l/min)   Pline (hPa)    Pref (hPa)\033[0m" << std::endl
	    << "----------------------------------------------------------------------------------------------------" << std::endl;
  for( int i=0; i<toRead; i++ ) {
    if( i == 5 ) {
        std::cout << "----------------------------------------------------------------------------------------------------" << std::endl
		  << "\033[1;33m Line        Date      Time        TP1 (°C)      TP2 (°C)      TP3 (°C)  Flow (l/min)   Pline (hPa)\033[0m" << std::endl
		  << "----------------------------------------------------------------------------------------------------";
    }
    else {
      std::cout << "  \033[1;31m" << i+1 << "       \033[36m" << dataTime[i] << "\033[0m";
      for( int j=0; j<5; j++ ) {
	if( data[i][j] > -9999 ) {
	  std::cout << "        \033[1;32m" << std::right;
	  if( i == 6 ) {
	    std::cout << std::setw(6) << std::fixed << std::setprecision( precision2[j] ) << data[i][j] << std::left << "\033[0m";
	  }
	  else {
	    std::cout << std::setw(6) << std::fixed << std::setprecision( precision[j] ) << data[i][j] << std::left << "\033[0m";
	  }
	}
	else {
	  std::cout << "             \033[1;32m-\033[0m";
	}
      }
    }
    std::cout << std::endl;   
  }
  std::cout << "----------------------------------------------------------------------------------------------------" << std::endl;
}

// <><><><><><> Main...
int main( int argc, char *argv[] ) {

  const int sleepSecs = 5;
  std::vector<QventinoClient*> clients;

  std::cout << "\033c";
  
  try {

    QventinoInit* init = QventinoInit::initialize();
    clients = init->getClients();
    
    while( true ) {
      for( int i=0; i<clients.size(); i++ ) {
	if( q20::bfound.test(i) ) {
	  bool status = false; int count = 0;
	  while( !status && count++ < 5 ) {
	    clients[i]->clientWrite( command );
	    usleep( 300000 );
	    std::string s = clients[i]->clientRead();
	    status = clients[i]->dataDecode( s );
	    if( status ) {
	      for( int j=0; j<5; j++ ) data[i][j] = clients[i]->getData(j);
	      std::time_t sinceEpoch = clients[i]->getSecondsSinceEpoch();
	      if( sinceEpoch > 0 ) {
		char timeBuffer[40];	      
		struct tm * timeDate;
		timeDate = localtime( &sinceEpoch );
		strftime( timeBuffer, 40, "%Y/%m/%d %H:%M:%S", timeDate );
		dataTime[i] = timeBuffer;
	      }
	      else {
		dataTime[i] = "----/--/-- --:--:--";
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
	else {
	  dataTime[i] = "----/--/-- --:--:--";
	}
      }
      dump( clients.size() );
      QventinoUtils::sleep( sleepSecs );
    }
  } catch( std::string error ) {
    std::cout << "\033[31mError: " << error << "\033[0m" << std::endl;
    return 1;
  }

  return 0;
  
}
