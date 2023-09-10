//
// Created by SamwelSuleiman on 05/09/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int errno;

// Comparison function
int comparator(const void *string1_void, const void *string2_void){
    // void is general, so we need it to be cast to string
    // we cast them and dereference them to get the value´
    // fist star is to dereference, the second is the cast
    // ** is because its a pointer to a pointer
    char *string1 = *(char **) string1_void;
    char *string2 = *(char **) string2_void;
    
    return strcmp(string1, string2);
}

// Main function
int main(int argc, char *argv[])
{

    // reading file
    FILE *fp;
    fp = fopen( "wlist0", "r");
    if (fp == NULL)
    {
        printf("\n The file could not be opened");
        exit(1);
    }

    int BUFFER_SIZE = 100;
    char *buffer;
    int numberOfStrings = 1;
    int lengthOfString = 0;

    // requesting memory for the line
    buffer = malloc(BUFFER_SIZE + 1); //leaving room for null byte added by fgets 
    if(buffer == NULL) 
        {
            printf("\n Space has not been created");
            return -1;
        }


    // Array of strings that will save them
    // This is an array of pointers to characters, not array of characters
    // Array of characters is initialized down there on line 54
    char **strings = malloc(numberOfStrings * sizeof(char *));

    // Getting the file contents 
    // save them in an array of arrays
    while (fgets(buffer, BUFFER_SIZE+1, fp) != NULL)
    {
        lengthOfString = strlen(buffer);
        //fputs(buffer, stdout);
        
        // Take pointer to charaacter at index i and make it point to this block of memory
        strings[numberOfStrings-1] = malloc(lengthOfString * sizeof(char *));

        // Copy string from buffer to this new dynamically allocated space
        strcpy(strings[numberOfStrings-1], buffer);

        numberOfStrings++;
        // dynamically create memory for the array of pointers to strings
        strings = realloc(strings, (numberOfStrings * sizeof(char *)));
    
    }
    
    
    printf(" \n program reaches here...");
    printf("\n\n");

    // sorting the array contents
    qsort(strings, numberOfStrings-1, sizeof(char *), comparator);
    
    printf(" \n program reaches here...");
    printf("\n");

    // Printing resulting strings array
    printf("\n Resulting strings array: \n\n");
    for (int i = 0; i < numberOfStrings; i++)
    {
        printf("strings[%d] = %s\n", i, strings[i]);
    }
    printf("\n");
    
    
    return 0;
}