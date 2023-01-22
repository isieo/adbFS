#!/bin/bash

echo Running supervisord in the background
/usr/bin/supervisord --configuration supervisord.conf &


echo Checking readiness via adb shell ls -d /

RETRIES=30

while [ $RETRIES -gt 0 ];
do
  echo hi $RETRIES
  RETRIES=$((RETRIES - 1))

  OUTPUT=$(adb shell ls -d /)

  echo "Output $OUTPUT"
  echo "adb devices"
  adb devices
  sleep 3
done