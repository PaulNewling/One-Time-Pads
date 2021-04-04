/* Author: Paul Newling
*  Class: OSU CS344 F2020
*  Program: Assignment 5: One-Time Pads */

#include "helper.h"


// Error function used for reporting issues, given in client.c
// https://repl.it/@cs344/83serverc?lite=true#server.c
void error(const char *msg, int exitCode) {
    fprintf(stderr, "%s\n",msg);
    exit(exitCode);
} 

/* 
* This function takes the socket and the message pointer and then
*   sends the length of the message and then the message itself.
 */
void sendMsg(int socketFD, char *message){

    char buffer[256];
    int charsWritten, msgLength = 0;

    // Get the message length and implace it into the transmission buffer. 
    //  This is then sent without a while loop since it will be small enough to 
    //  not fragment
    msgLength = strlen(message);
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "%d", msgLength);
    charsWritten = send(socketFD, buffer, strlen(buffer), 0);
    if(charsWritten < 0){
        error("Failed to send message length", 2);
    }

    // Sometimes the message are send too close together, so we sleep 50 milliseconds
    //  to ensure they come through as separate messages.
    usleep(50);
    
    // Send the text data of the message over the socket
    if (sendAll(socketFD, message, &msgLength) == -1){
        error("Error sending file\n", 2);
    }

}

/* 
* This function takes the socket and the message pointer and then
*   receives the length of the message and then the full message itself.
 */
void recvMsg(int socketFD, char *message){

    char buffer[256];
    int charsRead, msgLength, bytesIn = 0;

    // Read in message length, this is just a number so we don't expect it to fragment, so the
    //  while loop is only run once
    memset(buffer, '\0', sizeof(buffer));
    while(charsRead == 0){  
      charsRead = recv(socketFD, buffer, sizeof(buffer), 0); 
      if (charsRead < 0){
        error("Error reading from socket",2);
      }
      msgLength = atoi(buffer);
    }

    // Reset out transmission-read buffer and our character read count
    charsRead = 0;
    memset(buffer, '\0', sizeof(buffer));

    // Continuously loop over recv() until the entire message is read (as noted
    //  by reaching the message length). For each recv() segment we concatenate these 
    //  portions to create the full message.
    while(charsRead < msgLength){  

        bytesIn = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
      
        // Keep track of the chars read in
        charsRead += bytesIn;

        // This will only be negative if there is an error
        if (bytesIn < 0){
            error("Error reading from socket", 2);
        }
      
        // Create message by continually concatenating segmesnts
        strcat(message, buffer);
      
        // Clear transmission read buffer for next section
        memset(buffer, '\0', sizeof(buffer));
    } 
}


/* This function adds the text from a file to the full message that
*   will be later sent from client to server. It also adds a "@" character
*   the end of the file text in the buffer, this will be used for determining
*   what text is what in the server. (The server will parse the full message
*   text looking for "@" symbols.)
*/
void addFileToMessage(char *fileName, char *buffer){

    // Prepare getLine()
    size_t getLineBuf = 0;
    char *inputLine = NULL;
    FILE *file;

    // Open filename passed to function
    file = fopen(fileName, "r");
    if (!file){
        error("Error opening file.\n", 1);
    }

    // Get text from file, this will all be one line as dictated by the assignment
    getline(&inputLine,&getLineBuf, file);
    
    // Remove the trailing \n that getline adds
    inputLine[strcspn(inputLine, "\n")] = '\0';
    
    // Adds the file text to the full message buffer and attaches '@' character
    strcat(buffer, inputLine);
    strcat(buffer, "@");

    // Frees input line and closes file
    free(inputLine);
    fclose(file);
}


