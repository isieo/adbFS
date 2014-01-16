This variant of adbfs workous even WITHOUT having root access (busybox) on your phone!

Instructions:
=============

You will need `libfuse-dev` and `adb`. On ubuntu
    
    sudo apt-get install libfuse-dev android-tools-adb

Clone the repository 

    git clone git://github.com/spion/adbfs-rootless.git
    cd adbfs-rootless    

Build

    make

Optional: If you have a separate copy of android-sdk and would
like to use that adb, copy the binary adbfs to the `android-sdk/platform-tools`
directory. If platform-tools is in your $PATH you can skip this step.

Create a mount point if needed (e.g. in your home directory)

    mkdir ~/droid

You can now mount your device (also from the platform-tools dir):

    ./adbfs ~/droid

Have fun!

## troubleshooting

#### Error: device not found

When running you get the following error:

```
--*-- exec_command: adb shell ls
error: device not found
```

Solution: make sure that [USB Debugging is enabled][enable-usb-debug]

Then `sudo umount /media/mount/path` before trying again

#### Error: device offline

When running you get the following error:

```
--*-- exec_command: adb shell ls
error: device offline
```

Solution: make sure that 

1. your android-sdk-tools are up to date. Newer versions
   of Android also require newer versions of adb. For more info, see 
   [this stackoverflow post][error-device-offline]

2. you answer `Yes `when your phone asks whether it should allow the 
   computer with the specified RSA key to access the device.

Then `killall -9 adb; sudo umount /media/mount/path` before trying again


[enable-usb-debug]: http://www.droidviews.com/how-to-enable-developer-optionsusb-debugging-mode-on-devices-with-android-4-2-jelly-bean/
[error-device-offline]: http://stackoverflow.com/questions/10680417/error-device-offline
