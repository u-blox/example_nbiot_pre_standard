# Example NB-IoT Pre-Standard

This repository contains a pair of simple server-side and client-side example programs that demonstrate end-to-end communications using a u-blox NB-IoT module.  The server-side is written in C# and compiles under Microsoft Visual C# Express.  The client-side is written in C++ and compiles under Microsoft Visual C++ Express or under GCC with a GNU make file.

On the server-side, `Program.cs` must be populated with the host name of your Huawei network server, your account on that server, the password for that account and the UUID of the NB-IoT device you wish to communicate with.  The compiled executable `server-side` can then be run from a Windows command prompt.  It will connect to your Huawei network server account and display any uplink datagrams received from the NB-IoT device. You may simultaneously enter datagrams as strings and send them on the downlink to the NB-IoT device.

On the client-side, if you are building with GCC, ensure that the environment variable `GCC_PREFIX` exists and is set to the location of the GCC executable.  For instance, if GCC is at `c:\gccforwin\bin\gcc.exe`, `GCC_PREFIX` would be set to `c:\gccforwin\bin\`.

The compiled `client_side.exe` should be invoked at the Windows command prompt with the COM port that the NB-IoT device is connected to as a parameter, e.g:

`client_side COM1`

The NB-IoT device may be a physical device or it may be SoftRadio.  In the case where SoftRadio is used, you should download a Windows virtual COM port application such as com0com:

http://com0com.sourceforge.net/

Simply install and run com0com and it will create pairs of virtual COM ports (the default settings are fine).  When you start SoftRadio, connect it to one of these virtual COM ports instead of a real COM port.  As there are some subtle differences between the way SoftRadio operates and the way a real NB-IoT module operates, to run the client with SoftRadio you must invoke it at the Windows command prompt with the parameter `-s`, for example as follows:

`client-side -s COM1`

If you are using a real NB-IoT module, invoke it at the Windows command prompt without the `-s` parameter.

The client-side will connect to the module (or SoftRadio), check that it is registered with the network, send an initial "Hello World" string on the uplink and then send whatever you type at the command prompt as an uplink datagram.  After that it will check for downlink datagrams before prompting you once more for an uplink datagram.  Press `CTRL-C` to exit.