/* This function check to see that all of the characters in the file are
*   valid (only capital letters and spaces), erroring out if any non-valid
*   charactors are found. If everything is valid it returns the length of the file
*/
int getLengthCheckIsValid(char *fileName){

    char ch;
    int length = 0;
    FILE * file;

    // Open filename passed to function
    file = fopen(fileName, "r");
    if (file == NULL){
        error("Error opening file.\n", 1);
    }

    // Goes character by character until the end of file or a new line is hit
    //  checking each to see if it is an uppercase letter or space. If it is not
    //  an error is thrown
    ch = fgetc(file);
    while ( ch != EOF && ch != '\n'){
        if(isupper(ch) || ch == 32){ 
            length++;
        }
        else{
            error("Error: Bad character in file.\n", 1);
        }
        ch = fgetc(file);
    }

    // Close file, return length of file
    fclose(file);
    return length;
}


/* This function sends large text files by repeatedly calling send
*   while moving over a message buffer, using what send() returns
*   to know where to move the pointer to for the next send() call.
*   this was modified from Beej's Guide to Network Programming,
*   section 7.4 Handling Partial send()s:
*   https://beej.us/guide/bgnet/html/#sendall */
int sendAll(int socket, char *buffer, int *length)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *length; // how many we have left to send
    int n;

    // While less total sent is less than length of message
    while(total < *length) {

        // n is the amount of data sent with this call of send()
        n = send(socket, buffer+total, bytesleft, 0);

        // if n is negative there is an error sending;
        if (n == -1){
            break;
        }

        // Increase the pointer position the amount of data we sent this loop
        total += n;

        // Reduce the amount left the send
        bytesleft -= n;
    }

    *length = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

/* This function Set up the address struct for the server socket, it was 
*   adapted from the given code in https://repl.it/@cs344/83serverc?lite=true#server.c
*/
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}


// Converts an uppercase letter to an int, and a space to the number 26
int charToInt(char c){

    if(c == ' '){
        return 26;
    }

    return (c - 'A');
}

// Converts an int to an uppercase char and the number 26 to a space
char intToChar(int i){

    if(i == 26){
        return ' ';
    }

    return (i + 'A');
}


/* Encrypts a text file, with a key file in the method of a One-Time Pad
*   https://en.wikipedia.org/wiki/One-time_pad storing the encrypted text
*   in the ciphertext string
*/
void encrypt(char* cipherText, char *plainText, char *key){

    // Gets the length of the plaintext and creates an int array of the same
    //  length. This int array will be use to store the intrum partially coded
    //  message that will then be converted back into characters
    int length = strlen(plainText);
    int cipherInt[length];

    // Loop through entire length of plaintext
    for(int i = 0; i < length; i++){
      
        // Convert each letter from the plaintext and key into an int and add them together,
        //    modulo 27 (26 letters in the alphabet +1 for space) and store in the int array.
        cipherInt[i] = (charToInt(plainText[i]) + charToInt(key[i])) % 27;

        // Convert this int in the int array into an upperase letter and store in the cipher text.
        cipherText[i] = intToChar(cipherInt[i]);

    }

    // Add a newline to the end of the text as this will be needed for printing/sending
    cipherText[length] = '\n';
}


/* Decrypts a text file, with a key file in the method of a One-Time Pad
*   https://en.wikipedia.org/wiki/One-time_pad storing the decrypted text
*   in the plaintext string
*/
void decrypt(char* cipherText, char *plainText, char *key){

    // Gets the length of the ciphertext and creates an int array of the same
    //  length. This int array will be use to store the intrum partially decoded
    //  message that will then be converted back into characters
    int length = strlen(cipherText);
    int cipherInt[length];

    // Loop through entire length of plaintext
    for(int i = 0; i < length; i++){
      
        // Convert each letter of the ciphertext and key into an int and subtract the
        //  key from the ciphertext, store this in the int array.
        cipherInt[i] = (charToInt(cipherText[i]) - charToInt(key[i]));

        //  If any number is less than zero, add 27 as we can't convert negative number to characters
        //  at least in this program
        if (cipherInt[i] < 0){
            cipherInt[i] += 27;
        }

        // Convert ints to uppercase chars and store in plaintext
        plainText[i] = intToChar(cipherInt[i]);

    }

    // Add a newline to the end of the text as this will be needed for printing/sending
    plainText[length] = '\n';
}

