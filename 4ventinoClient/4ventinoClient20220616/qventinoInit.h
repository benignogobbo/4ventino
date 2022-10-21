#ifndef INIT_ALL_H
#define INIT_ALL_H

// +--------------------------------------+
// | Initialization                       |
// | Benigno Gobbo - INFN Trieste         |
// | V1.0 20190905                        |
// +--------------------------------------+

#include <vector>
#include "qventinoBoards.h"
#include "qventino.h"

class QventinoInit {
  
 public:
  static QventinoInit* initialize( void );
  inline std::vector<QventinoClient*>  getClients( void )  { return _boards; }

  
 protected:
  QventinoInit();

 private:
  static QventinoInit* _init;
  static std::vector<QventinoClient*> _boards;
  // functions
  void   _readJSON( void );
  void   _connectToBoards( void );
};
  
#endif // INIT_ALL_H
