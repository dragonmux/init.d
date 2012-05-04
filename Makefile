GCC ?= gcc
CC = $(GCC)
CFLAGS = -c -O2 -pthread -o $@ $<
LFLAGS = $(filter %.o, $^) -pthread -o $@
FLEX = flex -o $@ -8 -c $<
CHMOD = chmod 0755
STRIP = strip -s
INSTALL = install -m755

EXE = bashSource.o alsa apache dbus mysql synergys i18n swap sshd udev checkfs halt consolelog reboot

default: all

all: $(EXE)

install:
	$(INSTALL) $(EXE) /etc/rc.d/init.d

clean:
	rm -f *.o *~

.o: functions.h
	$(CC) $(LFLAGS)
	$(CHMOD) $@
	$(STRIP) $@

.c.o: functions.h
	$(CC) $(CFLAGS)

.y.c:
	$(FLEX)

.PHONY: default all install clean .c.o .o .y.c

alsa.o: alsa.c
apache.o: apache.c
dbus.o: dbus.c
mysql.o: mysql.c
synergys.o: synergys.c
i18n.o: i18n.c
i18n: i18n.o bashSource.o
swap.o: swap.c
sshd.o: sshd.c
udev.o: udev.c
checkfs.o: checkfs.c
halt.o: halt.c
consolelog.o: consolelog.c
consolelog: consolelog.o bashSource.o
reboot.o: reboot.c
bashSource.o: bashSource.y
