/************************
Assignment 3: System Monitoring Tool(Main File)
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
#include <ctype.h>
#include "functions.h"

int main(int argc, char *argv[])
{
	// Variables used to loop through args
	int x;
	// Holds the sample and delay values
	int samples = 10;
	int tdelay = 1;
	// Counter variable for CLA parsing
	int count = 0;
	// Holds whether any arguments are in argv
	char *s = NULL;
	char *u = NULL;
	char *seq = NULL;
	char *g = NULL;
	// Outputs the entire report stats
	if (argc <= 1)
	{
		char *s = "yes";
		char *u = "yes";
		// Using the controller function to format the output
		controller(&samples, &tdelay, seq, g, u, s);
	}
	// Case where we are given an argument
	else
	{
		// Loops to check which arguments are present
		for (x = 0; x < argc; x++)
		{
			// Checks for system
			if (strcmp(argv[x], "--system") == 0 || strcmp(argv[x], "-s") == 0)
			{
				s = "yes";
			}
			// Checks for user
			if (strcmp(argv[x], "--user") == 0 || strcmp(argv[x], "-u") == 0)
			{
				u = "yes";
			}
			// Checks for sequential
			if (strcmp(argv[x], "--sequential") == 0)
			{
				seq = "yes";
			}
			// Checks for graphical
			if (strcmp(argv[x], "--graphics") == 0 || strcmp(argv[x], "-g") == 0)
			{
				g = "yes";
			}
			// If only a digit is given for samples adn tdelay
			if (atoi(argv[x]))
			{
				if (count == 0)
				{
					// Convert to valid int
					samples = atoi(argv[x]);
					count = count + 1;
					// Check if valid sample amount
					if (samples < 1)
					{
						perror("Invalid Sample amount, too low, must be atleast 1 or greater");
						exit(1);
					}
				}
				else if (count == 1)
				{
					// Convert to valid int
					tdelay = atoi(argv[x]);
					count = count + 1;
					// Check if valid tdelay amount
					if (tdelay < 1)
					{
						perror("Invalid tdelay amount, too low must be 1 or greater");
						exit(1);
					}
				}
			}
			// Checks for samples then extracts the value
			char *ptr = strstr(argv[x], "--samples");
			if (ptr != NULL)
			{
				sscanf(argv[x], "%*[^0123456789]%d", &samples);
				// Check if valid sample amount
				if (samples < 1)
				{
					perror("Invalid Sample amount, too low, must be atleast 1 or greater");
					exit(1);
				}
			}
			// Checks for tdelay then extracts the value
			char *ptr2 = strstr(argv[x], "--tdelay");
			if (ptr2 != NULL)
			{
				sscanf(argv[x], "%*[^0123456789]%d", &tdelay);
				// Check if valid tdelay amount
				if (tdelay < 1)
				{
					perror("Invalid tdelay amount, too low must be 1 or greater");
					exit(1);
				}
			}
		}

		// If neither user and sys are specficed we set both to yes
		if ((u == NULL) && (s == NULL))
		{
			u = "yes";
			s = "yes";
		}

		// Calls the controller and send the CLA parsed
		controller(&samples, &tdelay, seq, g, u, s);
	}
}
