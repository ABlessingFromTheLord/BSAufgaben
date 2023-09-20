#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


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
    char *arguments = strtok(NULL, " \t");
    printf("\nGiven command: %s", command);
    printf("\nGiven parameters:%s", arguments);

    // starting process
    pid_t newProcess = fork();
    if (newProcess == -1)
    {
        printf("New process could not be created");
        return -1;
    }
    
    char *arr[] = {arguments, NULL};
    printf("%d\n", newProcess);
    execv("BSAufgaben/clash", arr);

    // calling waitpid with WNOHANG
    int status;

    while (1)
    {
        int code = waitpid(-1, &status, WNOHANG);
        // Error occurred in the last call
        if(code == -1){
            printf("Error while executing WAITPID with WNOHANG\n");
            return -1;
        }
        // checking other codes
        else if(code > 0){
                    if(WIFEXITED(status)){
                    printf("Exited , Status = [%d]\n", WEXITSTATUS(status));
                }

            else if(WIFSIGNALED(status)){
                printf("The process gets the signal = [%d]\n", WTERMSIG(status)); // gets the signal number
            }
            // monitoring if the child process is being signaled with a stop signal
            else if (WIFSTOPPED(status)){
                printf("Proces sis stoppped by [%d]\n", WSTOPSIG(status));
            }
            // check if process is being resumed fgrom  stop state
            else if (WIFCONTINUED(status)){
                printf("The process is being resumed");
            }
        }

   
    }
    
    return 0;

}