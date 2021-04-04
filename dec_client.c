/* Author: Paul Newling
*  Class: OSU CS344 F2020
*  Program: Assignment 5: One-Time Pads */

#include "helper.h"


/* 
* This client program was modified from the file given in the assignment
*   https://repl.it/@cs344/83clientc?lite=true#client.c
*   This program was modified to check the appropriate key and cipherText files
*   for correctness (ie only letters and spaces, no symbols), and that the key
*   file is longer than the plain text file. After this a handshake, plus the 
*   cipherText and keyfile are all loaded into one message and sent to the server.
*   if the handshake is accepted a plaintext is returned and the client prints
*   this text.
*/
int main(int argc, char *argv[]) {
    int socketFD, charsRead, textLength, keyLength, authenticate = 0;
    struct sockaddr_in serverAddress;
    char buffer[256], fullMessage[200000], plainText[100000];

    // Check to ensure the correct number of arguments have been passed via command line
    if (argc != 4) { 
        error("CLIENT ERROR: Invalid argument count.\n",1);
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT ERROR: Error opening socket",2);
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT ERROR: Error connecting",2);
    }

    // Check the cipherText and key file to ensure they are valid. Then place their lengths
    //  into the correstponding variables that will be used to check key > text length
    textLength = getLengthCheckIsValid(argv[1]);
    keyLength = getLengthCheckIsValid(argv[2]);
    
    // Ensure that the key file is larger than the cipherText, this is required for a one-time pad
    if (keyLength < textLength){
        error("CLIENT ERROR: key length too short",1);
    }

    // Add the corresponding handshake message to the full message
    char authToken[5] = "dec@";
    memset(fullMessage, '\0', sizeof(fullMessage));
    strcat(fullMessage, authToken);
    
    // Add the cipherText and key to the full message
    addFileToMessage(argv[1], fullMessage);
    addFileToMessage(argv[2], fullMessage);

    // Send the full message to the server
    sendMsg(socketFD, fullMessage);

    // Receive the handshake authorization.
    memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;

    // Since we are only receiving one text character we don't need to worry about large messages
    while(charsRead == 0){  
        charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); 
        if (charsRead < 0){
            error("CLIENT ERROR: Error reading from socket",2);
        }
        authenticate = atoi(buffer);
    }

    // If the handshake is not accepted the program will exit.
    if(!authenticate){
        char errorMsg[60];
        memset(errorMsg, '\0',  sizeof(errorMsg));
        sprintf(errorMsg, "CLIENT ERROR: Not supported by server on port %s", argv[3]);
        error(errorMsg,2);
    }
    
    // Occasionally the messages were stacking on top of each other, to prevent this we add a 
    // 50 millisecond sleep between recv()s
    usleep(50);

    // Receive the plaintext back from the server
    recvMsg(socketFD, plainText);

    // Print this text as appropriate
    printf("%s", plainText);

    // Close the socket
    close(socketFD); 
    return 0;
}