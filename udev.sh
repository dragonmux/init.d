#!/bin/bash
########################################################################
# Begin $rc_base/init.d/udev
#
# Description : Udev Boot Script
#
# Authors     : Based on Open Suse Udev Rules
#               kay.sievers@suse.de
#
# Adapted to  : Jim Gifford
# LFS	      : Alexander E. Patrakov
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
	boot_mesg -n "Creating /dev in tmpfs..." ${INFO}
	boot_mesg "" ${NORMAL}
	mount -n -t tmpfs -o mode=0755 udev /dev
	evaluate_retval

	boot_mesg "Copying static entries..." ${INFO}
	cp --preserve=all --recursive --remove-destination /lib/udev/devices/* /dev
	evaluate_retval
	
	boot_mesg "Setting Permissons on /dev/shm..." ${INFO}
	chmod 1777 /dev/shm
	evaluate_retval

	echo "" > /sys/kernel/uevent_helper

	# start udevd
	boot_mesg "Starting udevd..." ${SUCCESS}
	/sbin/udevd --daemon
	evaluate_retval

	# start coldplugging
	boot_mesg -n "Performing Coldplugging..." ${INFO}
	boot_mesg "" ${NORMAL}

	# unlikely, but we may be faster than the first event
	mkdir -p /dev/.udev/queue

	# configure all devices
	/sbin/udevadm trigger
	
	# this replaces the old loop, exits after all devices are done
	/sbin/udevadm settle

	echo_ok
	;;

    stop)
	boot_mesg "Stopping udevd..." ${FAILURE}
	killproc /sbin/udevd
	;;

    restart)
	boot_mesg "Restarting udevd..." ${WARNING}
	killproc /sbin/udevd
	loadproc /sbin/udevd --daemon
	evaluate_retval
	;;

    status)
	statusproc /sbin/udevd
	;;

    reload)
	boot_mesg "Reloading udev rules..." ${INFO}
	udevadm control reload_rules
	cp --preserve=all --recursive --update /lib/udev/devices/* /dev
	evaluate_retval
	;;

    force-reload)
	boot_mesg "Updating all available device nodes in /dev..." ${INFO}
	udevadm control reload_rules
	rm -rf /dev/.udev /dev/disk
	cp --preserve=all --recursive --update /lib/udev/devices/* /dev
	/sbin/udevadm trigger
	/sbin/udevadm settle
	evaluate_retval
	;;

    *)
	echo "Usage: $0 {start|stop|restart|status|reload|force-reload}"
	exit 1
	;;
esac
