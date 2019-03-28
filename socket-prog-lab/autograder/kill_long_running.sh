#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <procname> <time-in-sec>"
    exit 1
fi

PROCNAME=$1
MAXTIME=$2
TMPFILE="/tmp/kill_long_running.tmp.$$"

count=0
while true
do
    sleep 1
    rm -f "$TMPFILE"
    let count=count+1
    echo "Checking for processes with name $PROCNAME, count=$count, TMPFILE=$TMPFILE"
    ps -ewwo pid,etimes,start,cmd | grep "$PROCNAME" > "$TMPFILE"
    while read line
    do
	etimes=$(echo $line | awk '{print $2}')
	cmd=$(echo $line | awk '{print $4}')
	if [ "$cmd" != "$PROCNAME" ]; then
	    continue
	fi
	echo "etimes=$etimes MAXTIME=$MAXTIME"
	if [ "$etimes" -gt "$MAXTIME" ]; then
	    pid=$(echo $line | awk '{print $1}')
	    echo "$line"
	    echo "$pid running for $etimes sec, killing it"
	    kill -TERM $pid
	fi
    done < "$TMPFILE"
done
