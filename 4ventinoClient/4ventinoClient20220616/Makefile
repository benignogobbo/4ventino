#
# To build the things
# Benigno Gobbo 
# 20200218
#

CC     = g++
#CFLAGS = -O -std=c++11
CFLAGS = -g -std=c++11

all: single multi oldtable table tps reset tofile reconnect eeprom

single: single.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o single single.cc -L. -lqventino

multi : multi.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o multi multi.cc -L. -lqventino

oldtable : oldtable.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o oldtable oldtable.cc -L. -lqventino

table : table.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o table table.cc -L. -lqventino

tps : tablePlus7.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o tps tablePlus7.cc -L. -lqventino

reset : reset.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o reset reset.cc -L. -lqventino

tofile : tofile.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o tofile tofile.cc -L. -lqventino

reconnect : reconnect.cc qventino.h libqventino.a
	$(CC) $(CFLAGS) -o reconnect reconnect.cc -L. -lqventino

eeprom : libqventino.a
	cd EEPROM && $(MAKE) 

libqventino.a: qventinoInit.o qventinoClient.o qventinoEthernet.o qventinoSerial.o qventinoUtils.o
	rm -f libqventino.a
	ar rv libqventino.a qventinoInit.o qventinoClient.o qventinoEthernet.o qventinoSerial.o qventinoUtils.o

qventinoUtils.o: qventinoUtils.cc qventinoUtils.h
	$(CC) $(CFLAGS) -c qventinoUtils.cc

qventinoSerial.o: qventinoSerial.cc qventinoSerial.h
	$(CC) $(CFLAGS) -c qventinoSerial.cc

qventinoEthernet.o: qventinoEthernet.cc qventinoEthernet.h
	$(CC) $(CFLAGS) -c qventinoEthernet.cc

qventinoClient.o: qventinoClient.cc qventinoClient.h
	$(CC) $(CFLAGS) -c qventinoClient.cc

qventinoInit.o: qventinoInit.cc qventinoInit.h
	$(CC) $(CFLAGS) -c qventinoInit.cc

clean: 
	rm -f *.o *.a *~ libqventino.a single multi oldtable table tps reset tofile reconnect
	cd EEPROM && $(MAKE) clean && rm qventinoeeprom

