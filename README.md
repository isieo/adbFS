This variant of adbfs workous even WITHOUT having root access (busybox) on your phone!

Instructions:
=============


- Clone the repository 

    git clone git://github.com/spion/adbfs-rootless.git
    
- You will need libfuse-dev. On ubuntu
    
    sudo apt-get install libfuse-dev

- Build

    make

- Copy the binary adbfs to <path-to-android-sdk>/platform-tools directory. 
If platform-tools is in your $PATH you can skip this step.

- Create a mount point if needed (e.g. in your home directory)

    mkdir ~/droid
    
-  You can now mount your device from the platform-tools dir:

    ./adbfs ~/droid



