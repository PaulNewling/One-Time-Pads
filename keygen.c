
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char *argv[])
{
    time_t t;
    srand(time(&t));
    int keyLength, randNum;

    if (argc != 2)
    {
        //https://stackoverflow.com/questions/39002052/how-i-can-print-to-stderr-in-c
        fprintf(stderr,"%s","KEYGEN: You must provide number of characters in the key.\n");
        exit(1);
    }

    keyLength = atoi(argv[1]);
    
    
    for(int i = 0; i < keyLength; i++){
        randNum = rand() % 27;
        if(randNum == 26){
            randNum = 32;
        }
        else{
            randNum += 65;
        }
        printf("%c",randNum);
    }
    printf("\n");

    return 0;
}