include Makefile.inc

CFLAGS = -c $(OPTIM_FLAGS) -pthread -o $@ $<
LFLAGS = $(filter %.o, $^) -pthread -o $@

O_EXTRA = bashSource.o
EXE = alsa apache dbus mysql synergys i18n swap sshd udev checkfs halt consolelog reboot rsyslog localnet setclock mountfs samba

default: all

all: $(O_EXTRA) $(EXE)
	$(call run-cmd,rm,$(O_EXTRA:.o=.c),$(O_EXTRA:.o=.c))

install: all
	$(call run-cmd,install_bin,$(EXE),/etc/rc.d/init.d)

clean:
	$(call run-cmd,rm,init.d,$(patsubst %,%.o,$(EXE)) $(EXE) $(O_EXTRA))

.o:
	$(call run-cmd,ccld,$(LFLAGS))
#	$(call run-cmd,chmod,$@)
	$(call debug-strip,$@)

%.o: %.c functons.h
.c.o:
	$(call run-cmd,cc,$(CFLAGS))

.y.c:
	$(call run-cmd,flex)

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
mountfs.o: mountfs.c
samba.o: samba.c

bashSource.o: bashSource.y
