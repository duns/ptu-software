CFLAGS+= -Wall -W -g -O2 -D'IFUPDOWN_VERSION="0.6.5"'

CFILES := 
HFILES := 

OBJ := commandserver.sh

all : commandserver

commandserver : $(OBJ)
#	$(CC) $(CFLAGS) $^ $(LDFLAGS)
	cp commandserver.sh commandserver
	chmod 755 commandserver

install:
#	install -m 0755 -d     ${DESTDIR}/etc/init.d
#	install -m 0644 commandserver.conf ${DESTDIR}/etc/
#	install -m 0755 commandserver-initscript ${DESTDIR}/etc/init.d/commandserver
	install -m 0755 -d     ${DESTDIR}/sbin
	install -m 0755 commandserver ${DESTDIR}/sbin

clean : 
	rm -f *.o commandserver
.PHONY : clean install
