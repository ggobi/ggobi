#!/bin/sh
##
# open-xapp - wrapper script for starting X11 applications
#             in contrast to open-x11 it supports command line arguments
##

# check command line
if [ $# = 0 -o `echo "$1" | cut -c 1` = '-' ]; then
    echo "Usage: open-xapp <xapp> [<arg1> ...]"
    exit 1
fi

#
# when only one argument, we could in principle use open-x11
# this method seems to ignore ~/.Xdefaults however
#
# if [ $# = 1 ]; then
#     open-x11 $@
#     exit 0
# fi

#
# for more arguments we must guess the correct value of DISPLAY
# and call the xapp with its command line arguments explicitly
#

# set focus on X11
# (otherwise new application window will not have focus)
osascript -e 'tell application "X11" to activate'

# try to figure out DISPLAY cleverly
DISPLAY=""
for x in 0 1 2 3 4 5 6 7 8 9
do
  if [ -O /tmp/.X$x-lock ]
      then
      #echo "X11 setup: $USER has an active lock on DISPLAY :$x.0"
      DISPLAY=:$x.0
      break
  fi
done
if [ -z "$DISPLAY" ]
    then
    echo "$USER has no X11 DISPLAY open" 1>&2
    exit 1
fi
export DISPLAY

# seperate xapp name and its arguments
xapp="$1"
shift

# start application
$xapp $@ &

