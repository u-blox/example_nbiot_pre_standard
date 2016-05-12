// NB-IoT modem driver for NB-IoT example application

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "utilities.h"
#include "serial_driver.h"
#include "modem_driver.h"

// ----------------------------------------------------------------
// COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// At the end of all AT strings there is a...
#define AT_TERMINATOR "\r\n"

// Wait between polling the NB-IoT module AT interface
#define AT_RX_POLL_TIMER_MS 100

// OK
#define AT_OK "OK\r\n"

// ERROR
#define AT_ERROR "ERROR\r\n"

// ----------------------------------------------------------------
// PROTECTED FUNCTIONS
// ----------------------------------------------------------------

// Send a string, printf()-wise, to the NB-IoT module
uint32_t Nbiot::sendPrintf(const char * pFormat, ...)
{
    va_list args;
    bool success = false;
    uint32_t len = 0;

    if (gInitialised)
    {
        va_start(args, pFormat);
        len = vsnprintf(gTxBuf, sizeof(gTxBuf), pFormat, args);
        va_end(args);

        printf("Sending to module %s", gTxBuf);
        success = gpSerialPort->transmitBuffer((const char *) gTxBuf, len);
    }

    return success;
}

// Get characters (up to lenBuf of them) from the NB-IoT module into pBuf.
// If an AT terminator is found, or lenBuf characters have been read,
// return a count of the number of characters (including the AT terminator),
// otherwise return 0.
uint32_t Nbiot::getLine(char * pBuf, uint32_t lenBuf)
{
    int32_t x;
    uint32_t returnLen = 0;

    if (gInitialised)
    {
        if (gLenRx < lenBuf)
        {
            do
            {
                x = gpSerialPort->receiveChar();

                // If something was received, add it to the buffer
                // and check if a terminator has landed
                if (x >= 0)
                {
                    *(pBuf + gLenRx) = (char) x;
                    gLenRx++;
                    
                    if (x == AT_TERMINATOR[gMatched])
                    {
                        gMatched++;
                    }
                    else
                    {
                        gMatched = 0;
                    }
                }
            } while ((x >= 0) && (gLenRx < lenBuf) && (gMatched < sizeof(AT_TERMINATOR) - 1));        
        }
        
        if ((gMatched == sizeof(AT_TERMINATOR) - 1) || (gLenRx == lenBuf)) // -1 to omit 0 of string
        {
            // A terminator has been found, or we've hit a buffer limit, so let the caller know to
            // handle the line and reset the tracking variables for next time
            returnLen = gLenRx;
            gLenRx = 0;
            gMatched = 0;
        }    
    }
    
    return returnLen;
}

// Callback to handle AT stuff received from the NBIoT module
void Nbiot::rxTick()
{
    uint32_t len = getLine (gRxBuf, sizeof (gRxBuf));

    if (len > sizeof(AT_TERMINATOR) - 1) // -1 to omit NULL terminator
    {
        printf ("RxTick received %d characters from module: \"%.*s\".\r\n", (int) (len - (sizeof(AT_TERMINATOR) - 1)), (int) (len - (sizeof(AT_TERMINATOR) - 1)), gRxBuf);
        if (gpResponse == NULL)
        {
           gLenResponse = len;
           gpResponse = gRxBuf;
        }
    }
}

