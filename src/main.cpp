// This piece of code is taken from: geeksforkeeks.com
// for testing purposes only
// Example code: A simple server side code, which echos back the received message.
// Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <iostream>

#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAX_CLIENTS 3
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int _masterSocket, addrlen, _newSocket, _clientSocket[MAX_CLIENTS], activity, i, _valRead, sd;
    int _maxSD;
    struct sockaddr_in address;

    char buffer[BUFFER_SIZE + 1]; // data buffer of 1K + 1 bytes
    const char *replyBuffer = "--> [ACKED by server]\r\n";

    // set of socket descriptors
    fd_set readfds;

    // a message
    const char *message = "Connected to Simulation Server v1.0 \r\n";

    // initialise all _clientSocket[] to 0 so not checked
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        _clientSocket[i] = 0;
    }

    // create a master socket
    if ((_masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(_masterSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to localhost port 8888
    if (bind(_masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("--> [Server]- Listening on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(_masterSocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    puts("--> Waiting for connections ...");

    while (TRUE)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(_masterSocket, &readfds);
        _maxSD = _masterSocket;

        // add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            sd = _clientSocket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > _maxSD)
                _maxSD = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(_maxSD + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket ,
        // then its an incoming connection
        if (FD_ISSET(_masterSocket, &readfds))
        {
            if ((_newSocket = accept(_masterSocket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n ", _newSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // send new connection greeting message
            if (send(_newSocket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            // add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (_clientSocket[i] == 0)
                {
                    _clientSocket[i] = _newSocket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        // else its some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = _clientSocket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing , and also read the
                // incoming message
                if ((_valRead = read(sd, buffer, BUFFER_SIZE)) == 0)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    _clientSocket[i] = 0;
                }

                // Echo back the message that came in
                else
                {
                    // set the string terminating NULL byte on the end
                    // of the data read
                    buffer[_valRead] = '\0';
                    send(sd, replyBuffer, strlen(replyBuffer), 0);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    std::cout << "==== The Simulation process was completed successfully ===="
              << std::endl;

    return 0;
}
