// +--------------------------------------------------------+
// | Window interface for 4ventino EEPROM manipulation      |
// | Benigno Gobbo                                          |
// | © INFN Sezione di Trieste                              |
// | V0.3                                                   |
// | 2020-02-18                                             |
// +--------------------------------------------------------+

#ifndef Window_h
#define Window_h

#include <cstdint>
#include <QMainWindow>

struct EEPROMContent {
  char    init[5];          // char string to flag initialisation
  int16_t line;             // gas line number
  uint8_t mac[6];           // mac address
  char    passkey[5];       // setup pass key 
  uint8_t ip[4];            // IP address
  uint8_t dns[4];           // DNS 
  uint8_t gw[4];            // gateway
  uint8_t snm[4];           // subnet mac
  char    ntp[32];          // NTP server
  float   min[5];           // scaling straight line slope
  float   max[5];           // scaling straight line intercept
  uint8_t sleep;            // sleep flag
};

QT_BEGIN_NAMESPACE
class QMenu;
class QMenuBar;
class QTextEdit;
class QLabel;
class QPushButton;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QRadioButton;
QT_END_NAMESPACE

class Console;
class QventinoClient;

// +------------------+
// | class MainWindow |
// +------------------+

class MainWindow : public QMainWindow {

  Q_OBJECT

 public:

  MainWindow( void );

 private slots:

  void deviceConnectDisconnect( void );
  void reload( void );
  void send( void );
  void updateStatusConsole( void );
  void statusBoxHoldReleaseScroll( void );
  void statusBoxClearConsole( void );
  void setMode( int mode );

 private:

  QventinoClient *_client;
  std::string    _devAddress;
  std::string    _devPort;
  bool           _devConnected;
  Console        *_console;
  
  // Buttons
  QPushButton *_connectButton;
  QPushButton *_reloadButton;
  QPushButton *_sendButton;
  QPushButton *_quitButton;
  // Device
  QSpinBox     *_edevAddr[4];
  QSpinBox     *_edevPrt;
  QLineEdit    *_sdevModel;
  QComboBox    *_sdevSerNum;
  int          _devMode;
  QRadioButton *_setEther;
  QRadioButton *_setSerial;
  // Network
  QSpinBox *_ipAddr[4];
  QSpinBox *_dnsAddr[4];
  QSpinBox *_gwAddr[4];
  QSpinBox *_nwMask[4];
  QSpinBox *_ntAddr[4];
  // Scale
  QDoubleSpinBox *_tp1[2];
  QDoubleSpinBox *_tp2[2];
  QDoubleSpinBox *_flow[2];
  QDoubleSpinBox *_plin[2];
  QDoubleSpinBox *_pref[2];
  // Other
  QSpinBox *_lnNum;
  QSpinBox *_psk;
  QCheckBox      *_smp;
  // Status window
  QTextEdit   *_statusBoxText;
  bool        _statusBoxHoldScrollFlag;
  QPushButton *_statusBoxHoldReleaseScroll;
  //
  static EEPROMContent _eepromContent;
  bool _decodeRemoteData( std::string data );
  void _showRemoteData( void );
  std::string _encodeRemoteData( void );
  void _updateEepromContent( void );
  void _dumpEepromContent( void );
};

#endif // Window_h
