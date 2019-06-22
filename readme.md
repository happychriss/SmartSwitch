Double Smart Switch
===================

Super cheap home atomization with an ES8266 and a PIC8 processor.

ESP8266
-------
Wifi Communication to OpenHabian
Deep Sleep, on wake up reading the 2 switches and sending to OpenHabian

PIC8
----
Has the nice feature to define each input channel as "wake up on state change". When any of the switches at the PIC 
change status, it will wake up the ESP8266.
Git for PIC8: https://github.com/happychriss/SmartSwitch_PIC8


More information:
https://44-2.de/double-smart/


!!!!!! This Software contains hardcoded IP addresses and some work could be done on the ESP8266 for Wifi configuration :-) !!!!