// Serial port driver for NB-IoT example application

#include "stdint.h"
#include "stdio.h"
#include "windows.h"
#include "serial_driver.h"

// ----------------------------------------------------------------
// CLASSES/METHODS
// ----------------------------------------------------------------

// Constructor.
SerialPort::SerialPort()
{
    gSerialPortHandle = INVALID_HANDLE_VALUE;
}
 
// Destructor.
SerialPort::~SerialPort()
{
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(gSerialPortHandle);
    }
    
    gSerialPortHandle = INVALID_HANDLE_VALUE;
}
 
// Make a connection to a named port.
bool SerialPort::connect(const TCHAR * pPortName)
{
    bool success = false;
    DCB dcb;
    COMMTIMEOUTS timeouts;

    memset(&dcb, 0, sizeof(dcb));

    dcb.DCBlength = sizeof(dcb);

    dcb.BaudRate = 57600;
    dcb.Parity = NOPARITY;
    dcb.fParity = 0;
    dcb.StopBits = ONESTOPBIT;
    dcb.ByteSize = 8;
    
    memset(&timeouts, 0, sizeof(timeouts));

     // read() times out immediately, write() never times out
    timeouts.ReadIntervalTimeout = MAXDWORD;

    // Get a handle on the serial port
    gSerialPortHandle = CreateFile(pPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
 
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        // Set the comms port parameters and the timeouts
        if (SetCommState(gSerialPortHandle, &dcb) && SetCommTimeouts(gSerialPortHandle, &timeouts))
        {
            success = true;
        }
    }
    else
    {
        uint32_t err = ::GetLastError();
#ifdef _MSC_VER
        wchar_t * pMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &pMsgBuf, 0, NULL);
        printf("Error connecting to serial port %s %s.\n", pPortName, pMsgBuf);
#else
        uint32_t err = ::GetLastError();
        printf("Error %d connecting to serial port %s.\n", err, pPortName);
#endif
    }

    if (!success)
    {
        disconnect();
    }
    else
    {        
        clear();
    }

    return success;
}

// Disconnect from the port.
void SerialPort::disconnect(void)
{
    CloseHandle(gSerialPortHandle);
    gSerialPortHandle = INVALID_HANDLE_VALUE;
}

// Send lenBuf bytes from pBuf over the serial port, returning true
// in the case of success.
bool SerialPort::transmitBuffer(const char *pBuf, uint32_t lenBuf)
{
    unsigned long result = 0;

    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        WriteFile(gSerialPortHandle, pBuf, lenBuf, &result, NULL);
        if (!result)
        {
            printf ("Transmit failed with error code %ld.\n", GetLastError());
        }
    }

    return (bool) result;
}

// Get up to lenBuf bytes into pBuf from the serial port,
// returning the number of characters actually read.
uint32_t SerialPort::receiveBuffer (char *pBuf, uint32_t lenBuf)
{
    unsigned long result = 0;
    unsigned long readLength;

    readLength = 0;
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        result = ReadFile(gSerialPortHandle, pBuf, lenBuf, &readLength, NULL);
        if (!result)
        {
            printf ("Receive failed with error code %ld.\n", GetLastError());
        }
    }

    return (uint32_t) readLength;
}

// Read a single character from the serial port, returning
// -1 if no character is read.
int32_t SerialPort::receiveChar()
{
    char readChar = 0;
    int32_t returnChar = -1;
    unsigned long result = 0;
    unsigned long readLength;

    readLength = 0;
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        result = ReadFile(gSerialPortHandle, &readChar, sizeof (readChar), &readLength, NULL);
        if (result)
        {
            if (readLength > 0)
            {
                returnChar = (int32_t) readChar;
            }
        }
        else
        {
            printf ("Receive failed with error code %ld.\n", GetLastError());
        }
    }

    return returnChar;
}

// Clear the receive and transmit buffers of the serial port
void SerialPort::clear()
{
    PurgeComm (gSerialPortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

// End Of File
