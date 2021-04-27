/* myserver.cpp */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib> // update from deprecated header
#include <cstdio> // update from deprecated header
#include <cstring> // update from deprecated header
// The fstream library allows us to work with files.
// To use the fstream library, include both the standard <iostream> AND the <fstream> header file:
#include <iostream>
#include <fstream>
#include <pthread.h> // in order to make a threaded server
#include <sys/types.h> // in order to use pid_t tid = gettid();
#include <sys/stat.h> // for checking if the directory exists
#include <filesystem> // for creating the directory
#define BUF 1024
//#define PORT 6543
//temporarily change port because I can't bind to the other one right now
#define PORT 6655 // maybe i need to iterate through ports at some point? need to look up where the free ones start and end
#define BACKLOG 5 // number of connections i will accept at once, using 5 because it was specified in the server sample

// TODO: work on error handling e.g. if(Myfile.is_open()){...} oder wenn man ein file erstellt
//  oder if (socket creation, bind oder listen == failed == return -1) -> do something. Also wenn die funktionen -1 zurückgeben dann stimmt etwas nicht
//  oder buffer overflow etc
// TODO: write code to handle possible changes/adjustments to the exercise
// TODO: function prototype?
// TODO: have the server sending back responses in more situations, this will also help in checking whether the concurrency works correctly
// TODO: maybe i can move some of these variables back into main if i use a function prototype and move my handleRequest function to the bottom
// TODO: make sure files with the received messages are saved in the directory that was supplied as a parameter
// TODO: remove global variables by creating a function prototype and moving handleRequest to the bottom?
// TODO: readline()? siehe angabe zur übung
// TODO: most importantly, check for errors (e.g. send() function returns -1 [if (send(sd, string, len, 0) == -1) { /* error */ })
// TODO: second most importantly, implement synchronisation
// TODO: nachfragen ob ich das mit den threads anders machen sollte

// TODO: check for alternatives to so many global variables. function prototype may help. look at examples: https://www.youtube.com/watch?v=Pg_4Jz8ZIH4 https://gist.github.com/codehoose/020c6213f481aee76ea9b096acaddfaf
int create_socket, new_socket;
socklen_t addrlen;
char buffer[BUF];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // zur verwaltung von mutexes
int size;
struct sockaddr_in address, cliaddress;
// my own variables [should i move these to handleRequest?]
int fileCounter = 0; // name files starting from 0.txt etc. // TODO: should I use 0 or 1 here?
std::string path; // for the directory that was passed to the server

// int sockfd = socket(domain, type, protocol)
// sockfd: socket descriptor, an integer (like a file-handle)

// domain: integer, communication domain e.g., AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol)

// type: communication type
// SOCK_STREAM: TCP(reliable, connection oriented) [was in der angabe verlangt wird]
// SOCK_DGRAM: UDP(unreliable, connectionless)



//thread functions need to return a void pointer and they need to accept a pointer
//int size, int new_socket, char buffer[BUF], int fileCounter, int create_socket
void * handleRequest(void* pointer_create_socket) { // in order to have concurrency, I should also have function that can be called for each request
    int create_socket = *((int*)pointer_create_socket);
    free(pointer_create_socket); // not needed any longer

    //thread ausgeben start
    pid_t tid = gettid();
    std::string yourThreadNumber = "Hello! You are using thread number " + std::to_string(tid) + "\n";
    const char *result = yourThreadNumber.c_str();
    strcpy(buffer, result);
    send(new_socket, buffer, strlen(buffer),0);
    //thread ausgeben end

    path = "./" + path + "/"; //format the relative file path correctly

    do {
        // Receive a reply from the server
        size = recv (new_socket, buffer, BUF-1, 0);
        if( size > 0)
        {
            buffer[size] = '\0';
            printf ("Message received: %s\n", buffer);
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
                printf("THE PATH IS: %s\n", path.c_str());

                ++fileCounter;
                std::string filename = std::to_string(fileCounter) + ".txt";

                // Create and open a text file
                std::ofstream createdFile(path + filename);

                // starting in the array after the command,
                // read the contents of the array
                // as long as the message isn't ".\n":
                while (memcmp(buffer, ".", strlen(".")) != 0) {
                    // Write to the file
                    // recv again
                    //createdFile << recv (new_socket, buffer, BUF-1, 0); //something goes wrong here, the wrong characters are put in the file
                    // Wait for client to send data
                    int bytesReceived = recv(new_socket, buffer, BUF, 0);
                    //message = (new_socket->buffer);
                    if (memcmp(buffer, ".", strlen(".")) != 0) {
                        createdFile << std::string(buffer, 0, bytesReceived);
                    }
                }
                // Close the file
                createdFile.close();
                // Save file count persistently
                std::ofstream numberOfFiles(path + "fileCount.txt");
                numberOfFiles << std::to_string(fileCounter);
                numberOfFiles.close();

            } else if (memcmp(buffer, "LIST", strlen("LIST")) == 0) { // print the number of quotes/files //TODO: also print all files
                printf("LIST COMMAND RECOGNIZED\n"); // I don't actually have to count anything: i already counted it, all I have to do is save it persistently
                // assign value to string
                //std::string result = "The amount of quotes is " + getLine(std::ifstream numberOfFiles("fileCount.txt")) + "\n";
                // Create a text string, which is used to output the text file
                std::string result;
                // Read from the text file
                std::ifstream numberOfFiles(path + "fileCount.txt");
                std::getline(numberOfFiles, result);
                numberOfFiles.close();
                result.append(" is the amount of quotes \n");
                strcpy(buffer, result.c_str());
                send(new_socket, buffer, strlen(buffer),0);

            } else if (memcmp(buffer, "QUOTE", strlen("QUOTE")) == 0) {
                printf("QUOTE COMMAND RECOGNIZED\n");
                // basically, all I have to do is randomize a number between 1 and the highest one used
                // i can get the highest number by reading fileCount.txt

                std::string fileContent;
                int randomNumber;
                std::string highestFileNumber;

                // Read from the text file
                std::ifstream numberOfFiles(path + "fileCount.txt");
                std::getline(numberOfFiles, highestFileNumber);
                numberOfFiles.close();


                randomNumber = (rand() % std::stoi(highestFileNumber)) + 1;

                // Read from the text file
                std::ifstream createdFile(path + std::to_string(randomNumber) + ".txt");
                std::getline(createdFile, fileContent);
                createdFile.close();

                fileContent.append("\n");

                strcpy(buffer, fileContent.c_str());
                send(new_socket, buffer, strlen(buffer),0);

            } else if (memcmp(buffer, "LOGOUT", strlen("LOGOUT")) == 0) {
                printf("LOGOUT COMMAND RECOGNIZED\n");
                //close (create_socket); TODO
                //return EXIT_SUCCESS; // or should I put a break here?
                return nullptr; //replace return values with NULL to appease the compiler, since now our function is supposed to return a pointer [replace with nullptr instead of NULL because of clang-tidy]
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
            //return EXIT_FAILURE; TODO
            return nullptr; //replace return values with NULL to appease the compiler, since now our function is supposed to return a pointer [replace with nullptr instead of NULL because of clang-tidy]
        }
    } while (strncmp (buffer, "quit", 4)  != 0);
    close (new_socket);
    return nullptr;
}

