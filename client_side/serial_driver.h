// Serial port driver for NB-IoT example application

#ifndef _SERIAL_DRIVER_H_
#define _SERIAL_DRIVER_H_

// ----------------------------------------------------------------
// CLASSES
// ----------------------------------------------------------------

// Serial port interface
class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    // Make a connection to a named port.  On Windows the form of a
    // properly escaped string must be as follows:
    //
    // "\\\\.\\COMx"
    //
    // ...where x is replaced by the COM port number.  So, for COM17, the string
    // would be:
    //
    // "\\\\.\\COM17"
    //
    // Returns TRUE on success, otherwise FALSE.
    bool connect(const TCHAR * pPortName);
    
    // Disconnect from the current serial port.
    void disconnect(void);

    // Transmit lenBuf characters from pBuf over the serial port.
    // Returns TRUE on success, otherwise FALSE.
    bool transmitBuffer(const char * pBuf, uint32_t lenBuf);
    
    // Receive up to lenBuf characters into pBuf over the serial port.
    // Returns the number of characters received.
    uint32_t receiveBuffer(char * pBuf, uint32_t lenBuf);
    
    // Receive a single character from the serial port.
    // Returns -1 if there are no characters, otherwise it
    // returns the character (i.e. it can be cast to char).
    int32_t receiveChar();
    
    // Clear the serial port buffers, both transmit and receive.
    void clear();

protected:
    // The serial port handle, set to INVALID_HANDLE_VALUE if
    // not configured.
    HANDLE gSerialPortHandle;
};

#endif

// End Of File
