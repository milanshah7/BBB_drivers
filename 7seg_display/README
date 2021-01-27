/***************************************

* Name          : README
* Created on    : Wednesday 27 January 2021 04:29:04 PM 
* Last Modified : Wednesday 27 January 2021 06:34:20 PM
* Created By    : Milan Shah <milan.opensource@gmail.com> 
* Description   : This file gives general information about the connection
                  between the 7segment display and beaglebone black.

****************************************/

7 Segment Display Common Cathode
--------------------------------

In this type of display, 7 segment means 7 LEDs and Common Cathode
means cathode end of all LEDs are connected via common ground. The
Anode end of the LEDs are controlled by micro-controller.

=================
 Pin      LED
=================
 High     High
 Low      Low

7 Segment Display Common Anode
------------------------------

This type of display also has 7 LEDs. Anode end of all LEDs are connected
via common supply (+Vcc) and cathode end of all the LEDs are controlled by
micro-controller.

=================
 Pin      LED
=================
 High     Low
 Low      High

 Anode   : +ve (+Vcc)
 Cathode : -ve (GND)

There will be total 8 LEDs in 7-segment displays. 
  7 LEDs for segments
  1 LED for decimal point.

Total 10 pins:
  7 pins for 7 segments
  2 pins for common ground/supply
  1 pin for decimal point

Current Limiting Register will be used to limit the current passing to
LEDs. If high current is passed, then the LED will be burned.

Below is the equation to calculate register value in case its needed.

    V(s) - V(led)
R = -------------
         I(f)

where 
  V(s)   : Supply Voltage
  V(led) : LED forward voltage drop
  I(f)   : LED forward Current

The BBB GPIOs can only source 6mA to 8mA of current when in OUTPUT mode,
so if for example, the LED Max forward current is 20mA, then need not to
use any current limiting resistor.

===============================================================================
BBB_expansion_header_P8_pins      GPIO number           7Seg Display segment
===============================================================================
P8.7                               GPIO_66                    a
P8.8                               GPIO_67                    b
P8.9                               GPIO_69                    c
P8.10                              GPIO_68                    h //decimal point
P8.11                              GPIO_45                    d
P8.12                              GPIO_44                    e
P8.14                              GPIO_26                    f
P8.16                              GPIO_46                    g
P9.0                                                          Com // GND
P9.3                                                          Com // 3.3v
===============================================================================

