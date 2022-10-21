// +----------------------------------------+
// | Some Utilities for the 4ventino client |
// | Benigno Gobbo - INFN Trieste           |
// | V1.0 20190214                          |
// +----------------------------------------+

#ifndef QVENTINOUTILS_H
#define QVENTINOUTILS_H

#include <string>

class QventinoUtils {
 public:
  static int         kbhit( void );
  static std::string twoDigits( int );
  static bool        isFloat( std::string );
  static bool        isInt( std::string );
  static void        sleep( int );
};

#endif // QVENTINOUTILS_H
