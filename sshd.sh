#!/bin/bash
# Begin $rc_base/init.d/sshd

# Based on sysklogd script from LFS-3.1 and earlier.
# Rewritten by Gerard Beekmans  - gerard@linuxfromscratch.org

#$LastChangedBy: bdubbs $
#$Date: 2006-04-15 17:34:16 -0500 (Sat, 15 Apr 2006) $

. /etc/sysconfig/rc
. $rc_functions

pidfile=/var/run/sshd.pid

case "$1" in
    start)
        if [ ! -f /etc/ssh/ssh_host_key ]; then
          boot_mesg "Generating /etc/ssh/ssh_host_key"
          ssh-keygen -t rsa1 -f /etc/ssh/ssh_host_key -N ""
          evaluate_retval
        fi
        if [ ! -f /etc/ssh/ssh_host_dsa_key ]; then
          boot_mesg "Generating /etc/ssh/ssh_host_dsa_key"
          ssh-keygen -t dsa -f /etc/ssh/ssh_host_dsa_key -N ""
          evaluate_retval
        fi
        if [ ! -f /etc/ssh/ssh_host_rsa_key ]; then
          boot_mesg "Generating /etc/ssh/ssh_host_rsa_key"
          ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key -N ""
          evaluate_retval
        fi

        boot_mesg "Starting SSH Server..."
        # Also prevent ssh from being killed by out of memory conditions
        loadproc -p $pidfile /usr/sbin/sshd 
        sleep 1
        echo "-16" >/proc/`cat $pidfile`/oom_adj
        ;;

    stop)
        boot_mesg "Stopping SSH Server..."
        killproc -p $pidfile /usr/sbin/sshd
        ;;

    reload)
        boot_mesg "Reloading SSH Server..."
        reloadproc -p $pidfile /usr/sbin/sshd
        ;;

    restart)
        $0 stop
        sleep 1
        $0 start
        ;;

    status)
        statusproc -p $pidfile /usr/sbin/sshd
        ;;

    *)
        echo "Usage: $0 {start|stop|reload|restart|status}"
        exit 1
        ;;
esac

# End $rc_base/init.d/sshd
