#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include "sem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>





struct statistics {
	int lines;
	int lineHits;
	int files;
	int fileHits;
	int dirs;
	int activeGrepThreads;
	int maxGrepThreads;
	int activeCrawlThreads;
};

// (module-)global variables
static struct statistics stats;
static SEM *semStats;
static SEM *notifySem;
static SEM *grepThreadsSem;


// function declarations
static void* processTree(void* path);
static void processDir(char* path);
static void processEntry(char* path, struct dirent* entry);
static void* processFile(void* path);
// TODO: add declarations if necessary

static void usage(void) {
	fprintf(stderr, "Usage: palim <string> <max-grep-threads> <trees...>\n");
	exit(EXIT_FAILURE);
}

static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/**
 * \brief Initializes necessary data structures and spawns one crawl-Thread per tree.
 * Subsequently, waits passively on statistic updates and on update prints the new statistics.
 * If all threads terminated, it frees all allocated resources and exits/returns.
 */



int main(int argc, char** argv) {
	if(argc < 4) {
		usage();
	}

	// convert argv[2] (<max-grep-threads>) into long with strtol()
	errno = 0;
	char *endptr;
	stats.maxGrepThreads = strtol(argv[2], &endptr, 10);

	// argv[2] can not be converted into long without error
	if(errno != 0 || endptr == argv[2] || *endptr != '\0') {
		usage();
	}

	if(stats.maxGrepThreads <= 0) {
		fprintf(stderr, "max-grep-threads must not be negative or zero\n");
		usage();
	}

	// Datenbstrukturen initialisieren
	semStats = semCreate(1); // since we have single access to  critical section
	notifySem = semCreate(0); // producer-consumer pattern, we start with 0, consumer decreases, producers increases
	grepThreadsSem = semCreate(stats.maxGrepThreads);
	if (semStats = NULL || notifySem == NULL || grepThreadsSem == NULL)
	{
		die("semCreate");
	}
	

	// Crawl threads erzeugen
	pthread_t crawlThread[argc-3];
	for (int i = 0; i < (argc-3); i++)
	{
		if(pthread_create(&crawlThread, NULL, &processTree, argv[i+3]) != 0){
		die("pthread_create");
		}
	}

	// Blocking to wait of the change in statistics
	// creating a deep copy of it, not only referencing it
	struct statistics temp;

	while (1)
	{
		P(notifySem);

		P(semStats);	
		temp.lineHits = stats.lineHits;
		temp.lines = stats.lines;
		temp.fileHits = stats.fileHits;
		temp.files = stats.files;
		temp.dirs = stats.dirs;
		temp.activeCrawlThreads = stats.activeGrepThreads;
		temp.maxGrepThreads = stats.maxGrepThreads;
		temp.activeGrepThreads = stats.activeGrepThreads;
		V(semStats);

		// printing it
		if(printf("\r%i/%i lines, %i/%i files, %i directories, %i active threads", temp.lineHits, temp.lines, temp.fileHits, temp.files, temp.dirs, temp.activeGrepThreads) < 0)
		{
			die("printf");
		}
		// Force a write o all buffered data on the stdout stream.
		// If EOF is returned, an error has occured.
    	if (fflush(stdout) == EOF) die("fflush");

		// getting out of the loop
		if (temp.activeCrawlThreads == 0 && temp.activeGrepThreads == 0)
		{
			break;
		}
		
	}

	// prinmting one last time to get final stats that could have been left behind
	if(printf("\r%i/%i lines, %i/%i files, %i directories, %i active threads", temp.lineHits, temp.lines, temp.fileHits, temp.files, temp.dirs, temp.activeGrepThreads) < 0)
	{
		die("printf");
	}
	// Force a write o all buffered data on the stdout stream.
	// If EOF is returned, an error has occured.
	if (fflush(stdout) == EOF) die("fflush");







	// Aufräumen/ freigeben
	

	return EXIT_SUCCESS;
}

/**
 * \brief Acts as start_routine for crawl-Threads and calls processDir().
 *
 * It updates the stats.activeCrawlThreads field.
 *
 * \param path Path to the directory to process
 *
 * \return Always returns NULL
 */
