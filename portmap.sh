#!/bin/bash
# Begin $rc_base/init.d/portmap

#$LastChangedBy: bdubbs $
#$Date: 2005-08-01 14:29:19 -0500 (Mon, 01 Aug 2005) $

. /etc/sysconfig/rc
. $rc_functions

case "$1" in
	start)
		boot_mesg "Starting RPC Portmap"
		loadproc /sbin/portmap
		;;

	stop)
		boot_mesg "Stopping Portmap"
		killproc /sbin/portmap
		;;

	restart)
		$0 stop
		sleep 1
		$0 start
		;;

	status)
		statusproc /sbin/portmap
		;;

	*)
		echo "Usage: $0 {start|stop|restart|status}"
		exit 1
		;;

esac

# End $rc_base/init.d/portmap
