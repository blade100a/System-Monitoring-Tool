### System Monitoring Tool -- Concurrency & Signals
- Harri Pahirathan

### Files:
 - A3.c
 - functions.c
 - functions.h
 - Makefile
 - README.md

### Purpose:
Output the same monitor tool from assignment one but this time we would implement forks to run process and gather information from those process and output our monitor tool which essentially allowing concurrency make performance a bit more optimal. With the use of pipes, we will be able to transfer data from child to parent and vice versa if needed. We will also be implmenting some error checking as well as having signals for Ctrl+C and Ctrl+Z.

### How I solved and created my assignment:
- I had the main function do all the argument parsing which will determine exactly what the user is wanting from the program
- Memory usage, Cpu usage and User/Session have their own respective functions which will allow to have each function to be a process and what these functions will pipe the data that normally would print to the parent instead
- I decided to put all my functions in a h file and prototype to optimaize performance as well as to make the files look cleaner
- I had one function(controller) dedicating to being the main hub for the forking and piping called the controller which will create process to each function and then gather the data from the pipes, which it will interpet and print. Reason behind this will allow data manipulation to be easier and a dedicated function for this would be useful for debugging purposes.
- The main function after parsing all the commends will send all the commands that are needed as well as those that are not needed to the controller
- The controller will figure out in creating 1 or 2 or all 3 process as, if we need only systems report this would mean we need 2 process(memory and cpu usage) while user report we only need users/sessions. If we need both the controller will interpet that to make 3 process. 
- The idea from each function is to send data to the pipes which will place the data into their respective buffers
- To have concurrency I allow all proccess to be run in which we wait for all information to then proceed on manipulating and outputing the data. When each process is going we have multiple computations which is the basic definition of concurrency to then be piped to the parent and used for our output.

### Overview of Functions:
- void format(int *num);
	- This function is dedicated in producing the headers for each function. The int pointer num tells the function the exact header it wants depending on the value. In this case we use 0, 1, 2. 0 being Users/sessions, 1 being Number of cores, 2 being the Memory usage header.
- void controller(int *samples, int *tdelay, char *s, char *g, char *us, char *sys);
	- This function is dedicated to performing forks, pipes, and navigate the proccess data in which it will also print onto the stdout the nessesscery parts of the program that have been given or defaulted. In the idea of forking we dedicated 3 forks to 3 functions(mem, user, cpu), which is are only given --system then we run 2 process(mem and cpu), given --user then we run 1 process(user), and if we ae given both or default we run all three process. Each process is linked to only one parent in which it will wait for process completation to then read from the pipe and store into a buffer designed specifcally for that process. After obtaining the needed data, we will check what is needed to be displayed and print onto stdout. We also error check if the pipes failed to be made as well as error check if the forks fail to create the process. The parameters of this function take whats given from our CLA, to determine the exact output of our program. This will also be where we take signals for ctrl z and ctrl c, in which we ignore calls in the process but have a signal call above the loop thus taking any signal call.
- void header(int *samples, int *tdelay);
	- This function will shows us the number of samples and tdelay the program will run depending on the specfics given. Used to print the very start of the program.
- void userUsage(int fd);
	- This function will take in the fd in which the data will be piped in. To find the active users and sessions we use the utmp struct in which getutent should show us all users as well as the session there in. We will have one big buffer with ` in between each user which will then be writen into the fd for our parent to read.
- void sysUsage();
	- This function will display the systems current specs. Using the strcut utsname, we can find all nessesscery information and display them at the very end of the program completion, as this data is never changing while the main data maybe doing iterations, refreshing etc, once those are completed this is then printed.
- void cpuCore(int fd, char *g, int *tdelay);
	- This function is dedicated in producing the cpu usage as well as graphics if the program defines graphics as needed. We use the formula given on the A3 handout and where we find our values are in the /proc/stat file, in which we also error check in case we didnt access the file properly. We use tdelay in between i = 1 and then perform the same thing for i=2 as the data needs time to obtain new values to properly calculate cpu usage. For graphics each cpu usage will start with three "|" in which for each extra 1% we will add "|" onto the three already and then display that cpu usage percentage at the end. Similar to user, we use ` to seperate our usage and graphics for our parent to know which is which. We also capped the graphics | to 100 characters.
- void memoryUsage(int fd, char *g, int *x, float *oldMem);
	- This function is dedicated in producing the memory usage as well as graphics. It has extra paramters, int *x, is used to know what iteration we are at in the program, more so used for graphics to understand the deafult graphics at the beginning. We have float *oldMem, which has the last memory usage which will take the difference from the current so they can display the appropriate graphic. This function use struct sysinfo in determining our total ram, total swap, etc, and we multiple the values by 1024^3 to get GB. Similarly to eveything else we write into an fd which the parent will read and store into the buffer. For the graphics we capped the # and : to 50 as it will be to cluttered with alot.
- void catch_signal(int signo)
	- This functions is dedicated in taking in signals mainly ctrlz and ctrlc. The idea is for ctrl+c is to ask the user if they would quit, if so we exit as well as clear the console, if not we ignore and continues with the program, and if we dont get "yes" or "no" or the variations we have perror as well as the same ask until a proper valid response is given. Wtih ctrl+z we ignore the call in the controller function so we dont have to worry about it in the handler. We have to type Yes yes y Y or no No n N for a valid response for the handler to allow access or quit.
	
### How to Run: 
- One way would be to run "make" using the Makefile, then using ./myprog, should give you everything with samples of 10 and 1 second, and by adding more arguments, you can specfiy exactly what you want, which these commands are: (--system or -s), (--user or -u), --sequential, (--graphics or -g), (--samples=# or # that is the first arguemnt), and (--tdelay=# or # that is the second argument). The use make clean to remove the create myprog and object files created.
- The other would be a straight gcc line on the console if you dont want to use the makeFile


Date: 2023-04-11
