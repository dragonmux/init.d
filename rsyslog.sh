#!/bin/bash
########################################################################
# Begin $rc_base/init.d/sysklogd
#
# Description : Sysklogd loader
#
# Authors     : Gerard Beekmans - gerard@linuxfromscratch.org
#
# Version     : 00.00
#
# Notes       :
#
########################################################################

. /etc/sysconfig/rc
. ${rc_functions}

case "${1}" in
	start)
		boot_mesg "Starting system log daemon..." ${SUCCESS}
		loadproc rsyslogd -c3
		;;


	stop)
		boot_mesg "Stopping system log daemon..." ${FAILURE}
		killproc rsyslogd
		;;

	reload)
		boot_mesg "Reloading system log daemon config file..." ${WARNING}
		reloadproc rsyslogd
		;;

	restart)
		${0} stop
		sleep 1
		${0} start
		;;

	status)
		statusproc syslogd
		;;

	*)
		echo "Usage: ${0} {start|stop|reload|restart|status}"
		exit 1
		;;
esac

# End $rc_base/init.d/sysklogd
