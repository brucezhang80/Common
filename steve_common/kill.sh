#!/bin/sh

if [ $# -ne 2 ]
then
echo "./kill.sh -sigspec NAME"
else
for pid in `ps aux|grep -v "grep"|grep -v "kill.sh"|grep "$2"|awk '{print $2}'`
do
kill $1 $pid
done
fi
