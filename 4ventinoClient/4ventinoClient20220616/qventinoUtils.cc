// +----------------------------------------+
// | Some Utilities for the 4ventino client |
// | Benigno Gobbo - INFN Trieste           |
// | V1.0 20190214                          |
// +----------------------------------------+

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include "qventinoUtils.h"

// <><><><><><> Check for enter key...
int QventinoUtils::kbhit() {

    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

// <><><><><><> add a leading zero (in case...)
std::string QventinoUtils::twoDigits( int number ) {
  std::string s;
  std::stringstream ss;
  ss << number;
  if( number < 10 ) {
    s = '0' + ss.str();
  }
  else {
    s = ss.str();
  }
  return( s );
}

// <><><><><><> This to check if a string is a real number...
bool QventinoUtils::isFloat( std::string s ) {
    std::istringstream ss( s );
    float f;
    ss >> std::noskipws >> f;
    return ss.eof() && !ss.fail(); 
}

// <><><><><><> This to check if a string is an integer number...
bool QventinoUtils::isInt( std::string s ) {
    std::istringstream ss( s );
    int i;
    ss >> std::noskipws >> i;
    return ss.eof() && !ss.fail(); 
}

// <><><><><><> Sleep telling that you do sleep...
void QventinoUtils::sleep( int secs ) {  
  for( int j=0; j<secs; j++ ) {
    std::cout << "\033[2K\033[33mNow sleeping " << secs-j << " seconds \033[34m(<Enter> to exit)\033[0m" << std::endl;
    int hit = kbhit();
    if( hit != 0 ) {
      exit(0);
    }
    std::cout << "\033[2A" << std::endl;
  }
  std::cout << "\033c";
}