// Wait for an AT response.  If pExpected is not NULL and the
// AT response string begins with this string then say so, else
// wait for the standard "OK" or "ERROR" responses for a little
// while, else time out. The response string is copied into
// gpResponse if it is non-NULL (and a null terminator is added).
Nbiot::AtResponse Nbiot::waitResponse(const char * pExpected, time_t timeoutSeconds, char * pResponseBuf, uint32_t responseBufLen)
{
    AtResponse response = AT_RESPONSE_NONE;
    time_t startTime = time(NULL);

    if (gpResponse != NULL)
    {
        printf ("ERROR: response from module not cleared from last time.\n");
        gpResponse = NULL;
    }

    do {
        rxTick();

        if (gpResponse != NULL)
        {
            // Got a line, process it
            if ((strncmp(gpResponse, AT_OK, gLenResponse) == 0) && (gLenResponse == (sizeof (AT_OK) - 1))) // -1 to omit 0 of string
            {
                response = AT_RESPONSE_OK;
            }
            else if ((strncmp(gpResponse, AT_ERROR, gLenResponse) == 0) && (gLenResponse == (sizeof (AT_ERROR) - 1))) // -1 to omit 0 of string
            {
                response = AT_RESPONSE_ERROR;
            }
            else if ((pExpected != NULL) && (gLenResponse >= strlen (pExpected)) && (strcmp(gpResponse, pExpected) >= 0))
            {
                response = AT_RESPONSE_STARTS_AS_EXPECTED;
                if (pResponseBuf != NULL)
                {
                    // Copy the response string into pResponseBuf, with a terminator
                    if (gLenResponse > responseBufLen - 1)
                    {
                        gLenResponse = responseBufLen - 1;
                    }
                    memcpy (pResponseBuf, gpResponse, gLenResponse);
                    pResponseBuf[gLenResponse] = 0;
                }
            }
            else
            {
                if (pExpected != NULL)
                {
                    printf ("WARNING: unexpected response from module.\n");
                    printf ("Expected: %s... Received: %.*s\r\n", pExpected, (int) gLenResponse, gpResponse);
                }
                // Reset response pointer
                gpResponse = NULL;
            }
        }

        if (gpResponse == NULL)
        {
            Sleep(AT_RX_POLL_TIMER_MS);
        }

    } while ((response == AT_RESPONSE_NONE) && ((timeoutSeconds == 0) || (startTime + timeoutSeconds > time(NULL))));

    // Reset response pointer for next time
    gpResponse = NULL;

    return response;
}

static void charToTchar(const char *pIn, TCHAR *pOut, uint32_t size)
{
    memset (pOut, 0, size);
#ifdef _MSC_VER
    uint32_t size_needed = MultiByteToWideChar(CP_UTF8, 0, pIn, (int)strlen(pIn), NULL, 0);
    if (size_needed > size)
    {
        size_needed = size;
    }
    MultiByteToWideChar(CP_UTF8, 0, pIn, (int)strlen(pIn), pOut, size_needed);
#else
    strncpy(pOut, pIn, size);
#endif
}

// ----------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------


// Constructor
Nbiot::Nbiot(const char * pPortname)
{
    gpResponse   = NULL;
    gpSerialPort = NULL;
    gLenResponse = 0;
    gMatched = 0;
    gLenRx = 0;
    gInitialised = false;
    gpSerialPort = new SerialPort();
    TCHAR tcharPortname[MAX_PATH];

    charToTchar(pPortname, tcharPortname, sizeof (tcharPortname));

    if (gpSerialPort)
    {
        if (gpSerialPort->connect(tcharPortname))
        {
            printf ("Connected to port %s.\n", pPortname);
            gInitialised = true;
            // Flush out any initialisation messages from the modem
            waitResponse(NULL, DEFAULT_FLUSH_TIMEOUT_SECONDS);
        }
        else
        {
            delete gpSerialPort;
            printf ("Unable to connect to port %s.\n", pPortname);
        }
    }
}

