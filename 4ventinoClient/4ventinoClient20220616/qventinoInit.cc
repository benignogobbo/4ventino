// +--------------------------------------+
// | Initialization                       |
// | Benigno Gobbo - INFN Trieste         |
// | V1.0 20190905                        |
// +--------------------------------------+

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

// This is from here: https://github.com/nlohmann/json
#include "json.hpp"
// git cloned on 2019/09/05

#include "qventinoInit.h"

using json = nlohmann::json;

QventinoInit*                 QventinoInit::_init    = NULL;
std::vector<QventinoClient*>  QventinoInit::_boards  = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// These need to be initialized...
std::bitset<7>           q20::bfound( std::string( "0000000" ) );
// These don't (will be initialized from JSON file in Init)...
std::string              q20::mode;
std::string              q20::IPport;
std::vector<std::string> q20::IPaddr(7);
std::string              q20::UARTpn;
std::vector<std::string> q20::UARTsn(7);
std::bitset<7>           q20::bselected;

// <+><+><+><+><+><+> Single object instance
QventinoInit* QventinoInit::initialize( void ) {
  if( _init == 0 ) {
    _init = new QventinoInit;
    
  }
  return _init;
}

// <+><+><+><+><+><+> Constructor
QventinoInit::QventinoInit( void ) {

  try {
    // First of all set thinks from JSON file...
    _readJSON();
    _connectToBoards();
  }
  catch( std::string error ) {
    throw( std::string( "\033[31mError during initialisation: " + error + "\033[0m" ) );
    return;
  }
}

// <+><+><+><+><+><+> readJSON
void QventinoInit::_readJSON( void ) {
  
  std::ifstream f;
  f.open( q20::jsonFile );
  if( f.fail() ) {
    throw( std::string( "\033[31mOh shit!: cannot find " + q20::jsonFile + " file.\033[0m" ) );
    return;
  }

  json j_boards;
  f >> j_boards;
  f.close();

  std::unordered_map<std::string, int> umip,  umsn;
  
  q20::mode   = j_boards.at("board").at("mode").get<std::string>();
  q20::IPport = j_boards.at("board").at("port").get<std::string>();
  umip        = j_boards.at("board").at("ip").get<std::unordered_map<std::string,int>>();
  q20::UARTpn = j_boards.at("board").at("usbpn").get<std::string>();
  umsn        = j_boards.at("board").at("usbsn").get<std::unordered_map<std::string,int>>();
  q20::bselected = std::bitset<7>( j_boards.at("board").at("bselected").get<std::string>() );

  for( auto i : umip  ) {
    q20::IPaddr[i.second] = i.first;
  }
  for( auto i : umsn ) {
    q20::UARTsn[i.second] = i.first;
  }

  return;
}

// <+><+><+><+><+><+> readJSON
void   QventinoInit::_connectToBoards( void ) {

  if( q20::bselected.count() == 0 ) std::cout << "\033[33m Warning: there are no selected boards. Please check the JSON file.\033[0m" << std::endl;

  int i = 0;
  
  try {

    for( i=0; i<7; i++ ) {
      if( q20::bselected.test(i) ) {  // i-th board ( (i+1)-th line ) results selected
	QventinoClient* client = new QventinoClient();	
	if( q20::mode == "ethernet" ) {
	  client->clientConnect( 2, q20::IPaddr[i], q20::IPport ); // "ethernet" == 2
	}
	else if( q20::mode == "usb" ) {
	  client->clientConnect( 1, q20::UARTpn, q20::UARTsn[i] ); // "usb" == 1
	}
	else {
	  throw ( std::string( "\033[1;31mError: wrong connection mode, it must be 'ethernet' or 'usb'. Please check JSON file.\033[0m" ) );
	}
	_boards[i] = client;
	q20::bfound.set( i, 1 );
      }
    }
   
  } catch ( std::string error ) {
    std::stringstream ss;
    ss << "\033[1;31mError in connecting to board " <<  i <<  ", i.e. line " << i+1 << ".\033[0m";
    throw( ss.str() );
  }
}
