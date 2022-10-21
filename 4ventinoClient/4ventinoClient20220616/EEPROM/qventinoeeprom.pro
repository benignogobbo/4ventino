TARGET       = qventinoeeprom

QMAKE_CXXFLAGS += -O -pthread -std=c++11
#QMAKE_CXXFLAGS += -g -pthread -std=c++11

HEADERS =           \
    window.h        \
    console.h

SOURCES =            \
    window.cc        \
    console.cc       \
    main.cc

CONFIG += qt5

QT+=widgets

INCLUDEPATH += ../
LIBS += -L../ -lqventino