#pragma clang diagnostic push // surpress endless loop warning for this function
#pragma ide diagnostic ignored "EndlessLoop" // surpress endless loop warning for this function
int main (int argc, char *argv[]) {
  // protocol: Protocol value for Internet Protocol(IP), which is 0.
  // This is the same number which appears on protocol field in the IP header of a packet.(man protocols for more details)
  create_socket = socket (AF_INET, SOCK_STREAM, 0);

  memset(&address,0,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (PORT); // The htons() function makes sure that numbers are stored in memory in network byte order, which is with the most significant byte first (aka big-endian). Note that if you were working on a big-endian machine, the htons() function would not need to do any swapping since the number would already be stored in the right way in memory.

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
  listen (create_socket, BACKLOG); // so there was already a backlog of 5 specified
  
  addrlen = sizeof (struct sockaddr_in);

  //an dieser stelle sollte die synchronisation stattfinden
  while (true) {
     printf("Waiting for connections...\n");
     // int new_socket= accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     // It extracts the first connection request on the queue of pending connections for the listening socket,
     // sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket.
     // At this point, connection is established between client and server, and they are ready to transfer data.
     new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen ); // accept() liefert einen neuen Socket Deskriptor new_sd für die weitere Kommunikation mit dem Client
     if (new_socket > 0)
     {
        printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
        // char * strcpy ( char * destination, const char * source );
        // Copies the C string pointed by source into the array pointed by destination,
        // including the terminating null character (and stopping at that point).
        strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
        send(new_socket, buffer, strlen(buffer),0);

        //after this, a connection has been successfully established
        //whenever a connection has been established, i should call the handleRequest function

         // start check whether exists, if not create directory
         if( argc == 2 ) {
             printf("The argument supplied is %s\n", argv[1]);
             /* struct stat st = {0}; [this would be the way to do it in c]
             const std::string path = std::string(argv[1]);

             if (stat(path.c_str(), &st) == -1) {
                 mkdir(path.c_str(), 0777); // Here you should provide 0777, an octal number meaning all of r, w and x
             } */

             // c++ way of creating a directory
             path = std::string(argv[1]);
             try {
                 if(std::filesystem::create_directory(path)) {
                     std::cout << "Created a directory\n";
                 }
                 else {
                     std::cerr << "Failed to create a directory\n";
                 }
             } catch (const std::exception& e){
                 std::cerr << e.what() << '\n';
             }

         }
         else if( argc > 2 ) {
             printf("Too many arguments supplied.\n");
         }
         else {
             printf("One argument expected.\n");
         }
         // end check whether exists, if not create directory

     }

     //return handleRequest(size, new_socket, buffer, fileCounter, create_socket);
     pthread_t myThread; // pthread_t variable that keeps track of my thread
     //int *pointer_client = (int*)malloc(sizeof(int)); // less elegant, old fashioned c-esque way to do it
     int *pointer_create_socket = new int[sizeof(int)]; // the pointer shouldn't be interfered with by any other thread, therefore i allocate some space on the heap for an int
     *pointer_create_socket = create_socket; // store the value of the client socket
     pthread_create(&myThread, nullptr, handleRequest, pointer_create_socket); // pass in thread address, thread function (handleRequest), and a thread argument that needs to be a pointer [replace with nullptr instead of NULL because of clang-tidy]
     //the function passed as the thread callback must be a (void*)(*)(void*) type function
  }
  close (create_socket); // schließt den socket, danach weder senden noch empfangen möglich. Der Socket Deskriptor von create_socket wird auch wieder freigegeben.
  return EXIT_SUCCESS;
}
#pragma clang diagnostic pop // surpress endless loop warning for this function
