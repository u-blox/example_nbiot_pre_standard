# Example NB-IoT Pre-Standard

This repository contains a pair of simple server-side and client-side example programs that demostrate end-to-end communications using a u-blox NB-IoT module.  The server-side is written in C# and compiles under Microsoft Visual Studio.  The client-side is written in C++ and compiles under GCC with a GNU make file; this way the client-side may easily be ported to other targets if desired.

On the server-side, `Program.cs` must be populated with the host name of your Huawei network server, your account on that server, the password for that account and the UUID of the NB-IoT device you wish to communicate with.  The compiled executable `server-side` can then be run from a Windows command prompt.  It will connect to your Huawei network server account and display any uplink datagrams received from the NB-IoT device. You may simultaneously enter datagrams as strings and send them on the downlink to the NB-IoT device.

On the client-side, `main.c` should be populated with the COM port that the NB-IoT device is connected to.  This may be a physical device or it may be SoftRadio.  In the case where SoftRadio is used, you should download a Windows virtual COM port application such as com0com:

http://com0com.sourceforge.net/

Simply install and run com0com and it will create pairs of virtual COM ports (the default settings are fine).  When you start SoftRadio, connect it to one of these virtual COM ports instead of a real COM port.  Populate `main.c` with the name of other COM port of the pair.  Then build the client-side using the supplied GNU make file and any version of GCC for Windows.  As there are some subtle differences between the way SoftRadio operates and the way a real NB-IoT module operates, to run the client with SoftRadio you must invoke it at the Windows command prompt with the parameter `-s` as follows:

`client-side -s`

If you are using a real NB-IoT module, invoke it at the Windows command prompt without the `-s` parameter.

The client-side will connect to the module (or SoftRadio), check that it is registered with the network, send an initial "Hello World" string on the uplink and then send whatever you type at the command prompt as an uplink datagram.  After that it will check for downlink datagrams before prompting you once more for an uplink datagram.  Press `CTRL-C` to exit.
