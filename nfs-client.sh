#!/bin/bash
# Begin $rc_base/init.d/nfs-client

# Based on sysklogd script from LFS-3.1 and earlier.
# Rewritten by Gerard Beekmans  - gerard@linuxfromscratch.org

#$LastChangedBy: bdubbs $
#$Date: 2005-08-01 14:29:19 -0500 (Mon, 01 Aug 2005) $

. /etc/sysconfig/rc
. $rc_functions
. /etc/sysconfig/nfs-client

case "$1" in
	start)
		boot_mesg "Starting NFS statd..."
		loadproc /usr/sbin/rpc.statd

		if [ "$NFS4" = "yes" ]; then
			boot_mesg "Mounting rpc_pipefs virtual filesystem..."
			/bin/mount -t rpc_pipefs rpc_pipefs /var/lib/nfs/rpc_pipefs 2>&1 > /dev/null
			evaluate_retval

			boot_mesg "Starting NFS gssd..."
			loadproc /usr/sbin/rpc.gssd

			boot_mesg "Starting NFS idmapd..."
			loadproc /usr/sbin/rpc.idmapd
		fi

		;;

	stop)
		if [ "$NFS4" = "yes" ]; then
			boot_mesg "Stopping NFS idmapd..."
			killproc /usr/sbin/rpc.idmapd

			boot_mesg "Stopping NFS gssd..."
			killproc /usr/sbin/rpc.gssd

			boot_mesg "Unmounting rpc_pipefs virtual filesystem..."
			/bin/umount /var/lib/nfs/rpc_pipefs 2>&1 > /dev/null
			evaluate_retval
		fi

		boot_mesg "Stopping NFS statd..."
		killproc /usr/sbin/rpc.statd
		;;

	restart)
		$0 stop
		sleep 1
		$0 start
		;;

	status)
		statusproc /usr/sbin/rpc.statd
		if [ "$NFS4" = "yes" ]; then
			statusproc /usr/sbin/rpc.gssd
			statusproc /usr/sbin/rpc.idmapd
		fi
		;;

	*)
		echo "Usage: $0 {start|stop|restart|status}"
		exit 1
		;;
esac

# End $rc_base/init.d/nfs-client
