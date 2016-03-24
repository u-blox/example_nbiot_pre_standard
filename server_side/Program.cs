using System;
using System.Text;
using Neul.ServiceProvider;

namespace example
{
    class Program
    {
        // This is an example C# console application that talks to an NB-Iot
        // module.  It should be used in conjunction with the client-side
        // example program.
        //
        // You must fill in the URL of Huawei network server, the login
        // details for your account on that server and the UUID of the module
        // you wish to talk to.  Then compile this program and run it from a
        // command line.
        // You will be able to enter strings that will be sent to the module
        // as downlink datagrams and uplink datagrams received from the module
        // will be displated.  Press enter at the prompt without entering any
        // characters to exit the program.
        //
        // IMPORTANT: the DLLs that provide the communication path to the server
        // use IP ports 5671 (AMQP SSL) and 5672 (AMQP).  If you are unable
        // to connect to the NB-IoT device endpoint using this example and
        // your URL/username/password/UUID all appear correct, then make sure
        // there is no firewall on your PC, or on your company network, blocking
        // these ports.

        static Connection gConnection;
        static System.Threading.Timer gReceiveTimer;

        static void Main(string[] args)
        {
            // Fill these fields in with your Huawei server host name, the
            // credentials for your account on that server and the UUID of
            // the module you wish to communicate with (obtainable from your
            // Huawei server account page)
            String hostname = "iotv2b.neul.com";
            String username = "demo";
            String password = "demodemo";
            String uuid = "2c2fb400-f1d5-11e5-8ed5-fdef214758f5";

            String sendString;
            Boolean stop = false;

            gConnection = Connection.Create(hostname, username, password);
            Console.WriteLine("Purging old messages...");
            gConnection.PurgeMessages();
            Console.WriteLine(String.Format("Connecting to {0}...", hostname));
            gConnection.Open();
            Console.WriteLine("Connected.");
            // Start the receive callback timer
            gReceiveTimer = new System.Threading.Timer(receiveCallback, null, 1000, 0);

            while (!stop)
            {
                Console.WriteLine("Type a string to send and press <enter>, or just press <enter> on a blank line to terminate.");
                Console.Write("> ");
                sendString = Console.ReadLine();

                if (sendString.Length > 0)
                {
                    Guid guid = new Guid(uuid);
                    Byte[] sendDatagram = Encoding.UTF8.GetBytes(sendString);

                    // 4 is the UART endpoint on the module
                    Console.WriteLine(String.Format("Sending datagram \"{0}\" to uart endpoint.", Encoding.UTF8.GetString (sendDatagram)));
                    gConnection.Send(guid, 4, sendDatagram);
                }
                else
                {
                    stop = true;
                }
            }

            // Clean up
            if (gConnection != null)
            {
                Console.WriteLine("Disconnecting...");
                lock (gReceiveTimer)
                {
                    gReceiveTimer.Dispose();
                    gConnection.Shutdown();
                    gConnection = null;
                }
                Console.WriteLine("Disconnected.");
            }
        }

        // Timer callback for polling NeulNet
        static void receiveCallback(object state)
        {
            lock (gReceiveTimer)
            {
                if (gConnection != null)
                {
                    var jsonMsg = gConnection.Receive(100);
                    if (jsonMsg != null)
                    {
                        var rsp = jsonMsg as JsonMessages.AmqpResponse;
                        if (rsp != null)
                        {
                            Console.WriteLine(String.Format("[Received datagram: {0}, \"{1}\"]", rsp.ToString(), Encoding.UTF8.GetString (rsp.Data)));
                            Console.Write("> ");
                        }
                    }

                    gReceiveTimer.Change(1000, 0);
                }
            }
        }

    }
}