// Connect to the network
bool Nbiot::connect(bool usingSoftRadio, time_t timeoutSeconds)
{
    bool success = false;
    AtResponse response;
    time_t startTime = time(NULL);

    if (gInitialised)
    {
        if (timeoutSeconds > 0)
        {
            printf ("Checking for connection to network for up to %d seconds...\r\n", (int) timeoutSeconds);
        }
        else
        {
            printf ("Checking for connection to network...\r\n");
        }

        do {

            if (usingSoftRadio)
            {
                // Check for service at radio level (as SoftRadio
                // does not support AT+NAS)
                sendPrintf("AT+RAS%s", AT_TERMINATOR);
                response = waitResponse("+RAS:CONNECTED\r\n");
            }
            else
            {
                // First check for service using +NAS.
                sendPrintf("AT+NAS%s", AT_TERMINATOR);
                response = waitResponse("+NAS: Connected (activated)\r\n");
            }

            if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
            {
                // It worked, but need to also wait for the "OK"
                waitResponse();

                printf ("Connected to network, setting AT+SMI to 1.\r\n");

                // Set AT+SMI to be 1
                sendPrintf("AT+SMI=1%s", AT_TERMINATOR);
                response = waitResponse("+SMI:OK\r\n");
                if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
                {
                    // Absorb the trailing OK.
                    waitResponse();

                    // All done
                    success = true;
                    printf ("AT+SMI set to 1.\r\n");
                }

                // Here we could set up AT+NMI to be 2 and receive
                // datagrams asynchronously based on +NMI notifications
                // from the module but this simple example is kept 
                // deliberately kept single-threaded and so it instead
                // polls the modem with AT+MGR for downlink messages between
                // uplink message transmissions.
            }
            else
            {
                // Didn't work, wait before re-trying
                Sleep((timeoutSeconds * 1000) / 10);
            }
        } while ((!success) && ((timeoutSeconds == 0) || (startTime + timeoutSeconds > time(NULL))));
    }
    
    return success;
}

// Send a message to the network
bool Nbiot::send (char * pMsg, uint32_t msgSize, time_t timeoutSeconds)
{
    bool success = false;
    AtResponse response;
    uint32_t charCount = 0;

    // Check that the incoming message, when hex coded (so * 2) is not too big
    if ((msgSize * 2) <= sizeof(gHexBuf))
    {
        charCount = bytesToHexString (pMsg, msgSize, gHexBuf, sizeof(gHexBuf));
        printf("Sending datagram to network, %d characters: %.*s\r\n", msgSize, (int) msgSize, pMsg);
        sendPrintf("AT+MGS=%d, %.*s%s", msgSize, charCount, gHexBuf, AT_TERMINATOR);

        // Wait for confirmation
        response = waitResponse("+MGS:OK\r\n");

        if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
        {
            // It worked, wait for the "OK"
            waitResponse();

            // Now wait for the SENT indication
            response = waitResponse("+SMI:SENT\r\n", timeoutSeconds);

            if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
            {
                // All done
                success = true;
                printf ("Modem reports datagram SENT.\r\n");
            }
        }
    }
    else
    {
        printf ("!!! Datagram is too long (%d characters when only %d bytes can be sent).\r\n", msgSize, (int) (sizeof (gHexBuf) / 2));
    }

    return success;
}

// Receive a message from the network
uint32_t Nbiot::receive (char * pMsg, uint32_t msgSize, time_t timeoutSeconds)
{
    int bytesReceived = 0;
    AtResponse response;
    char * pHexStart = NULL;
    char * pHexEnd = NULL;

    printf("Receiving a datagram of up to %d byte(s) from the network...\r\n", msgSize);
    sendPrintf("AT+MGR%s", AT_TERMINATOR);

    response = waitResponse("+MGR:", timeoutSeconds, gHexBuf, sizeof (gHexBuf));

    if (response == AT_RESPONSE_STARTS_AS_EXPECTED)
    {
        if (sscanf(gHexBuf, " +MGR:%d,", &bytesReceived) == 1) // The space in the string is significant,
                                                               // it allows any whitespace characters in the input string
        {
            pHexStart = strchr (gHexBuf, ',');
            if (pHexStart != NULL)
            {
                pHexStart++;
                pHexEnd = strstr (pHexStart, AT_TERMINATOR);
                if ((pHexEnd != NULL) && (pMsg != NULL))
                {
                    hexStringToBytes (pHexStart, pHexEnd - pHexStart, pMsg, msgSize);
                }
            }

            // Wait for the OK at the end
            response = waitResponse("+MGR:OK\r\n");
        }
    }

    return (uint32_t) bytesReceived;
}

// End Of File
