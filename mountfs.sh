#!/bin/bash
########################################################################
# Begin $rc_base/init.d/mountfs
#
# Description : File System Mount Script
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
		boot_mesg -n "Remounting root file system in read-write mode..." ${WARNING}
		boot_mesg "" ${NORMAL}
		mount -n -o remount,rw / >/dev/null
		evaluate_retval

		# Remove fsck-related file system watermarks.
		rm -f /fastboot /forcefsck

		boot_mesg "Recording existing mounts in /etc/mtab..." ${INFO}
		> /etc/mtab
		mount -f / || failed=1
		mount -f /proc || failed=1
		mount -f /sys || failed=1
		(exit ${failed})
		evaluate_retval

		# This will mount all filesystems that do not have _netdev in
		# their option list.  _netdev denotes a network filesystem.
		boot_mesg -n "Mounting remaining file systems..." ${INFO}
		boot_mesg "" ${NORMAL}
		mount -a -O no_netdev >/dev/null
		evaluate_retval
		;;

	stop)
		boot_mesg "Unmounting all other currently mounted file systems..." ${INFO}
		umount -a -d -r >/dev/null
		evaluate_retval
		;;

	*)
		echo "Usage: ${0} {start|stop}"
		exit 1
		;;
esac

# End $rc_base/init.d/mountfs
