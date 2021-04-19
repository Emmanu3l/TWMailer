/* myserver.cpp */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// The fstream library allows us to work with files.
// To use the fstream library, include both the standard <iostream> AND the <fstream> header file:
#include <iostream>
#include <fstream>
#define BUF 1024
//#define PORT 6543
//temporarily change port because I can't bind to the other one right now
#define PORT 6543

void receiveQuote() { // in order to have concurrency, I should also have function that can be called for each request

}

int main (void) {
  int create_socket, new_socket;
  socklen_t addrlen;
  char buffer[BUF];
  int size;
  struct sockaddr_in address, cliaddress;

  // my own variables
  int fileCounter = 0; // name files starting from 1.txt etc.
  std::string message; //variable for receiving quotes via message
  //string message;

  // int sockfd = socket(domain, type, protocol)
  // sockfd: socket descriptor, an integer (like a file-handle)

  // domain: integer, communication domain e.g., AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol)

  // type: communication type
  // SOCK_STREAM: TCP(reliable, connection oriented)
  // SOCK_DGRAM: UDP(unreliable, connectionless)

  // protocol: Protocol value for Internet Protocol(IP), which is 0.
  // This is the same number which appears on protocol field in the IP header of a packet.(man protocols for more details)
  create_socket = socket (AF_INET, SOCK_STREAM, 0);

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (PORT);

  // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
  // After creation of the socket, bind function binds the socket to the address and port number specified in addr(custom data structure).
  // In the example code, we bind the server to the localhost, hence we use INADDR_ANY to specify the IP address.
  if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0) {
     perror("bind error");
     return EXIT_FAILURE;
  }
  // int listen(int sockfd, int backlog);
  // It puts the server socket in a passive mode, where it waits for the client to approach the server to make a connection.
  // The backlog, defines the maximum length to which the queue of pending connections for sockfd may grow.
  // If a connection request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED.
  listen (create_socket, 5);
  
  addrlen = sizeof (struct sockaddr_in);

  while (1) {
     printf("Waiting for connections...\n");
     // int new_socket= accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     // It extracts the first connection request on the queue of pending connections for the listening socket,
     // sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket.
     // At this point, connection is established between client and server, and they are ready to transfer data.
     new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen );
     if (new_socket > 0)
     {
        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
        // char * strcpy ( char * destination, const char * source );
        // Copies the C string pointed by source into the array pointed by destination,
        // including the terminating null character (and stopping at that point).
        strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
        send(new_socket, buffer, strlen(buffer),0);
     }
     do {
         // Receive a reply from the server
        size = recv (new_socket, buffer, BUF-1, 0);
        if( size > 0)
        {
           buffer[size] = '\0';
           printf ("Message received: %s\n", buffer); // why does the text work fine here but not later? Should I use a different variable for messages?
           // message received --> do stuff
           // begin my code
           // there seems to be a character add the end of the message which I have to remove
           // A better solution [for comparing string data received from a socket in C],
           // which does not depend on the received data being null terminated is to use memcmp:
            if (memcmp(buffer, "ADD", strlen("ADD")) == 0) {
                printf("ADD COMMAND RECOGNIZED\n");
                // get contents of buffer after \n
                // add every line after that into the file, including the new line
                // stop listening after ".\n" and save the file

                // To create a file, use either the ofstream or fstream class, and specify the name of the file.
                // To write to the file, use the insertion operator (<<).

                std::string filename = std::to_string(fileCounter) + ".txt";
                ++fileCounter;

                // Create and open a text file
                std::ofstream MyFile(filename);

                // starting in the array after the command,
                // read the contents of the array
                // as long as the message isn't ".\n":
                do {
                    // Write to the file
                    // recv again
                    //MyFile << recv (new_socket, buffer, BUF-1, 0); //something goes wrong here, the wrong characters are put in the file
                    // Wait for client to send data
                    int bytesReceived = recv(new_socket, buffer, BUF, 0);
                    if (bytesReceived == -1)
                    {
                        std::cerr << "Error in recv(). Quitting" << std::endl;
                        break;
                    }

                    if (bytesReceived == 0)
                    {
                        std::cout << "Client disconnected " << std::endl;
                        break;
                    }


                    //message = (new_socket->buffer);
                    MyFile << std::string(buffer, 0, bytesReceived) << std::endl;
                }
                while (memcmp(buffer, "\n.", strlen("\n.")) != 0);
                // Close the file
                MyFile.close();

            } else if (memcmp(buffer, "LIST", strlen("LIST")) == 0) {
                printf("LIST COMMAND RECOGNIZED\n");
            } else if (memcmp(buffer, "QUOTE", strlen("QUOTE")) == 0) {
                printf("QUOTE COMMAND RECOGNIZED\n");
            } else if (memcmp(buffer, "LOGOUT", strlen("LOGOUT")) == 0) {
                printf("LOGOUT COMMAND RECOGNIZED\n");
            } else {
                printf("COULD NOT READ COMMAND\n");
            }
           // end my code
        }
        else if (size == 0)
        {
           printf("Client closed remote socket\n");
           break;
        }
        else
        {
           perror("recv error");
           return EXIT_FAILURE;
        }
     } while (strncmp (buffer, "quit", 4)  != 0);
     close (new_socket);
  }
  close (create_socket);
  return EXIT_SUCCESS;
}