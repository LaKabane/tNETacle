#!/bin/sh
dscl . -create /Users/_tnetacle
dscl . -create /Users/_tnetacle UserShell /dev/null
dscl . -create /Users/_tnetacle RealName "tnetacle user"
#dscl . -create /Users/_tnetacle UniqueID "1010"
dscl . -create /Users/_tnetacle PrimaryGroupID 0
dscl . -create /Users/_tnetacle NFSHomeDirectory /var/empty
#dscl . -delete /Users/_tnetacle
