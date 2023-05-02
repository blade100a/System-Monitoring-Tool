/************************
Assignment 3: System Monitoring Tool(Functions)
Name: Harri Pahirathan
Student ID: 1003371549
Class: CSCB09-Winter 2023
Date: 2023-04-07
************************/

// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <utmp.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include "functions.h"

// Function depending on given paramter to print certain headers
void format(int *num)
{
	if (*num == 0)
	{
		// Starts a fresh line
		printf("\33[2K\r---------------------------------------\n");
		// Prints title for user
		printf("\33[2K\r### Sessions/users ###\n");
	}
	else if (*num == 1)
	{
		// Starts a fresh line
		printf("\33[2K\r---------------------------------------\n");
		// Finds the number of avaliable cores in the system
		printf("\33[2K\rNumber of cores: %d\n", get_nprocs());
	}
	else
	{
		// Starts a fresh line
		printf("\33[2K\r---------------------------------------\n");
		// Prints the memory title line
		printf("\33[2K\r### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
	}
}

// Functions that will control the program, forks and prints all given info
void controller(int *samples, int *tdelay, char *s, char *g, char *us, char *sys)
{
	// Clear screen
	printf("\033[0;0H");
	printf("\033[2J");
	// Variables
	int fdm[2], fdu[2], fdc[2];
	// The three process
	pid_t memPid, userPid, cpuPid;
	// Holds the lines obtained from process
	char buffer1[500];
	char buffer2[500];
	char buffer3[500];
	// Variable for looping
	int x;
	// For the wait of our process
	int status;
	// Formatter variables
	int num;
	// Holds the number of users printed
	int numUser = 0;
	// Used to calculate difference in memory
	float prevMem = 0.0000;
	// Used to hold output given from buffer
	char *gCpu[*samples];
	char *arr1[20];
	char *arr2[2];

	// Error check for all fds if pipe fails
	if (pipe(fdm) == -1 || pipe(fdu) == -1 || pipe(fdc) == -1)
	{
		perror("Pipe failed");
		exit(1);
	}
	// Catching signals and error checking
	if (signal(SIGINT, catch_signal) == SIG_ERR || signal(SIGTSTP, SIG_IGN) == SIG_ERR)
	{
		perror("Error obtaining singal for signal handler");
		exit(1);
	}
	// Loop through the number of iterations thatll be done
	for (x = 0; x < *samples; x++)
	{
		// Run both memory and cpu if needed
		if (!(sys == NULL))
		{
			// Creates Memory usage process
			memPid = fork();

			// Creates the Cpu usage process
			cpuPid = fork();

			if (memPid == 0)
			{
				// Ignore signal call in child
				if (signal(SIGINT, SIG_IGN) == SIG_ERR)
				{
					perror("Error ignoring signal");
				}
				// Memory pid Process
				// Before writing must close fd0
				close(fdm[0]);
				memoryUsage(fdm[1], g, &x, &prevMem);
				// Before reading must close fd1
				close(fdm[1]);
				// Finish the process
				exit(0);
			}
			else if (cpuPid == 0)
			{
				// Ignore signal call in child
				if (signal(SIGINT, SIG_IGN) == SIG_ERR)
				{
					perror("Error ignoring signal");
				}
				// Cpu pid Process
				// Before writing must close fd0
				close(fdc[0]);
				cpuCore(fdc[1], g, tdelay);
				// Before reading must close fd1
				close(fdc[1]);
				// Finish the process
				exit(0);
			}
			else if (memPid < 0 || cpuPid < 0)
			{
				// Failed to create Memory usage process
				perror("Error Creating Process(Mem or CPU)");
				exit(1);
			}

			// Wait for the proccesses information from each process
			waitpid(memPid, &status, 0);
			waitpid(cpuPid, &status, 0);

			// Reads from the pipe from the child processes
			read(fdm[0], buffer1, sizeof(buffer1));
			read(fdc[0], buffer3, sizeof(buffer3));
			// Store the first memory usage
			char oldmem[6];
			strncpy(oldmem, buffer1, 4);
			prevMem = atof(oldmem);
			// Seperates the cpu and graphics by `
			char *ptr2 = strtok(buffer3, "`");
			// Assigns the cpu usage line
			arr2[0] = ptr2;
			// Moves to next part of pointer
			ptr2 = strtok(NULL, "`");
			// Assigns the cpu graphics
			arr2[1] = ptr2;
		}
		// Checks if we need user process
		if (!(us == NULL))
		{
			// Creates User usage process
			userPid = fork();
			if (userPid == 0)
			{
				// Ignore signal call in child
				if (signal(SIGINT, SIG_IGN) == SIG_ERR)
				{
					perror("Error ignoring signal");
				}
				// User pid Process
				// Before writing must close fd0
				close(fdu[0]);
				userUsage(fdu[1]);
				// Before reading must close fd1
				close(fdu[1]);
				// Finish the process
				exit(0);
			}
			else if (userPid < 0)
			{
				// Failed to create User and sessions process
				perror("Error Creating User Process");
				exit(1);
			}
			// Wait from the pipe from user process
			waitpid(userPid, &status, 0);
			// Reads from the pipe of user process
			read(fdu[0], buffer2, sizeof(buffer2));
			// Seperates string by `
			char *ptr1 = strtok(buffer2, "`");
			// Will hold the users and session
			int y = 0;
			//  loop through user buffer and print each user on new lines
			while (ptr1 != NULL)
			{
				arr1[y] = ptr1;
				// Move to the next part that is sep by `
				ptr1 = strtok(NULL, "`");
				y += 1;
			}
			// Holds the number of users fo next iterations
			numUser = y;
		}
		// Now that we have our information from one iteration we print
		// Depending on the arguments we have
		// If we have sequential will print header for each iteration
		if (!(s == NULL))
		{
			// Format the top part of the output
			printf("iteration %d\n", x);
			header(samples, tdelay);
		}
		// else we are dealing with refreshing header
		else
		{
			printf("\033[s");
			// Format the top part
			header(samples, tdelay);
		}

		// Starts with memory usage printer check
		if (!(sys == NULL))
		{
			// Format of the title of memory usage
			num = 2;
			format(&num);
			// Loop and print \n for the number of samples that will be printed
			int t;
			if (!(s == NULL))
			{
				// Memory lines that are blank
				for (t = 0; t < *samples; t++)
				{
					printf("\n");
				}
				// Places cursor at the top for first iteration
				if (x == 0)
				{
					printf("\033[%dA", *samples + 1);
				}
				// Places cursor one below the first iteration
				else
				{
					printf("\033[%dA", *samples);
				}
			}
			else
			{
				// For first iteration we place all the empty new lines
				if (x == 0)
				{
					for (t = 0; t < *samples - 2; t++)
					{
						printf("\n");
					}
					// Move cursor back up once we have printed all newlines
					printf("\033[%dA", *samples - 1);
				}
			}
			// Moves down to the certain position each iteration will be
			printf("\033[%dB", x);
			// Prints the memory usage
			printf("%s\n", buffer1);
			// If its the last iteration we wont move down to the end
			if (x < *samples - 1)
			{
				printf("\033[%dB", *samples - x - 1);
			}
		}
		// Next is the user/session print if needed
		if (!(us == NULL))
		{
			// Format of the title of user/sessions
			num = 0;
			format(&num);
			// Prints each user/session found
			int pos = 0;
			for (pos = 0; pos < numUser; pos++)
			{
				printf("%s\n", arr1[pos]);
			}
			// If its only systems report then we have our delay here
			if (sys == NULL)
			{
				sleep(*tdelay);
			}
		}
		// If we need our Cpu usage report
		if (!(sys == NULL))
		{
			// Format of the title of CPU usage
			num = 1;
			format(&num);
			// Prints the cpu usage %
			printf("%s\n", arr2[0]);
			// If we need graphics for cpu usage
			if (!(g == NULL))
			{
				int pos2 = 0;
				// Allocate memory for graphic cpu usage and copy from given data
				gCpu[x] = malloc(sizeof(char) * (strlen(arr2[1]) + 1));
				strcpy(gCpu[x], arr2[1]);
				// Print each graphic cpu usage obtained so far
				for (pos2 = 0; pos2 < (x + 1); pos2++)
				{
					printf("%s\n", gCpu[pos2]);
				}
			}
		}
		// If its not sequential we need to return to the top of the console
		if (s == NULL && x < *samples - 1)
		{
			printf("\033[u");
			fflush(stdout);
		}
		// Resets buffers to not have any unwanted characters
		memset(buffer1, 0, sizeof(buffer1));
		memset(buffer2, 0, sizeof(buffer2));
		memset(buffer3, 0, sizeof(buffer3));
	}

	// Add System Information at the end
	sysUsage();
	// Frees any memory we allocated
	if (!(g == NULL) && !(sys == NULL))
	{
		int p;
		for (p = 0; p < *samples; p++)
		{
			free(gCpu[p]);
		}
	}
}

// Functions for Header for Samples and Memory usage
void header(int *samples, int *tdelay)
{
	printf("\33[2K\rNbr of samples: %d -- every %d secs\n", *samples, *tdelay);
	// Using the resource lib we use rusage to find max mem usage
	struct rusage mem;
	// Obtains data which has the memory usage
	getrusage(RUSAGE_SELF, &mem);
	// Prints the amount of memory being using from getrusage function
	printf("\33[2K\r Memory usage: %ld kilobytes\n", mem.ru_maxrss);
}

// Function dedicated providing the user usage
void userUsage(int fd)
{
	// To hold all lines of user/sessions seperated by `
	char usline[350];
	char uslines[400];
	// Resets buffer to be used for cat
	memset(uslines, 0, sizeof(uslines));
	char userName[UT_NAMESIZE + 1];
	// Used to find the current users and sessions
	struct utmp *user;
	// Resets to beginning
	setutent();
	// Loops through each active user and session
	while ((user = getutent()) != NULL)
	{
		if (user->ut_type == USER_PROCESS)
		{
			// Adds each line into uslines seperated by end char `
			sprintf(usline, "\33[2K\r %s       %s (%s)`", user->ut_user, user->ut_line, user->ut_host);
			strcat(uslines, usline);
		}
	}
	// Closes the utent
	endutent();
	// Write to parent and close
	write(fd, uslines, strlen(uslines) + 1);
	close(fd);
}

// Functions dedicated in providing the system information
void sysUsage()
{
	// Starts a fresh line
	printf("\33[2K\r---------------------------------------\n");
	printf("\33[2K\r### System Information ###\n");
	// Using utsname lib we'll find all the information needed
	struct utsname sysInfo;
	// If we have trouble accessing sysinfo
	if (uname(&sysInfo) == -1)
	{
		perror("uname error getting sysInfo");
		exit(1);
	}
	else
	{
		// Prints each line of information needed
		printf("\33[2K\r System Name = %s\n", sysInfo.sysname);
		printf("\33[2K\r Machine Name = %s\n", sysInfo.nodename);
		printf("\33[2K\r Version = %s\n", sysInfo.version);
		printf("\33[2K\r Release = %s\n", sysInfo.release);
		printf("\33[2K\r Architecture = %s\n", sysInfo.machine);
	}
	printf("\33[2K\r---------------------------------------\n");
}

// Functions dedicating in providing the cpu usage as well as graphics is needed
void cpuCore(int fd, char *g, int *tdelay)
{
	// lines and graphic line for parent
	char cpuline[350];
	char gline[150];
	// Variables to calculate CPU utilization
	int stat1[7];
	int stat2[7];
	int Ti1 = 0;
	int Ti2 = 0;
	int Ul1 = 0;
	int Ul2 = 0;
	float cpuUsage = 0.0;

	// open proc/stat where we scan to get all the variables
	FILE *fp;
	// Read proc/stat for information of the cpu to calculate
	fp = fopen("/proc/stat", "r");
	// Catch the error if problem with opening proc/stat
	if (fp == NULL)
	{
		perror("Error opening /proc/stat");
		exit(1);
	}
	// scan the first line for every variable needed except first
	fscanf(fp, "%*s %d %d %d %d %d %d %d", &stat1[0], &stat1[1], &stat1[2], &stat1[3], &stat1[4], &stat1[5], &stat1[6]);
	sleep(*tdelay);
	rewind(fp);
	// scan the first line once again for our second stat pointer
	fscanf(fp, "%*s %d %d %d %d %d %d %d", &stat2[0], &stat2[1], &stat2[2], &stat2[3], &stat2[4], &stat2[5], &stat2[6]);
	fclose(fp);
	// Using the math formula given in the assignment to get cpu Usage
	Ti1 = stat1[0] + stat1[1] + stat1[2] + stat1[3] + stat1[4] + stat1[5] + stat1[6];
	Ti2 = stat2[0] + stat2[1] + stat2[2] + stat2[3] + stat2[4] + stat2[5] + stat2[6];
	Ul1 = Ti1 - stat1[3];
	Ul2 = Ti2 - stat2[3];
	cpuUsage = (float)((float)(Ul2 - Ul1) / (Ti2 - Ti1)) * 100;
	// Add the line of cpu usage to string
	sprintf(cpuline, "\33[2K\r total cpu use = %.2f%%", cpuUsage);
	// If graphics is needed we also add that line
	if (!(g == NULL))
	{
		sprintf(gline, "`\33[2K\r	 |||%.*s %.2f", (int)cpuUsage, "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||", cpuUsage);
		// Add both the cpu usage and graphic line together seperated by `
		strcat(cpuline, gline);
	}
	write(fd, cpuline, strlen(cpuline) + 1);
	close(fd);
}

// Functions dedicated to provding memory usuage relative to number of samples and tdelay
void memoryUsage(int fd, char *g, int *x, float *oldMem)
{
	// Buffers to hold for both graphics and mem line
	char memline[350];
	char grapline[100];
	// Using sysinfo to find the information on mem usage
	struct sysinfo ram;
	// Used to compare difference for graphics
	float old = *oldMem;
	// Call to sysinfo to obtain information
	sysinfo(&ram);
	// get physical total and convet to gb
	float ptot = (float)(ram.totalram / (1024 * 1024 * 1024));
	// Get virtual total and convert to gb
	float vtot = (float)((ram.totalram + ram.totalswap) / (1024 * 1024 * 1024));
	// Get physical used and convert to gb
	float pused = (float)((ram.totalram - ram.freeram) / (1024 * 1024 * 1024));
	// Get virtual used and convert to gb
	float vused = (float)((ram.totalram + ram.totalswap - (ram.freeram + ram.freeswap)) / (1024 * 1024 * 1024));
	// Save mem line into buffer
	sprintf(memline, "%.2f GB / %.2f GB  -- %.2f GB / %.2f GB   ", pused, ptot, vused, vtot);
	// If we need graphics
	if (!(g == NULL))
	{
		// Checks if its first iteration
		if (*x == 0)
		{
			sprintf(grapline, "|o 0.00 (%.2f)", pused);
		}
		else
		{
			// if old usage larger we need # for difference
			if (old < pused)
			{
				old = pused - old;
				sprintf(grapline, "|%.*s* %.2f (%.2f)", (int)(old * 100), "##################################################", old, pused);
			}
			// If old usage is less, we need : for difference
			else
			{
				old = old - pused;
				sprintf(grapline, "|%.*s* %.2f (%.2f)", (int)(old * 100), "::::::::::::::::::::::::::::::::::::::::::::::::::", old, pused);
			}
		}
		// combine both graphics and mem usage line
		strcat(memline, grapline);
	}
	// Write to parent and close
	write(fd, memline, strlen(memline) + 1);
	close(fd);
}

// This function will be used if a signal ctrl+z or ctrl+c is found
void catch_signal(int signo)
{
	char res[10];
	// Check what the user wants to do
	printf("\33[2K\rHi User would you like to quit the program?(yes/no):");
	scanf("%s", res);
	// Exits the program
	if (strcmp(res, "yes") == 0 || strcmp(res, "Yes") == 0 || strcmp(res, "y") == 0 || strcmp(res, "Y") == 0)
	{
		// We clear the screen
		printf("\033[0;0H");
		printf("\033[2J");
		// Successfully exited the program
		printf("\33[2K\rSUCCESSFUL EXIT\n");
		exit(0);
	}
	// If given no we delete the last line
	else if (strcmp(res, "no") == 0 || strcmp(res, "No") == 0 || strcmp(res, "n") == 0 || strcmp(res, "N") == 0)
	{
		// Moves up so the program can continue writing from the top
		printf("\033[%dA\r", 1);
	}
	// Error if given something else
	else
	{
		// Error check if given something else
		perror("INVALID response, try Yes yes y Y or no No n N");
		// Goes back to the top for the ask again
		printf("\033[%dA\r", 2);
		// Recursive call until valid input
		catch_signal(2);
	}
}
