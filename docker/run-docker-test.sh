#!/bin/bash

echo Running supervisord in the background
# cd /root || exit
/usr/bin/supervisord --configuration supervisord.conf &

cd /src || exit



wait_available() {

  RETRIES=60
  WAIT_DIR="$1"

  while [ $RETRIES -gt 0 ];
  do
    RETRIES=$((RETRIES - 1))

    OUTPUT=$(adb shell ls -d "$WAIT_DIR" 2> /dev/null)

    if [ "$OUTPUT" == "$WAIT_DIR" ]
    then
      break;
    fi
    sleep 1

  done

  if [ $RETRIES -le 0 ]
  then
    echo "Emulator directory $1 was not available, exiting"
    exit 1
  fi

}

echo Checking readiness via adb shell ls -d /sdcard/Android
wait_available /sdcard/Android

mkdir -p /adbfs
./adbfs /adbfs

echo Ready to run adbfs tests

BASE_DIR=/adbfs/sdcard/test

test_mkdir() {

  mkdir "$BASE_DIR/x"
  output=$(ls -lad "$BASE_DIR/x")
  # TODO: verify output is correct

}



mkdir "$BASE_DIR"

test_mkdir




# todo, test mkdir

# create file with cat

# copy file


# touch to update time

# copy preserving timestamps

rm -rf "$BASE_DIR"


