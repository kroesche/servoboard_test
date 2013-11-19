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
This is a *quick overview* of the procedure.  You should review the full test procedure document found here **TBD**.

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
You can find a pre-built binary of this application in the **gcc** directory.
