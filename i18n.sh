#!/bin/bash
########################################################################
# Begin $rc_base/init.d/i18n
#
# Description : Setup for i18n
#
# Authors     : Jim Gifford
#
# Version     : 00.00
#
# Notes       :
#
########################################################################

. /etc/sysconfig/rc
. ${rc_functions}

if [ -f /etc/sysconfig/i18n ]; then
	. /etc/sysconfig/i18n
fi

case "${1}" in
	start)
		# Set the font
		#
		boot_mesg "Setting Font..." ${INFO}
		if [ "${FONT_TRANSLATION}" ]; then
			FONTTRANS="-m ${FONT_TRANSLATION}"
		fi
		for ttynum in $(seq 1 12) ; do
			setfont ${FONT} ${FONTTRANS} -C /dev/tty${ttynum}
		done
		evaluate_retval

		# Load the keymap in UTF-8
		#
		boot_mesg "Setting keymap to UTF-8..." ${INFO}
		if [ "${DUMPKEYS_CHARSET}" ]; then
			DUMPKEYS_OPTS="-c ${DUMPKEYS_CHARSET}"
		fi
		evaluate_retval

		# Set terminal encoding to UTF-8
		#
		if [ "${UNICODE}" = "yes" ] ; then
			dumpkeys ${DUMPKEYS_OPTS} | loadkeys --unicode
			TTYENCODE=$'\033%G'
			TTYMESSAGE=UTF-8
		else
			TTYENCODE=$'\033(K'
			TTYMESSAGE=ASCII
		fi

		# Enable multibyte input
		#
		boot_mesg "Enabling Multibyte input..." ${INFO}
		kbd_mode -u
		evaluate_retval

		# Setup Keymaps
		#
		if [ "${WINDOWS_KEYBOARD}" = "yes" ]; then
			WINDOWS_KEYMAP=windowkeys
		fi
		loadkeys -q ${WINDOW_KEYMAP} ${KEYMAP} ${EXTENDED_KEYMAPS}

		# Setting up the Terminals
		#
		boot_mesg "Setting encoding to ${TTYMESSAGE}.." ${INFO}
		for ttynum in $(seq 1 12) ; do
			echo -ne ${TTYENCODE} > /dev/tty${ttynum}
		done
		evaluate_retval

	;;

	*)
		echo "Usage: ${0} {start}"
		exit 1
	;;
esac

# End $rc_base/init.d/i18n
