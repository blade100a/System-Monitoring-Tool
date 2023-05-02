/************************
Assignment 3: System Monitoring Tool(Prototyping/Header)
Name: Harri Pahirathan
Student ID: 1003371549
Class: CSCB09-Winter 2023
Date: 2023-04-07
************************/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void format(int *num);                                                             // Prototype the format function
void controller(int *samples, int *tdelay, char *s, char *g, char *us, char *sys); // Prototypes the controller function
void header(int *samples, int *tdelay);                                            // Prototypes the header function
void userUsage(int fd);                                                            // Prototypes the user and sessions function
void sysUsage();                                                                   // Prototypes the system information function
void cpuCore(int fd, char *g, int *tdelay);                                        // Prototypes the cpu usage function
void memoryUsage(int fd, char *g, int *x, float *oldMem);                          // Prototypes the memory usage function
void catch_signal(int signo);                                                      // Prototypes the catch signal function

#endif
