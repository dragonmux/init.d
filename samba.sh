#!/bin/bash
# Begin $rc_base/init.d/samba

# Based on sysklogd script from LFS-3.1 and earlier.
# Rewritten by Gerard Beekmans  - gerard@linuxfromscratch.org

#$LastChangedBy: bdubbs $
#$Date: 2005-08-01 14:29:19 -0500 (Mon, 01 Aug 2005) $

. /etc/sysconfig/rc
. $rc_functions

smbdpid=/var/run/smbd.pid
nmbdpid=/var/run/nmbd.pid

case "$1" in
	start)
		boot_mesg "Starting nmbd..."
		loadproc -p $nmbdpid /usr/sbin/nmbd -D

		boot_mesg "Starting smbd..."
		loadproc -p $smbdpid /usr/sbin/smbd -D
		;;

	stop)
		boot_mesg "Stopping smbd..."
		killproc -p $smbdpid /usr/sbin/smbd

		boot_mesg "Stopping nmbd..."
		killproc -p $nmbdpid /usr/sbin/nmbd
                ;;

	reload)
		boot_mesg "Reloading smbd..."
		reloadproc -p $smbdpid /usr/sbin/smbd

		boot_mesg "Reloading nmbd..."
		reloadproc -p $nmbdpid /usr/sbin/nmbd
		;;

	restart)
		$0 stop
		sleep 1
		$0 start
		;;

	status)
		statusproc -p $nmbdpid /usr/sbin/nmbd
		statusproc -p $smbdpid /usr/sbin/smbd
		;;

	*)
		echo "Usage: $0 {start|stop|reload|restart|status}"
		exit 1
		;;
esac

# End $rc_base/init.d/samba
