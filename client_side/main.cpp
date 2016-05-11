// This is a client-side example program for use with the u-blox NB-IoT
// modules.  It connects with the NB-IoT module (or SoftRadio) via a
// COM port, sends an initial uplink datagram, then sends uplink datagrams
// based on user input and polls for downlink datagrams from the
// network.
// It should be used in conjunction with the server-side example code.

#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "windows.h"
#include "utilities.h"
#include "serial_driver.h"
#include "modem_driver.h"

// ----------------------------------------------------------------
// COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

#define DIR_SEPARATORS "\\/"
#define EXT_SEPARATOR "."

// ----------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------

// Main accepts two command-line arguments:
//
// -s: if this is present then it is assumed that SoftRadio is
// in use, otherwise a real NB-IoT module is assumed.
//
// string: specifies the port name to use, e.g. COM8
//
// The parameters may be provided in any order
int main(int argc, char* argv[])
{
    bool success = true;
    bool usingSoftRadio = false;
    bool gotPortString = false;
    char portString[8];
    char winPortString[16] = "\\\\.\\";   // Windows format for port management
    char datagram[256] = "Hello World!";
    Nbiot * pModem = NULL;
    uint32_t datagramLen = strlen (datagram);
    char * pUserInput;
    char * pChar;
    char * pExeName;

    // Find the exe name in the first argument
    pChar = strtok (argv[0], DIR_SEPARATORS);
    while (pChar != NULL)
    {
        pExeName = pChar;
        pChar = strtok (NULL, DIR_SEPARATORS);
    }
    if (pExeName != NULL)
    {
        // Remove the extension
        pChar = strtok (pExeName, EXT_SEPARATOR);
        if (pChar != NULL)
        {
            pExeName = pChar;
        }
    }

    // Check the command line parameters
    for (int32_t x = 1; success && (x < argc); x++)
    {
        if (!usingSoftRadio && (strcmp (argv[x], "-s") == 0))
        {
            usingSoftRadio = true;
        }
        else if (!gotPortString)
        {
            gotPortString = true;
            strncpy(portString, argv[x], sizeof (portString));
            strcat(winPortString, argv[x]);
        }
        else
        {
            printf("Unknown command-line parameter '%s'.\n", argv[x]);
            success = false;
        }
    }    
    
    if (success && gotPortString)
    {
        // Initialise the module and register with the network
        pModem = new Nbiot(winPortString);
        
        if (pModem)
        {
            printf ("Initialising module...\n");
            success = pModem->connect(usingSoftRadio);

            if (success)
            {
                // Send the initial "hello" that is in the buffer at start of day
                printf ("Sending initial datagram \"%*s\".\n", datagramLen, datagram);
                success = pModem->send (datagram, datagramLen);

                if (success)
                {
                    while (success)
                    {
                        // Get user input
                        printf ("Type in a datagram to send to the network and press <enter>, or just press <enter> to check the downlink.\n");
                        printf ("> ");
                        pUserInput = fgets (datagram, sizeof (datagram), stdin);                    
                        if (pUserInput && (strlen(datagram) > 1))
                        {
                            // If there was user input, send it on the uplink,
                            // omitting the newline character from the end
                            success = pModem->send (datagram, strlen(datagram) - 1);
                            if (!success)
                            {
                                printf ("Failed to send uplink datagram.\n");
                            }
                        }
                        
                        // Check for any downlink data
                        // Set datagramLen to the maximum size we can receive
                        datagramLen = sizeof (datagram);
                        datagramLen = pModem->receive (datagram, datagramLen);
                        
                        if (datagramLen > 0)
                        {
                            printf ("Datagam received from network: \"%.*s\".\n", datagramLen, datagram);
                        }
                    }
                }
                else
                {
                    printf ("Failed to send initial datagram to the network.\n");
                }
            }
            else
            {
                printf ("Failed to connect to the network.\n");
            }
        }
        else
        {
            printf ("Unable to connect serial port '%s'.\n", portString);
        }
    }
    else
    {
        printf("Usage:\n");
        printf("%s [-s] <port>\n", pExeName);
        printf("...where -s is used to indicate that Soft Radio is being used and <port> is\n");
        printf("the serial port where the AT interface of the NBIoT modem can be found.\n");
        printf("For example: %s -s COM1\n\n", pExeName);
    }
}
