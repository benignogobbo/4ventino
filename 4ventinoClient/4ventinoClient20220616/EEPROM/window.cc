// +--------------------------------------------------------+
// | Window interface for 4ventino EEPROM manipulation      |
// | Benigno Gobbo                                          |
// | © INFN Sezione di Trieste                              |
// | V0.3                                                   |
// | 2020-02-18                                             |
// +--------------------------------------------------------+

#include <QtWidgets>
#include <QTextEdit>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include "window.h"
#include "console.h"
#include "qventinoUtils.h"
#include "qventino.h"
#include "json.hpp"

using json = nlohmann::json;


// +------------------+
// | class MainWindow |
// +------------------+

EEPROMContent MainWindow::_eepromContent = {
  "", 0, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "",
  { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
  "", { 1.0, 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0, 0.0, 0.0 }, 0
};

// +-----------------------------------------------------------+
// | Checks if a string corresponds to a floating point number |
// +-----------------------------------------------------------+
bool isFloat( std::string s ) {
  std::istringstream ss( s );
  float f;
  ss >> std::noskipws >> f;
  return ss.eof() && !ss.fail(); 
}

// +----------------------------------------------------+
// | Checks if a string corresponds to a decimal number |
// +----------------------------------------------------+
bool isByte( std::string s ) {
  s = std::string( "0x" ) + s;
  std::istringstream ss( s );
  int i;
  ss >> std::hex >> std::noskipws >> i;
  return ss.eof() && !ss.fail(); 
}

// +--------------------------+
// | return hex digits string |
// +--------------------------+
template<typename T> std::string toHex( T number ) {
  std::stringstream s;
  s << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << int( number );
  return s.str();
}

// +----------------------------------+
// | fixed precision number to string |
// +----------------------------------+
template <typename T> std::string toStr( const T number, const int n = 2 ) {
    std::ostringstream s;
    s.precision( n );
    s << std::fixed << number;
    return s.str();
}

// +----------------------------------+
// | fixed precision number to string |
// +----------------------------------+
void readJSON( void ) {
  std::ifstream f;
  f.open( std::string("../") + q20::jsonFile );
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


// +----+
// | UI |
// +----+
MainWindow::MainWindow( ) : QMainWindow( ) {

  _console = new Console();
  _console->startConsoleStream();
  _statusBoxHoldScrollFlag = false;
  _devConnected = false;
  _client = new QventinoClient;

  try {
    readJSON();
  } catch( std::string error ) {
    std::cout << "<font color=#FF0000>readJSON: " << error << "</font><br>" << std::endl;
    updateStatusConsole();
    exit(1);
  }
  
  QWidget *mainWidget = new QWidget;
  mainWidget->setWindowTitle( tr( "4ventino EEPROM editor" ) );
  mainWidget->setStyleSheet( "QWidget {font-family: 'Arial'; font-size: 10pt}" "QPushButton:disabled { color: gray }" );  
  setCentralWidget( mainWidget );

  // The elementary menu
  QAction *exitAction = new QAction( tr("E&xit"), this ) ;
  exitAction->setShortcuts( QKeySequence::Quit );
  exitAction->setStatusTip( tr("Exit qventinoeeprom") );
  connect( exitAction, SIGNAL( triggered() ), this, SLOT( close() ) );
  QMenu *fileMenu = menuBar()->addMenu( tr("&File") );
  menuBar()->setStyleSheet( "background-color: rgb( 30, 60, 120 )" );
  fileMenu->setStyleSheet( "background-color: rgb( 10, 40, 100 ); selection-color: yellow" );
  fileMenu->addAction( exitAction );

  QGridLayout *mainLayout = new QGridLayout( mainWidget );
  
  QGroupBox *deviceBox0 =  new QGroupBox( tr( "Device" ) );
  deviceBox0->setStyleSheet( "QGroupBox { font-family: 'Arial'; font-size: 12pt }" );
  deviceBox0->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  QGroupBox *deviceBox1 =  new QGroupBox( tr( "" ) );
  deviceBox1->setStyleSheet( "QGroupBox { font-family: 'Arial'; font-size: 12pt }" );
  deviceBox1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  QGroupBox *networkBox = new QGroupBox( tr( "Network" ) );
  networkBox->setStyleSheet( "QGroupBox { font-family: 'Arial'; font-size: 12pt }" );
  networkBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  QGroupBox *scalingBox = new QGroupBox( tr( "Scaling" ) );
  scalingBox->setStyleSheet( "QGroupBox { font-family: 'Arial'; font-size: 12pt }" );
  scalingBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  QGroupBox *othersBox = new QGroupBox( tr( "Other Settings" ) );
  othersBox->setStyleSheet( "QGroupBox { font-family: 'Arial'; font-size: 12pt }" );
  othersBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  QGroupBox *buttonBox = new QGroupBox;
  QGroupBox *statusBox = new QGroupBox( tr( "Info..." ) );
  
  QLabel *mainTitle = new QLabel( tr( "\n4ventino EEPROM editor\n" ) );
  mainTitle->setStyleSheet( "QLabel { font-family: 'Arial'; font-size: 16pt; color: rgb( 255, 255, 255 ) }" );

  QLabel *lastLine = new QLabel( tr( "©2020 I.N.F.N. Sezione di Trieste\n\"Handwritten\" by Benigno. Disclaimer: this software is experimental so, please, use it with due caution." ) );
  lastLine->setStyleSheet( "QLabel { font-family: 'Arial'; font-size: 9pt; color: rgb( 210, 210, 210 ) }" );

  mainLayout->addWidget( mainTitle,  0, 0, 1, 4 );
  mainLayout->addWidget( deviceBox0, 1, 0, 1, 3 );
  mainLayout->addWidget( deviceBox1, 1, 3, 1, 1  );
  mainLayout->addWidget( networkBox, 2, 0, 1, 2 );
  mainLayout->addWidget( scalingBox, 2, 2, 1, 2 );
  mainLayout->addWidget( othersBox,  3, 0, 1, 2 );
  mainLayout->addWidget( buttonBox,  3, 2, 1, 2 );
  mainLayout->addWidget( statusBox,  4, 0, 1, 4 );
  mainLayout->addWidget( lastLine,   5, 0, 1, 4 );

  // Device parameters setup
  QGridLayout *devlo0 = new QGridLayout;
  QGridLayout *devlo1 = new QGridLayout;

  QButtonGroup *buttonGroup = new QButtonGroup;
  connect( buttonGroup, SIGNAL( buttonClicked(int) ), this, SLOT( setMode(int) ) );
  _setEther = new QRadioButton;
  _setEther->setChecked( true );
  _devMode = 2;
  buttonGroup->addButton( _setEther, 2 );
  
  QLabel *edevAddrLb = new QLabel( tr( "Current device IP Address" ) );
  std::string s = q20::IPaddr[0];
  for( int i=0; i<4; i++ ) {  
    _edevAddr[i] = new QSpinBox;
    _edevAddr[i]->setFixedWidth( 50 );
    _edevAddr[i]->setButtonSymbols( QSpinBox::NoButtons );
    _edevAddr[i]->setAlignment( Qt::AlignRight );
    _edevAddr[i]->setRange( 0, 255 );
    _edevAddr[i]->setSingleStep( 1 );
    if( i<3 ) _edevAddr[i]->setValue( std::stoi( s.substr( 0, s.find( "." ) ) ) );
    else _edevAddr[i]->setValue( 0 );
    s = s.substr( s.find( "." )+1 );
    _edevAddr[i]->setSuffix( "" );
  }
  
  QLabel *edevPrtLb = new QLabel( tr( "  and communication port" ) );
  _edevPrt = new QSpinBox;
  _edevPrt->setFixedWidth( 100 );
  _edevPrt->setButtonSymbols( QSpinBox::NoButtons );
  _edevPrt->setAlignment( Qt::AlignRight );
  _edevPrt->setRange( 0, 99999 );
  _edevPrt->setSingleStep( 1 );
  _edevPrt->setValue( std::stoi(q20::IPport) );
  _edevPrt->setSuffix( "" );

  _setSerial = new QRadioButton;
  buttonGroup->addButton( _setSerial, 1 );
  
  QLabel *sdevModLb = new QLabel( tr( "Serial device model" ) );
  _sdevModel = new QLineEdit( q20::UARTpn.c_str() );
  QLabel *sdevSnLb = new QLabel( tr( "          and serial number" ) );
  _sdevSerNum = new QComboBox;
  _sdevSerNum->setStyleSheet( "QComboBox { min-width: 100px; }" );
  for( unsigned int i=0; i< q20::UARTsn.size(); i++ ) {
    _sdevSerNum->addItem( q20::UARTsn[i].c_str() );
  }
  
  _sdevSerNum->setFixedWidth( 80 );
  
  _connectButton = new QPushButton( tr("Connect" ), this );
  _connectButton->setFixedSize( QSize( 100, 30 ) ); 
  _connectButton->setStyleSheet( "alignment: center; background-color: rgb( 0, 100, 50 )" );
  connect( _connectButton, SIGNAL( clicked() ), this, SLOT( deviceConnectDisconnect() ) );

  devlo0->addWidget( _setEther, 0, 0 );
  devlo0->addWidget( edevAddrLb, 0, 1 );
  devlo0->addWidget( _edevAddr[0], 0, 2 );
  devlo0->addWidget( _edevAddr[1], 0, 3 );
  devlo0->addWidget( _edevAddr[2], 0, 4 );
  devlo0->addWidget( _edevAddr[3], 0, 5 );
  devlo0->addWidget( edevPrtLb, 0, 6 );
  devlo0->addWidget( _edevPrt, 0, 7 );

  devlo0->addWidget( _setSerial, 1, 0 );
  devlo0->addWidget( sdevModLb, 1, 1 );
  devlo0->addWidget( _sdevModel, 1, 2, 1, 4 );
  devlo0->addWidget( sdevSnLb, 1, 6 );
  devlo0->addWidget( _sdevSerNum, 1, 7 );
  deviceBox0->setLayout( devlo0 );
  devlo1->addWidget( _connectButton, 0, 0 ); 
  deviceBox1->setLayout( devlo1 );
  
  // Network parameters setup 
  QGridLayout *netLo = new QGridLayout;

  QLabel *ipAddrLb = new QLabel( tr( "IP Address" ) );
  ipAddrLb->setFixedWidth( 150 );
  for( int i=0; i<4; i++ ) {
    _ipAddr[i] = new QSpinBox;
    _ipAddr[i]->setButtonSymbols( QSpinBox::NoButtons );
    _ipAddr[i]->setAlignment( Qt::AlignRight );
    _ipAddr[i]->setRange( 0, 255 );
    _ipAddr[i]->setSingleStep( 1 );
    _ipAddr[i]->setValue( 0 );
    _ipAddr[i]->setSuffix( "" );
  }
  
  QLabel *dnsAddrLb = new QLabel( tr( "DNS Address" ) );
  for( int i=0; i<4; i++ ) {
    _dnsAddr[i] = new QSpinBox;
    _dnsAddr[i]->setButtonSymbols( QSpinBox::NoButtons );
    _dnsAddr[i]->setAlignment( Qt::AlignRight );
    _dnsAddr[i]->setRange( 0, 255 );
    _dnsAddr[i]->setSingleStep( 1 );
    _dnsAddr[i]->setValue( 0 );
    _dnsAddr[i]->setSuffix( "" );
  }

  QLabel *gwAddrLb = new QLabel( tr( "Gateway Address" ) );
  for( int i=0; i<4; i++ ) {
    _gwAddr[i] = new QSpinBox;
    _gwAddr[i]->setButtonSymbols( QSpinBox::NoButtons );
    _gwAddr[i]->setAlignment( Qt::AlignRight );
    _gwAddr[i]->setRange( 0, 255 );
    _gwAddr[i]->setSingleStep( 1 );
    _gwAddr[i]->setValue( 0 );
    _gwAddr[i]->setSuffix( "" );
  }
    
  QLabel *nwMaskLb = new QLabel( tr( "Network Mask" ) );
  for( int i=0; i<4; i++ ) {
    _nwMask[i] = new QSpinBox;
    _nwMask[i]->setButtonSymbols( QSpinBox::NoButtons );
    _nwMask[i]->setAlignment( Qt::AlignRight );
    _nwMask[i]->setRange( 0, 255 );
    _nwMask[i]->setSingleStep( 1 );
    _nwMask[i]->setValue( 0 );
    _nwMask[i]->setSuffix( "" );
  }
  
  QLabel *ntAddrLb = new QLabel( tr( "NTP Server Address" ) );
  for( int i=0; i<4; i++ ) {
    _ntAddr[i] = new QSpinBox;
    _ntAddr[i]->setButtonSymbols( QSpinBox::NoButtons );
    _ntAddr[i]->setAlignment( Qt::AlignRight );
    _ntAddr[i]->setRange( 0, 255 );
    _ntAddr[i]->setSingleStep( 1 );
    _ntAddr[i]->setValue( 0 );
    _ntAddr[i]->setSuffix( "" );
  }
  
  netLo->addWidget( ipAddrLb, 0, 0 );
  netLo->addWidget( _ipAddr[0], 0, 1 );
  netLo->addWidget( _ipAddr[1], 0, 2 );
  netLo->addWidget( _ipAddr[2], 0, 3 );
  netLo->addWidget( _ipAddr[3], 0, 4 );

  netLo->addWidget( dnsAddrLb, 1, 0 );
  netLo->addWidget( _dnsAddr[0], 1, 1 );
  netLo->addWidget( _dnsAddr[1], 1, 2 );
  netLo->addWidget( _dnsAddr[2], 1, 3 );
  netLo->addWidget( _dnsAddr[3], 1, 4 );

  netLo->addWidget( gwAddrLb, 2, 0 );
  netLo->addWidget( _gwAddr[0], 2, 1 );
  netLo->addWidget( _gwAddr[1], 2, 2 );
  netLo->addWidget( _gwAddr[2], 2, 3 );
  netLo->addWidget( _gwAddr[3], 2, 4 );

  netLo->addWidget( nwMaskLb, 3, 0 );
  netLo->addWidget( _nwMask[0], 3, 1 );
  netLo->addWidget( _nwMask[1], 3, 2 );
  netLo->addWidget( _nwMask[2], 3, 3 );
  netLo->addWidget( _nwMask[3], 3, 4 );

  netLo->addWidget( ntAddrLb, 4, 0 );
  netLo->addWidget( _ntAddr[0], 4, 1 );
  netLo->addWidget( _ntAddr[1], 4, 2 );
  netLo->addWidget( _ntAddr[2], 4, 3 );
  netLo->addWidget( _ntAddr[3], 4, 4 );

  networkBox->setLayout( netLo );
  
  // Scaling parameters setup   
  QGridLayout *scalLo = new QGridLayout;

  QLabel *tp1Lb = new QLabel( tr( "TP1" ) );
  QLabel *fma1Lb = new QLabel( tr( "4 mA:" ) );
  QLabel *tma1Lb = new QLabel( tr( "     20 mA:" ) );
  
  _tp1[0] = new QDoubleSpinBox;
  _tp1[0]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _tp1[0]->setAlignment( Qt::AlignRight );
  _tp1[0]->setRange( -200, 0 );
  _tp1[0]->setSingleStep( 1 );
  _tp1[0]->setDecimals( 0 );
  _tp1[0]->setValue( 0 );
  _tp1[0]->setSuffix( " °C" );

  _tp1[1] = new QDoubleSpinBox;
  _tp1[1]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _tp1[1]->setAlignment( Qt::AlignRight );
  _tp1[1]->setRange( 0, 50 );
  _tp1[1]->setSingleStep( 1 );
  _tp1[1]->setDecimals( 0 );
  _tp1[1]->setValue( 1 );
  _tp1[1]->setSuffix( " °C" );

  QLabel *tp2Lb = new QLabel( tr( "TP2" ) );
  QLabel *fma2Lb = new QLabel( tr( "4 mA:" ) );
  QLabel *tma2Lb = new QLabel( tr( "     20 mA:" ) );
  
  _tp2[0] = new QDoubleSpinBox;
  _tp2[0]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _tp2[0]->setAlignment( Qt::AlignRight );
  _tp2[0]->setRange( -200, 0 );
  _tp2[0]->setSingleStep( 1 );
  _tp2[0]->setDecimals( 0 );
  _tp2[0]->setValue( 0 );
  _tp2[0]->setSuffix( " °C" );

  _tp2[1] = new QDoubleSpinBox;
  _tp2[1]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _tp2[1]->setAlignment( Qt::AlignRight );
  _tp2[1]->setRange( 0, 50 );
  _tp2[1]->setSingleStep( 1 );
  _tp2[1]->setDecimals( 0 );
  _tp2[1]->setValue( 1 );
  _tp2[1]->setSuffix( " °C" );

  QLabel *flowLb = new QLabel( tr( "Flow" ) );
  QLabel *fma3Lb = new QLabel( tr( "4 mA:" ) );
  QLabel *tma3Lb = new QLabel( tr( "     20 mA:" ) );
  
  _flow[0] = new QDoubleSpinBox;
  _flow[0]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _flow[0]->setAlignment( Qt::AlignRight );
  _flow[0]->setRange( 0, 10 );
  _flow[0]->setSingleStep( 0.1 );
  _flow[0]->setDecimals( 1 );
  _flow[0]->setValue( 0 );
  _flow[0]->setSuffix( " l/min" );

  _flow[1] = new QDoubleSpinBox;
  _flow[1]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _flow[1]->setAlignment( Qt::AlignRight );
  _flow[1]->setRange( 0, 10 );
  _flow[1]->setSingleStep( 0.1 );
  _flow[1]->setDecimals( 1 );
  _flow[1]->setValue( 1 );
  _flow[1]->setSuffix( " l/min" );

  QLabel *pLinLb = new QLabel( tr( "P line" ) );
  QLabel *fma4Lb = new QLabel( tr( "4 mA:" ) );
  QLabel *tma4Lb = new QLabel( tr( "     20 mA:" ) );

  _plin[0] = new QDoubleSpinBox;
  _plin[0]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _plin[0]->setAlignment( Qt::AlignRight );
  _plin[0]->setRange( 500, 1000 );
  _plin[0]->setSingleStep( 10 );
  _plin[0]->setDecimals( 0 );
  _plin[0]->setValue( 0 );
  _plin[0]->setSuffix( " hPa" );

  _plin[1] = new QDoubleSpinBox;
  _plin[1]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _plin[1]->setAlignment( Qt::AlignRight );
  _plin[1]->setRange( 1000, 1500 );
  _plin[1]->setSingleStep( 10 );
  _plin[1]->setDecimals( 0 );
  _plin[1]->setValue( 1 );
  _plin[1]->setSuffix( " hPa" );

  QLabel *pRefLb = new QLabel( tr( "P reference" ) );
  QLabel *fma5Lb = new QLabel( tr( "4 mA:" ) );
  QLabel *tma5Lb = new QLabel( tr( "     20 mA:" ) );

  _pref[0] = new QDoubleSpinBox;
  _pref[0]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _pref[0]->setAlignment( Qt::AlignRight );
  _pref[0]->setRange( 500, 1000 );
  _pref[0]->setSingleStep( 10 );
  _pref[0]->setDecimals( 0 );
  _pref[0]->setValue( 0 );
  _pref[0]->setSuffix( " hPa" );

  _pref[1] = new QDoubleSpinBox;
  _pref[1]->setButtonSymbols( QDoubleSpinBox::NoButtons );
  _pref[1]->setAlignment( Qt::AlignRight );
  _pref[1]->setRange( 1000, 1500 );
  _pref[1]->setSingleStep( 10 );
  _pref[1]->setDecimals( 0 );
  _pref[1]->setValue( 1 );
  _pref[1]->setSuffix( " hPa" );
  
  scalLo->addWidget( tp1Lb, 0, 0 );
  scalLo->addWidget( fma1Lb, 0, 1 );
  scalLo->addWidget( _tp1[0], 0, 2 );
  scalLo->addWidget( tma1Lb, 0, 3 );
  scalLo->addWidget( _tp1[1], 0, 4 );

  scalLo->addWidget( tp2Lb, 1, 0 );
  scalLo->addWidget( fma2Lb, 1, 1 );
  scalLo->addWidget( _tp2[0], 1, 2 );
  scalLo->addWidget( tma2Lb, 1, 3 );
  scalLo->addWidget( _tp2[1], 1, 4 );

  scalLo->addWidget( flowLb, 2, 0 );
  scalLo->addWidget( fma3Lb, 2, 1 );
  scalLo->addWidget( _flow[0], 2, 2 );
  scalLo->addWidget( tma3Lb, 2, 3 );
  scalLo->addWidget( _flow[1], 2, 4 );

  scalLo->addWidget( pLinLb, 3, 0 );
  scalLo->addWidget( fma4Lb, 3, 1 );
  scalLo->addWidget( _plin[0], 3, 2 );
  scalLo->addWidget( tma4Lb, 3, 3 );
  scalLo->addWidget( _plin[1], 3, 4 );

  scalLo->addWidget( pRefLb, 4, 0 );
  scalLo->addWidget( fma5Lb, 4, 1 );
  scalLo->addWidget( _pref[0], 4, 2 );
  scalLo->addWidget( tma5Lb, 4, 3 );
  scalLo->addWidget( _pref[1], 4, 4 );

  scalingBox->setLayout( scalLo );
  
  // Other parameters
  QHBoxLayout *otherLo = new QHBoxLayout;

  QLabel *lnLb = new QLabel( "Line number " );
  _lnNum = new QSpinBox;
  _lnNum->setButtonSymbols( QSpinBox::NoButtons );
  _lnNum->setAlignment( Qt::AlignRight );
  _lnNum->setRange( 1, 7 );
  _lnNum->setSingleStep( 1 );
  _lnNum->setValue( 1 );
  _lnNum->setFixedWidth( 40 ); 

  QLabel *pwLb = new QLabel( "      Passkey " );
  _psk = new QSpinBox;
  _psk->setButtonSymbols( QSpinBox::NoButtons );
  _psk->setAlignment( Qt::AlignRight );
  _psk->setRange( 0, 9999 );
  _psk->setSingleStep( 1 );
  _psk->setValue( 0 );

  QLabel *smLb = new QLabel( "      Set Sleep Mode "  );
  _smp = new QCheckBox();
  
  otherLo->addWidget( lnLb );
  otherLo->addWidget( _lnNum );
  otherLo->addWidget( pwLb );
  otherLo->addWidget( _psk );
  otherLo->addWidget( smLb );
  otherLo->addWidget( _smp );

  othersBox->setLayout( otherLo );

  // Buttons
  QHBoxLayout *btnLo = new QHBoxLayout;
  
  _reloadButton = new QPushButton( tr("Reload" ), this );
  _reloadButton->setFixedSize( QSize( 100, 30 ) );
   connect( _reloadButton, SIGNAL( clicked() ), this, SLOT( reload() ) );
  _reloadButton->setEnabled( false );
  
  _sendButton = new QPushButton( tr("Send" ), this );
  _sendButton->setFixedSize( QSize( 100, 30 ) );
   connect( _sendButton, SIGNAL( clicked() ), this, SLOT( send() ) );
  _sendButton->setEnabled( false );

  _quitButton = new QPushButton( tr("Quit" ), this );
  _quitButton->setFixedSize( QSize( 100, 30 ) );
  _quitButton->setStyleSheet( " background-color: rgb( 100, 0, 0 )" );
  connect( _quitButton, SIGNAL( clicked() ), this, SLOT( close() ) );
  
  btnLo->addWidget( _reloadButton ); 
  btnLo->addWidget( _sendButton );
  btnLo->addWidget( _quitButton ); 
  buttonBox->setLayout( btnLo );

  // Status window
  _statusBoxText = new QTextEdit();
  _statusBoxText->setReadOnly( true );
  _statusBoxText->setWindowTitle( "Status Monitor" );

  _statusBoxHoldReleaseScroll = new QPushButton( tr( "Hold" ), this );
  _statusBoxHoldReleaseScroll->setStyleSheet( "background-color: rgb( 0, 50, 50 )" );
  connect( _statusBoxHoldReleaseScroll, SIGNAL( clicked() ), this, SLOT( statusBoxHoldReleaseScroll() ) );

  QPushButton *statusBoxClearConsole = new QPushButton( tr( "Clear" ), this );
  statusBoxClearConsole->setStyleSheet( "background-color: rgb( 0, 50, 50 )" );
  connect( statusBoxClearConsole, SIGNAL( clicked() ), this, SLOT( statusBoxClearConsole() ) );

  QGridLayout *statusBoxLayout = new QGridLayout;
  statusBoxLayout->addWidget( _statusBoxText, 0, 0, 1, 8 );
  statusBoxLayout->addWidget( _statusBoxHoldReleaseScroll, 1, 0 );
  statusBoxLayout->addWidget( statusBoxClearConsole, 1, 1 );
  statusBox->setLayout( statusBoxLayout );

}

// +----------------------+
// | reset displayed data |
// +----------------------+
void MainWindow::reload( void ) {
  _showRemoteData();
}

// +------------------------+
// | set communication mode |
// +------------------------+
void MainWindow::setMode( int mode ) {
  _devMode = mode;
}

// +-----------------------------+
// | send parameters to 4ventino |
// +-----------------------------+
void MainWindow::send( void ) {
  _updateEepromContent();
  std::string s = "get";
  s += _encodeRemoteData();
  //std::cout << s << "<br>" << std::endl;
  _client->clientWrite( s );
  usleep( 800000 );
  std::string status = _client->clientRead();
  if( status.compare( 0, 4, "Done" ) == 0 ){
    std::cout << "<font color=#00FF50>" << status << "</font><br>" << std::endl;
  }
  else {
    std::cout << "<font color=#FF0000>" << status << "</font><br>" << std::endl;
  }
  //_dumpEepromContent();
  updateStatusConsole();
}

// +---------------------------------------+
// | connect to/disconnect from a 4ventino |
// +---------------------------------------+
void MainWindow::deviceConnectDisconnect( void ) {

  if( _devConnected ) {
    try {
      std::cout << "<font color=#00FF55>Trying to disconnect from device</font><br>" << std::endl;
      updateStatusConsole();
      _client->clientDisconnect();
      _devConnected = false;
      _connectButton->setText( tr("Connect" ) );
      std::cout << "<font color=#00FF55> Device disconnected</font><br>" << std::endl;
      updateStatusConsole();
      _connectButton->setEnabled( true );
    } catch( std::string error ) {
      std::cout << "<font color=#FF0000>Device disconnection attempt failed: " << error << "</font><br>" << std::endl;
      updateStatusConsole();
      _connectButton->setEnabled( true );
      return;
    }
    return;
  }
  _connectButton->setEnabled( false );
  std::string modelOrAddress = "", serialNumberOrPort = "";

  if( _devMode == 1 ) {
    modelOrAddress = _sdevModel->text().toStdString();
    serialNumberOrPort = _sdevSerNum->currentText().toStdString();
  }
  else {
    modelOrAddress = std::to_string( _edevAddr[0]->value() ) + "." + std::to_string( _edevAddr[1]->value() ) + "." +
                     std::to_string( _edevAddr[2]->value() ) + "." + std::to_string( _edevAddr[3]->value() );
    serialNumberOrPort = std::to_string( _edevPrt->value() );
  }
  
  try {
    std::cout << "<font color=#00FF55>Trying to connect to " << modelOrAddress << " on port " << serialNumberOrPort << "</font><br>" << std::endl;
    updateStatusConsole();
    _client->clientConnect( _devMode, modelOrAddress, serialNumberOrPort );
    if( _devMode == 1 ) usleep( 600000 ); 
    std::string command = "vers";   
    _client->clientWrite( command );
    usleep( 500000 );
    _devConnected = true;
    std::string vers = _client->clientRead();
    std::cout << std::endl << "<font color=#00FF55>Connected! </font><font color=#FFFF00>Firmware Version: " << vers << "</font><br>" << std::endl;
    updateStatusConsole();
    _connectButton->setText( tr("Disconnect" ) );
    _connectButton->setEnabled( true );
  } catch( std::string error ) {
    std::cout << "<font color=#FF0000>Device connection attempt failed: " << error << "</font><br>" << std::endl;
    updateStatusConsole();
    _connectButton->setEnabled( true );
    return;
  }
  try {
    if( _devMode == 1 ) usleep( 600000 );     
    std::string command = "give";
    std::cout << "<font color=#00FF55>Asking for current EEPROM configuration.</font><br>" << std::endl;
    updateStatusConsole();
    _client->clientWrite( command );
    usleep( 500000 );
    std::string s = "";
    int count = 0;
    while( s == std::string("") && count++ < 20 ) {
      s = _client->clientRead();
      usleep( 100000 );
    }
    if( _decodeRemoteData ( s ) ) {
      std::cout << "<font color=#00FF55>Got current EEPROM configuration.</font><br>" << std::endl;
      updateStatusConsole();
      _showRemoteData();
      _sendButton->setEnabled( true );
      _reloadButton->setEnabled( true );
    }
    else {
      std::cout << "<font color=#FF0000>Failed in getting EEPROM configuration.</font><br>" << std::endl;
      updateStatusConsole();
    }
  } catch( std::string error ) {
    std::cout << "<font color=#FF0000>Error during data transfer from/to device: " << error << "</font><br>" << std::endl;
    updateStatusConsole();
    return;
  }  
}

// +--------------------------------+
// | Hold/Release console scrolling |
// +--------------------------------+
void MainWindow::statusBoxHoldReleaseScroll( void ) {
  if( _statusBoxHoldScrollFlag ) {
    _statusBoxHoldReleaseScroll->setText( tr( "Hold" ) );
    _statusBoxHoldScrollFlag = false;
  }  else {
    _statusBoxHoldReleaseScroll->setText( tr( "Release" ) );
    _statusBoxHoldScrollFlag = true;
  }
}

// +---------------+
// | clear console |
// +---------------+
void MainWindow::statusBoxClearConsole( void ) {
  consoleText.clear();
  updateStatusConsole();  
}

// +----------------+
// | update console |
// +----------------+
void MainWindow::updateStatusConsole( void ) {
  if( _statusBoxHoldScrollFlag ) return;
  _statusBoxText->setText( QString::fromStdString( consoleText ) );
  QScrollBar *sb = _statusBoxText->verticalScrollBar();
  sb->setValue( sb->maximum() );
}

// +-------------------------------------------------+
// | convert received string to EEPROMContent struct |
// +-------------------------------------------------+
bool MainWindow::_decodeRemoteData( std::string s ) {
    updateStatusConsole();
  if( s[0] != 'S' && s[s.length()-2] != 'E' ) {
    return false;
  }
  s = s.substr( 1 );
  if( s[0] != 'B' ) { return( false ); };
  s = s.substr( 1 ); // line
  if( isByte( s.substr( 0, 2 ) ) ) {
    _eepromContent.line = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  if( s[0] != 'B' ) { return( false ); };
  s = s.substr( 1 ); // sleep
  if( isByte( s.substr( 0, 2 ) ) ) {
    _eepromContent.sleep = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
  }
  for( int i=0; i<5; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // passkey*5
      _eepromContent.passkey[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // ip*4
      _eepromContent.ip[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // dns*4
      _eepromContent.dns[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // gw*4
      _eepromContent.gw[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  for( int i=0; i<4; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // snm*4
      _eepromContent.snm[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  for( int i=0; i<32; i++ ) { 
    s = s.substr( 2 );
    if( s[0] != 'B' ) { return( false ); };
    s = s.substr( 1 ); 
    if( isByte( s.substr( 0, 2 ) ) ) { // ntp*32
      _eepromContent.ntp[i] = std::stod( std::string( "0x" ) + s.substr( 0, 2 ) );
    }
  }
  s = s.substr( 2 );
  for( int i=0; i<5; i++ ) { 
    if( s[0] != 'F' ) { return( false ); };
    s = s.substr( 1 );
    int pos = s.find( "F" );
    if( pos == -1 ) pos = s.length()-3;
    std::string val = s.substr( 0, pos );
    if( isFloat( val ) ) {
      _eepromContent.min[i] = std::stof( val );
    }
    else {
      _eepromContent.min[i] = -9999.;
    }
    s = s.substr( pos );
  }

  for( int i=0; i<5; i++ ) { 
    if( s[0] != 'F' ) { return( false ); };
    s = s.substr( 1 );
    int pos = s.find( "F" );
    if( pos == -1 ) pos = s.length()-3;
    std::string val = s.substr( 0, pos );
    if( isFloat( val ) ) {
      _eepromContent.max[i] = std::stof( val );
    }
    else {
      _eepromContent.max[i] = -9999.;
    }
    s = s.substr( pos );
  }
  if( s[0] != 'E' ) {
    return( false ); 
  }
  return true;
}

// +----------------------------------+
// | show 4ventino setting parameters |
// +----------------------------------+
void MainWindow::_showRemoteData( void ) {

  _lnNum->setValue( _eepromContent.line );
  if( _eepromContent.sleep ) {
    _smp->setCheckState( Qt::Checked );
  }
  else {
    _smp->setCheckState( Qt::Unchecked );
  }
  _psk->setValue( std::stod( _eepromContent.passkey ));
  for( int i=0; i<4; i++ ) {
    _ipAddr[i]->setValue( _eepromContent.ip[i] );
    _dnsAddr[i]->setValue( _eepromContent.dns[i] );
    _gwAddr[i]->setValue( _eepromContent.gw[i] );
    _nwMask[i]->setValue( _eepromContent.snm[i] );
  }
  std::string s = _eepromContent.ntp;
  for( int i=0; i<4; i++ ) {
    _ntAddr[i]->setValue( std::stod( s.substr( 0, s.find( "." ) ) ) );
    s = s.substr( s.find( "." )+1 );
  }
  updateStatusConsole();
  _tp1[0]->setValue( _eepromContent.min[0] );
  _tp1[1]->setValue( _eepromContent.max[0] );
  _tp2[0]->setValue( _eepromContent.min[1] );
  _tp2[1]->setValue( _eepromContent.max[1] );
  _flow[0]->setValue( _eepromContent.min[2] );
  _flow[1]->setValue( _eepromContent.max[2] );
  _plin[0]->setValue( _eepromContent.min[3] );
  _plin[1]->setValue( _eepromContent.max[3] );
  _pref[0]->setValue( _eepromContent.min[4] );
  _pref[1]->setValue( _eepromContent.max[4] );
  
  return;   
}

// +---------------------------------------------------------------+
// | encode EEPROMContent struct to a string to be sent to 4ventino|
// +---------------------------------------------------------------+
std::string MainWindow::_encodeRemoteData( void ) {
  std::string s = "S";
  s += "B" + toHex( uint8_t( _eepromContent.line & 0xFF )) + "B" + toHex( uint8_t( _eepromContent.sleep & 0xFF ));
  for( int i=0; i<5; i++ ) {
    s += "B" + toHex( _eepromContent.passkey[i] );
  }
  for( int i=0; i<4; i++ ) {
    s += "B" + toHex( _eepromContent.ip[i] );
  }
  for( int i=0; i<4; i++ ) {
    s += "B" + toHex( _eepromContent.dns[i] );
  }
  for( int i=0; i<4; i++ ) {
   s += "B" + toHex( _eepromContent.gw[i] );
  }
  for( int i=0; i<4; i++ ) {
    s += "B" + toHex( _eepromContent.snm[i] );
  }
  for( int i=0; i<32; i++ ) {
    s += "B" + toHex( _eepromContent.ntp[i] );
  }
  for( int i=0; i<5; i++ ) {
    s += "F" + toStr( _eepromContent.min[i] );
  }
  for( int i=0; i<5; i++ ) {
    s += "F" + toStr( _eepromContent.max[i] );
  }
  s += "E";
  return( s );
}

// +-------------------------------------------------+
// | update EEPROMContent struct with displayed data |
// +-------------------------------------------------+
void MainWindow::_updateEepromContent( void ) {
  _eepromContent.line = _lnNum->value();
  if( _smp->isChecked() ) {
    _eepromContent.sleep = 1;
  }
  else {
    _eepromContent.sleep = 0;
  }
  strncpy( _eepromContent.passkey, std::to_string( _psk->value() ).c_str(), 4 );
  _eepromContent.passkey[4] = '\0';
  for( int i=0; i<4; i++ ) {
    _eepromContent.ip[i]  = _ipAddr[i]->value();
    _eepromContent.dns[i] = _dnsAddr[i]->value();
    _eepromContent.gw[i]  = _gwAddr[i]->value();
    _eepromContent.snm[i] = _nwMask[i]->value();
  }
  std::stringstream s; s << std::dec << _ntAddr[0]->value() << "." << _ntAddr[1]->value() << "." << _ntAddr[2]->value() << "." << _ntAddr[3]->value();
  strncpy( _eepromContent.ntp, s.str().c_str(), 31 );
  _eepromContent.ntp[31] = '\0';
  _eepromContent.min[0] = _tp1[0]->value();
  _eepromContent.max[0] = _tp1[1]->value();
  _eepromContent.min[1] = _tp2[0]->value();
  _eepromContent.max[1] = _tp2[1]->value();
  _eepromContent.min[2] = _flow[0]->value();
  _eepromContent.max[2] = _flow[1]->value();
  _eepromContent.min[3] = _plin[0]->value();
  _eepromContent.max[3] = _plin[1]->value();
  _eepromContent.min[4] = _pref[0]->value();
  _eepromContent.max[4] = _pref[1]->value();
  return;    
}

// +-----------------------------------------------------------------------+
// | dump EEPROMContent struct content to console. Used for debug purposes |
// +-----------------------------------------------------------------------+
void MainWindow::_dumpEepromContent( void ) {
  std::cout << "line:    " << _eepromContent.line << "<br>" << std::endl;
  std::cout << "sleep:   " << int( _eepromContent.sleep ) << "<br>" << std::endl;
  std::cout << "passkey: " << _eepromContent.passkey << "<br>" << std::endl;
  std::cout << "ip:     ";
  for( int i=0; i<4; i++ ) {
    std::cout << int( _eepromContent.ip[i] ) << ".";
  }
  std::cout << "<br>" << std::endl;
  std::cout << "dns:    ";
  for( int i=0; i<4; i++ ) {
    std::cout << int( _eepromContent.dns[i] ) << ".";
  }
  std::cout << "<br>" << std::endl;
  std::cout << "gw:     ";
  for( int i=0; i<4; i++ ) {
    std::cout << int( _eepromContent.gw[i] ) << ".";
  }
  std::cout << "<br>" << std::endl;
  std::cout << "snm:    ";
  for( int i=0; i<4; i++ ) {
    std::cout << int( _eepromContent.snm[i] ) << ".";
  }
  std::cout << "<br>" << std::endl;
  std::cout << "ntp:    " << _eepromContent.ntp << "<br>" << std::endl;
  for( int i=0; i<5; i++ ) {
    std::cout << i << " min: " << _eepromContent.min[i] << ", max: " << _eepromContent.max[i] << "<br>" << std::endl;
  }
  std::cout << "<br>" << std::endl;
  updateStatusConsole();
  return;
}
