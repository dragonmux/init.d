MAKEFLAGS = -R
GCC ?= gcc
ifeq ($(strip $(DEBUG)), 1)
	OPTIM_FLAGS = -ggdb
else
	OPTIM_FLAGS = -O2
	DEBUG = 0
endif
CC = $(GCC)
CFLAGS = -c $(OPTIM_FLAGS) -pthread -o $@ $<
LFLAGS = $(filter %.o, $^) -pthread -o $@
FLEX = flex -o $@ -8 -c $<
CHMOD = chmod 0755
STRIP = strip -s
INSTALL = install -m755

O_EXTRA = bashSource.o
EXE = alsa apache dbus mysql synergys i18n swap sshd udev checkfs halt consolelog reboot rsyslog localnet setclock

default: all

all: $(O_EXTRA) $(EXE)

install:
	$(INSTALL) $(EXE) /etc/rc.d/init.d

clean:
	rm -f *.o *~

.o: functions.h
	$(CC) $(LFLAGS)
	$(CHMOD) $@
	if [ $(DEBUG) -eq 0 ]; then $(STRIP) $@; fi

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
rsyslog.o: rsyslog.c
localnet.o: localnet.c
localnet: localnet.o bashSource.o
setclock.o: setclock.c
setclock: setclock.o bashSource.o
bashSource.o: bashSource.y
