#!/bin/bash
########################################################################
# Begin $rc_base/init.d/wpa_supplicant
#
# Description : WPA Supplicant Control Script
#
# Authors     : Rachel Mant - rachel@rachelmant.com
#
# Version     : 00.00
#
# Notes       :
#
########################################################################

. /etc/sysconfig/rc
. ${rc_functions}

case "$1" in
	start)
		boot_mesg -n "Starting wpa_supplicant.." ${SUCCESS}
		boot_mesg "" ${NORMAL}
		if [ ! -f /var/run/wpa_supplicant.pid ]; then
			/sbin/wpa_supplicant -B -P/var/run/wpa_supplicant.pid -Dwired -c/etc/wpa_supplicant.conf -W -ieth0
		else
			boot_mess "Already running!" ${WARNING}
			(exit 1)
		fi
		evaluate_retval
		;;
	stop)
		boot_mesg "Stopping wpa_supplicant.." ${FAILURE}
		killproc /sbin/wpa_supplicant
		echo_ok
		;;
	restart)
		"${0}" stop
		sleep 1
		"${0}" start
		;;
	*)
	echo "Usage: ${0} {start|stop|restart}"
		exit 1
		;;
esac

# End $rc_base/init.d/wpa_supplicant
