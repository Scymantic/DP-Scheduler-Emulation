# DP-Scheduler-Emulation


#**OVERVIEW**

**Everything was written in C on a Linux based system (Mac) but I do use some C++ functions.  The biggest thing here is that I have not tested it on a Windows machine (sorry but comitments abound) I may do so in the future.  For now, assume it won't run on a Windows machine.**

The QueueArray.h defines a priority queue structure.

These two (2) programs, commander.cpp and processManager.cpp emulate what a Dynamic Priority Scheduer would look like.

The commander process reads from std in.
The process Manager reads from the pipe that the commander process creates.

I have added two versions of an example txt file holding a bunch of commands.  The cmdrFull.txt will print the state of the machine before and after each and every operation.  Whereas the cmndrSparse.txt only prints the state of the system at certain points.

Note that the commander is currently sleeping for two (2) seconds before writing to the pipe this was a design requirement it can easily be turned off by simply removing or commenting out the 'sleep(2)' lines in the commander.cpp.

**If you don't know how to redirect input:**

**./commander < "filename".txt**

The code is heavily commentated but I will give a cursory explanation here as well:

##**What do the functions do and how are they called?**

###**Function #1: 'S'**   
The 'S' takes three (3) integer arguments ONLY: The process ID, the Value stored in the process & the Total Process Runtime.   It creates a new process (as a PCB object) and reads the required values from the command pipe then it sets all other values to their defaults it then pushes the process into the PCB table and: If there is NOT a process currently running it sets the newly created process as running.  If their IS a process currently running it pushes the current process' PID into the ReadyQueue (i.e. processes ready to execute queue)

###**Function #2: 'B'**   
The 'B' function has one (1) integer argument ONLY: A resource ID. It blocks the currently running process for a given resource.  Note that each Resource Queue is a priority queue of size 4 (i.e. 4 priority levels 0-3) therefore if you have 2 resource queues the resource ID can only be 0-1.  In this case there are three (3) Resource Queues therefore the Resource ID can only be a number from 0-2.  After blocking the currently running process the function moves a process (with highest priority) from the Ready Queue onto the "CPU" (currently running).  If there is nothing in the ready queue it simply says so and returns.

###**Function #3: 'U'**  
The 'U' function has one (1) integer argument ONLY: A resource ID.  It removes a process (with highest priority) from the Blocked State resource queue that matches the given resource ID.  If there is NOT a currently running process it makes the process it just unblocked the currently running process.  If there IS a currently running process it pushes the process it just got onto the ReadyQueue.

_**It is important to note that all of the Queues used are Priority Queues and that the only data that is placed into these queues is the Process ID.  This allows things to remain relatively simple.**_

###**Function #4: 'C'**  
The 'C' function takes two (2) arguments a command (charachter) and a number (integer).  The function applies arithmetic to the value of the currently running process based on the command and number passed to it. 
* _Command 'A':_   Add the number to the current process' value.
* _Command 'S':_   Subtract the number from the current process' value.
* _Command 'M':_   Multiply the current process' value by the number.
* _Command 'D':_   Divide the current process' value by the number.  (Checks for division by 0)
This "takes up" the "CPU" therfore the once all arithmetic is done the 'C' function calls the scheduler function.

###**Function #5: 'Q'**
The 'Q' function takes zero (0) arguments and calls the scheduler helper function.  Please read the comments in the code for how this works its long and complex and I really don't want to have to write it again.  **The best explanation is in the proceeManager.h file the name of the function is scheduler()**

###**Function #6: 'T'**
The 'T' function takes zero (0) arguments.  It calculates the turnaround time for all finished processes prints it alsong with some other info and terminates the system.  **There should only be ONE (1) 'T' function call EVER since anything after the T call will never run. ;)**

###**Function #6: 'P'**
The 'P' function takes zero (0) arguments.  All it does is print the current state of the system.  It's a monster I tried to clean it up and make it as legible as I can but...you'll have to decide for yourselves.



#Problems/Concerns?

There should not be any problems.  Although in order to use functions like atoi() which is used in the processManager you MUST compile with a c++ 11 standard.  The makefile has this already so it should hopefuly work if not you'll simply have to do it by hand.

##**VERY IMPORTANT**
If you decide to do single operations (i.e. not using the txt files):

./commander  **(Note that there are NO arguments! The code checks for this though so you are fine if you forget)**

Then you enter say:  S 1 0 1

And then nothing happens!!  And you will be very tempted to say "What the hell is this I thought it worked?!!!".
**What's happening is that the program is waiting for your next input.  IT WILL ONLY PRINT STUFF OUT IF YOU PASS IT A 'P' COMMAND.**  I kid you not I was debugging a perfectly working program because I thought that had broken when I first noticed this behavior, I lost like 3hrs of my life, it was sad, don't let it hapen to you! :)

##Final Thoughts
I'm fairly certain I've covered everything if I missed something:
* A:  Look at the comments in the code it might be there
* B:  Feel free to PM me or email or whatever and I will be happy to answer questions
