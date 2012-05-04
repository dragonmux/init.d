#!/bin/bash
# Begin $rc_base/init.d/alsa

# Based on sysklogd script from LFS-3.1 and earlier.
# Rewritten by Gerard Beekmans  - gerard@linuxfromscratch.org
# ALSA specific parts by Mark Hymers - markh@linuxfromscratch.org
# Stores mixer settings in the default location: /etc/asound.state

#$LastChangedBy: bdubbs $
#$Date: 2005-08-01 14:29:19 -0500 (Mon, 01 Aug 2005) $

. /etc/sysconfig/rc
. $rc_functions

case "$1" in
	stop)
		boot_mesg "Stopping ALSA...    Saving volumes..." ${INFO}
		loadproc /usr/sbin/alsactl store
		#boot_mesg "                    Removing MIDI font..."
		#loadproc sfxload -i
		;;

	*)
		echo "Usage: $0 stop"
		exit 1
		;;
esac

# End $rc_base/init.d/alsa