static void* processTree(void* path) {
	// thread detaching
	int pdet = pthread_detach(pthread_self);
	if (pdet != 0)
	{
		die("pthread_detach");
	}

	// updating statistics
	P(semStats);
	stats.activeCrawlThreads += 1;
	V(semStats);
	V(notifySem);


	// Calling process dir for paths
	processDir(path);
	
	return NULL;
}

/**
 * \brief Iterates over all directory entries of path and calls processEntry()
 * on each entry (except "." and "..").
 *
 * It updates the stats.dirs field.
 *
 * \param path Path to directory to process
 */

static void processDir(char* path) {
	// getting to the entries
	DIR *currentDirectoryPointer = opendir(path);
	errno = 0;
	struct dirent *currentEntry = readdir(currentDirectoryPointer); // readdir is iterator function which returns current entry, if any, if not returns NULL
	// to distinguish between a normal NULL and a critical section null, ie if only ernno is 0 then its only end of iteration otherwise 
	// when errno != 0 but readdir is NULL, then it was a critical error inside readdir

	// updating statistics
	P(semStats);
	stats.dirs += 1;
	V(semStats);
	V(notifySem);

	// iterating as long as current entry isnt NULL 
	while (currentEntry != NULL)
	{
		// process current entry
		// TODO adjust iterator
		errno = 0;
		
		// checking that the directory name isn't "." or ".."
		if(strcmp(".", currentEntry->d_name) != 0 && strcmp("..", currentEntry->d_name) != 0){
			processEntry(path, currentEntry);
		} 
		// resetting errno
		errno = 0;
		currentEntry = readdir(currentEntry);

	}
	// we get outside while loop only when readdir returns null
	// we check if this is a critical error
	if (errno != 0)
	{
		die("readdir");
	}
	
	// closing the directory
	if(closedir(currentDirectoryPointer) == -1){
		die("closedir");
	}
	
}






/**
 * \brief Spawns a new grep-Thread if the entry is a regular file and calls processDir() if the entry
 * is a directory.
 *
 * It updates the stats.activeGrepThreads if necessary. If the maximum number of active grep-Threads is
 * reached the functions waits passively until another grep-Threads can be spawned.
 *
 * \param path Path to the directory of the entry
 * \param entry Pointer to struct dirent as returned by readdir() of the entry
 */
static void processEntry(char* path, struct dirent* entry) {
	// we deep copy the path because the path is modified 
	//get length of the new string
    int entryPathLength = strlen(path) + strlen(entry->d_name) + 2; // +2 to specify the length of the string
    
    //malloc and check for error during malloc
    char* entryPath = malloc(entryPathLength);
    if(entryPath == NULL) die("malloc");

    //snprintf takes in the destination, length, format string (like in print f)  and the arguments
    snprintf(entryPath, entryPathLength, "%s/%s", path, entry->d_name); // concat string and copy them into entrypath


	struct stat buf;
	// getting meta information of the current path
	if (lstat("path", &buf) == -1)
	{
		// couldn't open current entry
		free(entryPath);
		return 0;
	}
	// checking if it is a directory
	if (S_ISDIR(buf.st_mode))
	{
		// recursively call process dir since it is a directory
		processDir(entryPath);
	}
	else if (S_ISREG(buf.st_mode))
	{
		// is a regular file
		// Updating statistics
		P(semStats);
		stats.files += 1;
		V(semStats);
		V(notifySem);

		// creating as new grab threads
		P(grepThreadsSem); // claim the resource since we have a limited space
		pthread_t grepThread;
		if(pthread_create(grepThread, NULL, processFile, entryPath) != 0){
			die("pthread_create");
		}


	}

}







/**
 * \brief Acts as start_routine for grep-Threads and searches all lines of the file for the
 * search pattern.
 *
 * It updates the stats.files, stats.lines, stats.fileHits, stats.lineHits
 * stats.activeGrepThreads fields if necessary.
 *
 * \param path Path to the file to process
 *
 * \return Always returns NULL
 */
static void* processFile(void* path) {
	//TODO: implement me!

	return NULL;
}

