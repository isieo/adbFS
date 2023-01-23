#!/bin/bash

echo Running supervisord in the background
# cd /root || exit
/usr/bin/supervisord --configuration supervisord.conf &

cd /src || exit


echo Checking readiness via adb shell ls -d /

RETRIES=60

while [ $RETRIES -gt 0 ];
do
  RETRIES=$((RETRIES - 1))

  OUTPUT=$(adb shell ls -d /)

  if [ "$OUTPUT" == "/" ]
  then
    break;
  fi
  sleep 1
  # adb connect 127.0.0.1:5555
done

if [ $RETRIES -le 0 ]
then
  echo "Could not connect to emulator, exiting"
  exit 1
fi

echo Ready to run adbfs

mkdir -p /adbfs
./adbfs /adbfs

ls -la /adbfs