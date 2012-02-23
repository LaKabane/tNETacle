#!/bin/sh

case $1 in
	"first")
		pid=`ps h -C tNETacle | cut -d ' ' -f 1 | head -n 1` 
		tail -f --pid=$pid output.$pid
		;;
	"second")
		pid=`ps h -C tNETacle | cut -d ' ' -f 1 | tail -n 1`
		tail -f --pid=$pid output.$pid
		;;
	*)
		tmux new-session -d "strace -ff -o output ./bin/tNETacle" \; \
			split-window -d "./run.sh first" \; \
			split-window -d "./run.sh second" \; \
			select-layout main-vertical \; \
				attach
		rm output.*
esac
