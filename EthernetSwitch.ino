/*
#include <SPI.h>
#include <Ethernet.h>
*/


#include <UIPEthernet.h>
#include <UIPUdp.h>

EthernetServer server(80);  // create a server at port 80

const int startPin = 3;
const int cnt = 2;

String HTTP_req;          // stores the HTTP request
boolean port_status[cnt];   // state of port, off by default

void setup()
{
    // MAC address from Ethernet shield sticker under board
    byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };                                       
      
    IPAddress ip(192,168,1,179); //<<< ENTER YOUR IP ADDRESS HERE!!!
  
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    for (int i = startPin; i < startPin + cnt; i++)
      pinMode(i, OUTPUT);
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req.concat(c);
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    //Serial.print("HTTP_req: "); Serial.println(HTTP_req);
                    
                    bool found = false;
                    for (int i = 0; !found && i < HTTP_req.length() - 11; i++)
                    {
                      found = HTTP_req[i + 0] == 'G' && 
                              HTTP_req[i + 1] == 'E' && 
                              HTTP_req[i + 2] == 'T' && 
                              HTTP_req[i + 3] == ' ' && 
                              HTTP_req[i + 4] == '/' && 
                              HTTP_req[i + 5] == 's' && 
                              HTTP_req[i + 6] == 'w' && 
                              HTTP_req[i + 7] == 'i' &&
                              HTTP_req[i + 8] == 't' &&
                              HTTP_req[i + 9] == 'c' &&
                              HTTP_req[i + 10] == 'h';
                    }

                    if (found)
                    {
                        //Serial.println("send web page");
                        
                        // send a standard http response header
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();
                        // send web page
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Arduino port Control</title>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<p>Click to switch ports on and off.</p>");
                        client.println("<form method=\"get\">");
                        ProcessBtn(client);
                        client.println("</form>");
                        client.println("</body>");
                        client.println("</html>");
                    }
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

void ProcessBtn(EthernetClient cl)
{
    bool found = false;
    bool on_off = false;
    int port = 0;
    for (int i = 0; !found && i < HTTP_req.length() - 19; i++)
    {
      found = HTTP_req[i + 0] == 'G' && 
              HTTP_req[i + 1] == 'E' && 
              HTTP_req[i + 2] == 'T' && 
              HTTP_req[i + 3] == ' ' && 
              HTTP_req[i + 4] == '/' && 
              HTTP_req[i + 5] == 's' && 
              HTTP_req[i + 6] == 'w' && 
              HTTP_req[i + 7] == 'i' && 
              HTTP_req[i + 8] == 't' && 
              HTTP_req[i + 9] == 'c' && 
              HTTP_req[i + 10] == 'h' && 
              HTTP_req[i + 11] == '?' && 
              HTTP_req[i + 12] == 'P' && 
              HTTP_req[i + 13] == 'O' &&
              HTTP_req[i + 14] == 'R' &&
              HTTP_req[i + 15] == 'T' &&
              HTTP_req[i + 17] == '=' &&
              (HTTP_req[i + 18] == '0' || HTTP_req[i + 18] == '1');
      if (found)
        for (found = false, port = 0; port < cnt; port++)
          if (found = (HTTP_req[i + 16] == '1' + port))
          {
            on_off = HTTP_req[i + 18] == '1';
            break;
          }
    }

    if (found) {
        port_status[port] = on_off;

        if (port_status[port]) {    // switch port on
            digitalWrite(startPin + port, HIGH);
        }
        else {              // switch port off
            digitalWrite(startPin + port, LOW);
        }
    }

    for (int j = 0; j < cnt; j++)
    {
      char data[3];
      cl.print("<br><button name=\"PORT");
      sprintf(data, "%c", '1' + j);
      cl.print(data);
      cl.print("\" value=\"");
      if (port_status[j])
        cl.print("0");
      else
        cl.print("1");
      cl.print("\" onclick=\"submit();\">Toggle port ");
      sprintf(data, "%d", j + 1);
      cl.print(data);
      cl.print(" (now ");
      if (port_status[j])
        cl.print("on");
      else
        cl.print("off");
      cl.println(")</button><br>");
    }
}
