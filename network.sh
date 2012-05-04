#!/bin/bash
########################################################################
# Begin $rc_base/init.d/network
#
# Description : Network Control Script
#
# Authors     : Gerard Beekmans - gerard@linuxfromscratch.org
#		Nathan Coulson - nathan@linuxfromscratch.org
#		Kevin P. Fleming - kpfleming@linuxfromscratch.org
#
# Version     : 00.00
#
# Notes       :
#
########################################################################

. /etc/sysconfig/rc
. ${rc_functions}

if [ ! -f /etc/sysconfig/network ]; then \
	boot_mesg "Cannot change network state: /etc/sysconfig/network missing!" ${INFO}; \
	echo_failure; \
	exit 0; \
fi
. /etc/sysconfig/network

case "${1}" in
	start)
		boot_mesg "Bringing up network interfaces.." ${INFO}
		# Start all network interfaces
		for file in ${network_devices}/ifconfig.*
		do
			interface=${file##*/ifconfig.}

			# skip if $file is * (because nothing was found)
			if [ "${interface}" = "*" ]
			then
				continue
			fi

			IN_BOOT=1 ${network_devices}/ifup ${interface}
		done
		echo_ok
		;;

	stop)
		boot_mesg "Taking down network interfaces.." ${INFO}
		# Reverse list
		FILES=""
		for file in ${network_devices}/ifconfig.*
		do
			FILES="${file} ${FILES}"
		done

		# Stop all network interfaces
		for file in ${FILES}
		do
			interface=${file##*/ifconfig.}

			# skip if $file is * (because nothing was found)
			if [ "${interface}" = "*" ]
			then
				continue
			fi

			IN_BOOT=1 ${network_devices}/ifdown ${interface}
		done
		echo_ok
		;;

	restart)
		${0} stop
		sleep 1
		${0} start
		;;

	*)
		echo "Usage: ${0} {start|stop|restart}"
		exit 1
		;;
esac

# End /etc/rc.d/init.d/network
