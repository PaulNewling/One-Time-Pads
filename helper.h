/* Author: Paul Newling
*  Class: OSU CS344 F2020
*  Program: Assignment 5: One-Time Pads */

#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // usleep()
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <ctype.h>      // isUpper()


void error(const char *, int);
void addFileToMessage(char *, char *);
int getLengthCheckIsValid(char *);
void sendMsg(int, char *);
void recvMsg(int, char *);
int sendAll(int , char *, int*);
void setupAddressStruct(struct sockaddr_in* , int);
int charToInt(char);
char intToChar(int);
void encrypt(char* , char *, char *);
void decrypt(char* , char *, char *);

#endif