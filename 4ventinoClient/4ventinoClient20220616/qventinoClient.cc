// +---------------------------------------+
// | Client class for the 4ventino         |
// | Benigno Gobbo - INFN Trieste          |
// | V1.4 20200217                         |
// +---------------------------------------+

#include <iostream>
#include <exception>
#include <sstream>
#include "qventinoUtils.h"
#include "qventinoSerial.h"
#include "qventinoEthernet.h"
#include "qventinoClient.h"

// <><><><><><> connect to I/O channel
void QventinoClient::clientConnect( int mode, std::string param1, std::string param2 ) {
  try {
    _mode = mode;
    _param[0] = param1;
    _param[1] = param2;
    if( _mode == 1 ) {
      _serial.serialConnect( _param[0], _param[1] );
    }
    if( _mode == 2 ) {
      _ethernet.ethernetConnect( _param[0], std::stoi( _param[1] ) );
    }
  } catch( std::string error ) {
    throw( error );
    return;
  }
}

// <><><><><><> disconnect to I/O channel
void QventinoClient::clientDisconnect( void ) {
  try {
    if( _mode == 1 ) {
      _serial.serialDisconnect();
    }
    if( _mode == 2 ) {
      _ethernet.ethernetDisconnect();
    }
  } catch( std::string error ) {
    throw( error );
    return;
  }
}

// <><><><><><> reconnect to I/O channel
void QventinoClient::clientReconnect( int mode, std::string param1, std::string param2 ) {
  try {
    _mode = mode;
    _param[0] = param1;
    _param[1] = param2;
    if( _mode == 1 ) {
      _serial.serialDisconnect();
      _serial.serialConnect( _param[0], _param[1] );
    }
    if( _mode == 2 ) {
      _ethernet.ethernetDisconnect();
      _ethernet.ethernetConnect( _param[0], std::stoi( _param[1] ) );
    }
  } catch( std::string error ) {
    throw( error );
    return;
  }
}

// <><><><><><> send data to the I/O channel
void QventinoClient::clientWrite( std::string s ) {
  try {
    if( _mode == 1 ) {
      _serial.serialWrite( s );
    }
    else if( _mode == 2 ) {
      _ethernet.ethernetWrite( s );
    }
  } catch( std::string error ) {
    throw( error );
    return;
  }
}

// <><><><><><> get data from the I/O channel
std::string QventinoClient::clientRead( void ) {
  std::string s = "";
  try {
    if( _mode == 1 ) {
      s = _serial.serialRead();
    }
    else if( _mode == 2 ) {
      s = _ethernet.ethernetRead();
    }
  } catch( std::string error ) {
    throw( error );
    return( std::string( "" ) ); 
  }
  return( s );
}

// <><><><><><> decode the collected data
bool QventinoClient::dataDecode( std::string s ) {
  if( s[0] != 'D' && s[s.length()-2] != 'E' ) {
    //std::stringstream ss;  ss << "\033[31mError: the received string has a wrong format: "  << s << "\033[0m";
    //throw( ss.str() );
    return( false );
  }
  s = s.substr( 1 );
  _year = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 4 ) ) ) {
    _year = std::stod( s.substr( 0, 4 ) );
  }
  s = s.substr( 4 );
  _month = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _month = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  _day = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _day = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  if( s[0] != 'T' ) {
    //std::stringstream ss;  ss << "\033[31mError: the received string has a wrong format: " << s  << "\033[0m";
    //throw( ss.str() );
    return( false );
  }
  s = s.substr( 1 );
  _hour = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _hour = std::stod( s.substr( 0, 2 ) );
  }
  _minute = -9999;
  s = s.substr( 2 );
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _minute = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  _second = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _second = std::stod( s.substr( 0, 2 ) );
  }
  if( _year > -9999 && _month > -9999 && _day > -9999 && _hour > -9999 && _minute > -9999 && _second > -9999 ) {
    struct tm dataTime;
    dataTime.tm_year  = _year - 1900;
    dataTime.tm_mon   = _month -1;
    dataTime.tm_mday  = _day;
    dataTime.tm_hour  = _hour;
    dataTime.tm_min   = _minute;
    dataTime.tm_sec   = _second;
    dataTime.tm_isdst = -1;
    _sinceEpoch       = mktime( & dataTime );
  }
  else {
    _sinceEpoch = 0;
  }
  s = s.substr( 2 );
  for( int i=0; i<5; i++ ) {
    if( s[0] != 'M' ) {
      //std::stringstream ss;  ss << "\033[31mError: the received string has a wrong format: "  << s << "\033[0m";
      //throw( ss.str() );
      return( false ); 
    }
    s = s.substr( 1 );
    int pos = s.find( "M" );
    if( pos == -1 ) pos = s.length()-3;
    std::string meas = s.substr( 0, pos );
    if( QventinoUtils::isFloat( meas ) ) {
      _data[i] = std::stof( meas );
    }
    else {
      _data[i] = -9999;
    }
    s = s.substr( pos );
  }
  if( s[0] != 'E' ) {
    //std::stringstream ss;  ss << "\033[31mError: the received string has a wrong format: "  << s << "\033[0m";
    //throw( ss.str() );
    return( false ); 
  }
  return( true );
}

// <><><><><><> dump data to the display
void QventinoClient::dataDump( void ) { 
  if( _year > -9999 ) std::cout << _year << "-";
  else std::cout << "-----";
  if( _month > -9999 ) std::cout << QventinoUtils::twoDigits( _month )  << "-";
  else std::cout << "---";
  if( _day > -9999 ) std::cout << QventinoUtils::twoDigits( _day )  << " ";
  else std::cout << "-- ";
  if( _hour > -9999 ) std::cout << QventinoUtils::twoDigits( _hour )  << ":";
  else std::cout << "--:";
  if( _minute > -9999 ) std::cout << QventinoUtils::twoDigits( _minute )  << ":";
  else std::cout << "--:";
  if( _second > -9999 ) std::cout << QventinoUtils::twoDigits( _second )  << std::endl;
  else std::cout << "--" << std::endl;
  if( _data[0] > -9999 ) std::cout << "TP1    : " << _data[0] << " °C" << std::endl;
  else std::cout << "TP1    : ------- °C" << std::endl;
  if( _data[1] > -9999 ) std::cout << "TP2    : " << _data[1] << " °C" << std::endl;
  else std::cout << "TP2    : ------- °C" << std::endl;
  if( _data[2] > -9999 ) std::cout << "Flow   : " << _data[2] << " l/min" << std::endl;
  else std::cout << "Flow   : ------- l/min" << std::endl;
  if( _data[3] > -9999 ) std::cout << "Pline  : " << _data[3] << " hPa" << std::endl;
  else std::cout << "Pline  : ------- hPa" << std::endl;
  if( _data[4] > -9999 ) std::cout << "Pref   : " << _data[4] << " hPa" << std::endl;
  else std::cout << "Pref   : ------- hPa" << std::endl;
}

// <><><><><><> the the i-th data value
float QventinoClient::getData( int i ) {
  if( i > -1 && i < 5 ) {
    return( _data[i] );
  }
  else {
    std::stringstream ss;  ss << "\033[31mError: data array index should be between 0 and 4.\033[0m";
    throw( ss.str() );
    return( 0 );
  }
}
