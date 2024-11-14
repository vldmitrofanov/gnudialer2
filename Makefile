SHELL=/bin/sh

# Feel free to comment out the "guessed values" and enter the paths explicitly
# if you have troubles getting them from ./mkhelper or mysql_config.

RUNAS := `ps wwwaux | grep asterisk  | sed -e 's/ /\n/' | sed -e 's/ /\n/' | head -n 1`
DIST := `./detect_os.sh`
UB=/usr/bin

#MYSQLVER := `mysql -V | cut -c25-27`    
MYSQLVER := $(shell mysql_config --version)
# moved from c++11
CXXFLAGS = -std=c++20 -Wall -I/usr/include/mysql -I/usr/include/jdbc -DDEBUG

# Linker flags
LDFLAGS = -L/usr/lib/mysql -lmysqlcppconn -lcurl 

ifeq ($(shell `detect`),RedHat)
 LMYSQLCLIENT := `mysql_config --libs | sed -e 's/ /\n/' | sed -e 's/ /\n/' | head -n 2 | tail -n 1`
else
 LMYSQLCLIENT=-lmysqlclient
endif

CC=g++ -Wall -Wshadow
#CC=g++ -Weffc++

all: printdistinfo printrunasinfo printasteriskv printmysqlver printmysqlinc thegnudialer #dialeradmin agenthours stats newstats allstats up
	@echo
	@echo "Done! Now type \"make install\" to install."
	@echo

printdistinfo:
	@echo
	@echo "Checking Linux Dist Info..."
	@echo
	@echo "DISTRIBUTION: " ${DIST}
	@echo

printrunasinfo:
	@echo
	@echo "Asterisk being RUN..."
	@echo
	@echo "AS: " ${RUNAS}
	@echo
                                        

printmysqlver:
	@echo
	@echo "Checking MySQL Version..."
	@echo
	@echo "MYSQL VERSION: " $(MYSQLVER)
	@echo
	@echo "THIS MUST SAY 4.1 OR HIGHER!!!!!!"
	@echo "CURRENTLY THERE ARE NO CHECKS TO STOP COMPILING IF TOO OLD!"                
	@echo

printmysqlinc:
	@echo
	@echo "Checking for mysql_config..."
	@echo
	@locate mysql_config
	@echo
	@echo "OK."
	@echo

printasteriskv:
	@echo
	@echo "Checking for asterisk..."
	@echo
	@/usr/sbin/asterisk -V
	@echo
	@echo "OK."
	@echo

thegnudialer:
	$(CC) gnudialer2.cpp DBConnection.cpp HttpClient.h -g -o gnudialer2 `mysql_config --include` `mysql_config --libs` $(CXXFLAGS) $(LDFLAGS)

clean:
	rm gnudialer2 $(CXXFLAGS) $(LDFLAGS)

install: printdistinfo printrunasinfo printmysqlver printmysqlinc theinstall

theinstall:
	cp gnudialer2 ${UB}
	mkdir -p /usr/lib/gnudialer2
	chmod a+rw /usr/lib/gnudialer2
	mkdir -p /usr/lib/gnudialer2/timezones
	chmod a+rw /usr/lib/gnudialer2/timezones
	cp ./timezones/* /usr/lib/gnudialer2/timezones/
	mkdir -p /var/log/asterisk/gnudialer2
	chmod a+rw /var/log/asterisk/gnudialer2
	mkdir -p /tmp/exports
	mkdir -p /tmp/sales
	chmod 755 /tmp/exports
	chmod 755 /tmp/sales
	touch /tmp/bogus.helper.1-1-1980         
	chmod 666 /tmp/*.helper.*
	@echo
	@echo "Done!"
	@echo

uninstall:  
	rm ${UB}gnudialer2
	rm -rf /usr/lib/gnudialer2/timezones/
	rm -rf /tmp/exports
