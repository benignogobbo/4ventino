// +--------------------------------------------------------+
// | 4ventino EEPROM manipulation                           |
// | Benigno Gobbo                                          |
// | © INFN Sezione di Trieste                              |
// | V0.1                                                   |
// | 2020-01-21                                             |
// +--------------------------------------------------------+

#include <iostream>
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include "window.h"

// +------------+
// |    main    |
// +------------+

int main( int argc, char **argv ) {

  QApplication application( argc, argv ); 

  application.setOrganizationName( "I.N.F.N." );
  application.setApplicationName( "4ventino EEPROM" );

  application.setStyle( QStyleFactory::create( "fusion" ) );
  QPalette palette;
  palette = application.palette();
  palette.setColor( QPalette::Window, QColor( 50, 50, 50 ) );
  palette.setColor( QPalette::WindowText, Qt::white );
  palette.setColor( QPalette::Base, Qt::black );
  palette.setColor( QPalette::AlternateBase, QColor( 25, 25, 25 ) );
  palette.setColor( QPalette::ToolTipBase, QColor( 255, 255, 220 ) );  
  palette.setColor( QPalette::ToolTipText, Qt::black );
  palette.setColor( QPalette::Text, Qt::white );
  palette.setColor( QPalette::Button, QColor( 50, 50, 50 ) );
  palette.setColor( QPalette::ButtonText, Qt::white );
  palette.setColor( QPalette::BrightText, Qt::white );
  palette.setColor( QPalette::Light, QColor( 75, 75, 75 ) );
  palette.setColor( QPalette::Midlight, QColor( 62, 62, 62 ) );
  palette.setColor( QPalette::Dark, QColor( 25, 25, 25 ) );
  palette.setColor( QPalette::Mid, QColor( 33, 33, 33 ) );
  palette.setColor( QPalette::Shadow, Qt::black );
  palette.setColor( QPalette::Highlight, QColor(42, 130, 218));
  palette.setColor( QPalette::HighlightedText, Qt::black);
  palette.setColor( QPalette::Link, QColor(42, 130, 218));
  palette.setColor( QPalette::LinkVisited, QColor(22, 110, 198));
  application.setPalette( palette );
    
  MainWindow mainWindow;
  mainWindow.show();
  
  return application.exec();  
}   

