Summary
=======

Test Program for the 12-server BoosterPack
==========================================
by Joe Kroesche (kroesche.org)

---

License
-------
Copyright &copy; 2013, Joseph Kroesche (kroesche.org).  All rights reserved.
This software is released under the FreeBSD license, found in the accompanying file LICENSE.txt and at the following URL:
  http://www.freebsd.org/copyright/freebsd-license.html
This software is provided as-is and without warranty.

**The author and this software is not affiliated with Texas Instruments.  This is not a product of Texas Instruments, and TI is not responsible for this software.  Please do not ask TI for support with this software.**

Overview
========
This test application, called servoboard_test, is meant to be used for testing the [12-servo BoosterPack](http://tronics.kroesche.org/servo-boosterpack.html).  See also https://github.com/kroesche/lp-servo12.  It is meant to serve as a kind of production test of the servo board.

Test Procedure
==============
This is a *quick overview* of the procedure.  You should review the full test procedure document found in the [doc](https://github.com/kroesche/servoboard_test/blob/master/doc) directory.

0. This procedure assumes you have already loaded the test program onto the LaunchPad.
1. With the 12-servo BoosterPack not plugged into the LaunchPad, apply 6V power supply to the power connector.
2. Use the 5V connector J10 to verify the on-board power supply is 5V.
3. Vary the input power supply from 6-12V.  Verify the on-board power supply remains at 5V.
4. Disconnect the external power supply.
5. Attach a Stellaris/Tiva LaunchPad to the 12-servo BoosterPack.  Be sure to review the documentation for the BoosterPack for necessary modifications to the LaunchPad.
6. Make sure the power switch is in the "Device" position.
7. Apply the external power supply with a value from 6-12V.
8. Verify the green power LED is on.
9. Adjust the external power supply to 8V.  When it is at 8V the RGB LED should be green.  It will be blue when the power supply is less than 8V and red when the power supply is greater than 8V.  When it is green at 8V it means the analog sensing circuit is working correctly.
10. Use a hobby servo to briefly plug onto each of the 12 servo connectors.  Verify the servo moves back and forth.  If you don't have a hobby servo, you can also use an oscilloscope to observe the servo control signal on each servo connector.  You will see a varying pulse width from about 1-2 ms.
11. Disconnect the power supply.
12. Done.

Pre-built Binary
================
You can find a pre-built binary of this application in the [gcc](https://github.com/kroesche/servoboard_test/tree/master/gcc) directory.

Building
========

Directory Layout
----------------
In order to set up a project to build this project, I have to assume a directory layout.  You can change this around however you like, but this is how the current Makefile and project files assume the layout:

* DEV_ROOT (some directory you use for development)
** servoboard_test (this repository)
** stellaris_drivers (dependent repository containing servo driver)

The stellaris_drivers repository contains the servo-wt driver (.c and .h) that are needed for this project.  You can lay out both repositories as shown above, or you can just take the servo-wt driver source files and add them to this project.

This project also assumes that TivaWare is present somewhere.  You use a build variable named TIVAWARE_ROOT to point to the location.  You can also use StellarisWare instead of TivaWare but you will need to change some other variables ... see the comments in the Makefile for details.

Makefile
--------
You can use the Makefile if you prefer command line builds.  This works on Mac or Linux and possibly windows if you install cygwin.  You must have the GCC arm-none-eabi- toolchain installed somewhere.

**Build Variables** (in Makefile)

* TIVAWARE_ROOT (where you installed TivaWare)
* USE_STELLARISWARE (if you want to use StellarisWare instead of TivaWare - see comments)
* TOOLPATH (where you installed the ARM compiler)

Code Composer Studio
--------------------
I also created a CCS project.  I only tested this on Windows XP.  The project is located in the directory named “ccs”.  You should import this project into your workspace.  You will need to tell it where to find TivaWare by setting the location using TIVAWARE_ROOT.  You can set this from the project Properties.  Go to Resource—>Linked Resources, and edit the Path Variable named TIVAWARE_ROOT.
