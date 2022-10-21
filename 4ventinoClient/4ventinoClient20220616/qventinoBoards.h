#ifndef QVENTINOBOARDS_H
#define QVENTINOBOARDS_H

#include <string>
#include <vector>
#include <bitset> 

namespace q20 {

  const std::string jsonFile = "qventinoBoards.json";
  
  extern std::string mode;                 // "ethernet" or "usb"
  extern std::string IPport;               // TCP/IP communication port
  extern std::vector<std::string> IPaddr;  // boards IP addresses
  extern std::string UARTpn;               // USB-UART interface device product name
  extern std::vector<std::string> UARTsn;  // USB-UART interface device serial number
  extern std::bitset<7> bselected;         // bit list of board i.e. lines to use. bit+1 = line
  extern std::bitset<7> bfound;            // bit list of found boards
}

#endif // QVENTINOBOARDS_H
