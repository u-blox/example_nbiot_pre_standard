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
// MAIN
// ----------------------------------------------------------------

// Main accepts a single command-line switch, "-s".  If this is
// present then it is assumed that SoftRadio is in use, otherwise a
// real NB-IoT module is assumed.
int main(int argc, char* argv[])
{
    bool success = false;
    bool usingSoftRadio = false;
    char datagram[256] = "Hello World!";
    uint32_t datagramLen = strlen (datagram);
    Nbiot * pModem = new Nbiot("\\\\.\\COM107");
    char * pUserInput;

    // Check the command line parameters for the SoftRadio switch
    if (argc == 2)
    {
        if (strcmp (argv[1], "-s") == 0)
        {
            usingSoftRadio = true;
        }
    }

    // Initialise the module and register with the network
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
}
