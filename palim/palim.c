#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include "sem.h"

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
	if (semStats = NULL || notifySem == NULL)
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
	while (1)
	{
		P(notifySem);

		P(semStats);	
		// creating a deep copy of it, not only referencing it
		struct statistics temp;
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
	int pdet = pthread_detach(pthread_self);
	if (pdet != 0)
	{
		die("pthread_detach");
	}

	//
	
	
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
	// TODO: implement me!

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
	//TODO: implement me!

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

