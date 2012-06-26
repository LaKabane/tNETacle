#!/bin/sh

# Little helper for the developers

name="_tnetacle"
real_name="tNETacle"
home="/var/empty"
shell="/sbin/nologin"

[ -d /var/empty ] || mkdir /var/empty
chown root /var/empty
chmod 700 /var/empty

if [ "`uname`" = "Darwin" ]; then
	dscl . -create /Users/$name
	dscl . -create /Users/$name UserShell $shell
	dscl . -create /Users/$name RealName $real_name
	dscl . -create /Users/$name PrimaryGroupID 0
	dscl . -create /Users/$name NFSHomeDirectory $home
else
	useradd -s $shell -d $home -c $real_name $name
fi
