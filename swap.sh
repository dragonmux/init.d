#!/bin/bash
########################################################################
# Begin $rc_base/init.d/swap
#
# Description : Swap Control Script
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
		boot_mesg -n "Activating all swap files/partitions.." ${INFO}
		boot_mesg "" ${NORMAL}
		swapon -a
		evaluate_retval
		;;

	stop)
		boot_mesg -n "Deactivating all swap files/partitions.." ${INFO}
		boot_mesg "" ${NORMAL}
		swapoff -a
		evaluate_retval
		;;

	restart)
		${0} stop
		sleep 1
		${0} start
		;;

	status)
		boot_mesg "Retrieving swap status.." ${INFO}
		echo_ok
		echo
		swapon -s
		;;

	*)
		echo "Usage: ${0} {start|stop|restart|status}"
		exit 1
		;;
esac

# End $rc_base/init.d/swap
