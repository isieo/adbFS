Instructions
============

Compilation
-----------

You will need at least the following on Debian:

- android-tools-adb
- libfuse-dev
- pkg-config
- cmake

Run `make` to build

Install
-------

Put the compiled binary into your ``<path-to-android-sdk>/tools``
directory.

Use
---

::

    $ adbfs <Mountpoint>
