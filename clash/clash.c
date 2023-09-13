#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include<unistd.h>


int main(int argc, char *argv[])
{
    // outputting current directory
    // path upto 1024 characters
    char cwd[1024];
    if(getcwd(cwd, 1024) == NULL)
    {
        printf("Get current working directory failed \n");
        if (errno == ERANGE)
        {
            printf("Path exceeds max buffer length \n");
            return 1;
        }
        
    }
    printf("\n\nCurrent working directory: %s\n", cwd);
    printf("Please input command: ");

    // getting input from standard input
    // why 256? have no idea
    char eingabe[256];

    if(fgets(eingabe, sizeof(eingabe), stdin) == NULL){
        printf("\nfgets failed");
        return -1;
    }

    // command tokenization
    // how can I use multiple delimiters as options
    char *command = strtok(eingabe, " \t");
    char *parameter = strtok(NULL, " \t");
    printf("\nGiven command: %s", command);
    printf("\nGiven parameters:%s", parameter);

    // starting process
    



}