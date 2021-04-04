/* Author: Paul Newling
*  Class: OSU CS344 F2020
*  Program: Assignment 5: One-Time Pads */

#include "helper.h"


/* 
* This server program was modified from the file given in the assignment
*   https://repl.it/@cs344/83serverc?lite=true#server.c
*   this was modified to then support multiple processes as well as sending and receiving 
*   larger messages of variable length. The server creates a connection with a client
*   and receives from the client a handshake, plaintext file and key file. If the handshake
*   is correct the server then returns the ciphertext of the plaintext encrypted with the key.
*/
int main(int argc, char *argv[]){
    int connectionSocket, authenticate, charsWritten = 0;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t spawnPid;
    char buffer[256], fullMessage[200000], plainText[100000], key[100000], cipherText[100000];
    char *token, *saveptr;


    // Check to ensure the correct number of arguments have been passed by the command line
    if (argc != 2) { 
        error("SERVER ERROR: Invalid argument count.", 1);
    } 
    
    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("SERVER ERROR: Error opening socket", 2);
    }

    // Make the socket reusable this was adapted from Beej's guide, section 5.3 bind()
    int yes=1;
    if (setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        error("SERVER ERROR: SetSockOpt error.\n", 2);
    } 

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("SERVER ERROR: Error on binding", 2);
    }

    // Start listening for connetions. Allow up to 5 connections.
    listen(listenSocket, 5); 
    
    // Accept a connection, blocking if one is not available until one connects
    while(1){

        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
        error("SERVER ERROR: Error on accept", 2);
        }

        // After the connection is accepted, create a fork, the parent will close and the child will create
        //  and return the encrypted text. This will allow the parent to continue to fork off children to 
        //  encrypt text while the parent accepts socket connections.
        spawnPid = fork();

        switch(spawnPid){

            // If -1 this is an error with the fork(), print this message to the user
            case -1:
                error("SERVER ERROR: Fork() failed.\n", 2);
                break;

            // If the fork returns 0, this is the child process where we will read, encrypt
            //  and return the message
            case 0:

                // Clear all message buffers to ensure they are prepared to receive the messages
                memset(fullMessage, '\0', sizeof(fullMessage));
                memset(plainText, '\0', sizeof(plainText));
                memset(cipherText, '\0', sizeof(cipherText));

                // Receive the message over connectionSocket into fullMessage
                recvMsg(connectionSocket, fullMessage);

                // The full message contains the handshake, plaintext message and key, all seperated
                //  by the "@" symbol. This can be done since we already check each message for symbols,
                //  and any text with symbols will be aborted before it gets to this stage, that way we know
                //  we  are not getting an errant symbol
                token = strtok_r(fullMessage, "@", &saveptr);

                // First token is check against the encryption hand shake, if it is missing this connection is
                //  not authorized and  will return a 0 to the client so they client can exit as well
                if(strstr(token, "enc") == NULL){

                    // Invalid handshake
                    authenticate = 0;

                    // Clear buffer, implace authenticate token and send to client
                    memset(buffer, '\0', sizeof(buffer));
                    sprintf(buffer, "%d", authenticate);
                    charsWritten = send(connectionSocket, buffer, 2, 0);
                    if(charsWritten < 0){
                        error("SERVER ERROR: Failed to send authenticate\n", 2);
                    }

                    // Print error to user
                    error("SERVER ERROR: Invalid Client Authentication.", 2);
                }

                // If the handshake is successful, send the appropriate return message to the client so
                //  the both client/server can continue.
                else{
                    authenticate = 1;
                    memset(buffer, '\0', sizeof(buffer));
                    sprintf(buffer, "%d", authenticate);
                    charsWritten = send(connectionSocket, buffer, 2, 0);
                    if(charsWritten < 0){
                        error("SERVER ERROR: Failed to send authenticate\n", 2);
                    }
                }

                // Handshake was successful, so now the plaintext is copied from the received message
                token = strtok_r(NULL, "@", &saveptr);
                strcpy(plainText, token);

                // Key is also copied form the full message
                token = strtok_r(NULL, "@", &saveptr);
                strcpy(key, token);

                // PlainText is encrypted with the key and becomes the cipherText
                encrypt(cipherText, plainText, key);

                // CipherText is sent back to the client
                sendMsg(connectionSocket, cipherText);

                // Child exits successfully (This also closes the socket)
                exit(0);

            // Parent process just closes the socket and goes back to listening
            default:
                close(connectionSocket);

        }
     
     }
     
    // Close the listening socket
    close(listenSocket); 
    return 0;
}
