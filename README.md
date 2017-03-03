# DP-Scheduler-Emulation


#**OVERVIEW**

These two (2) programs, commander.cpp and processManager.cpp emulate what a Dynamic Priority Scheduer would look like.

The commander process reads from std in.
The process Manager reads from the pipe that the commander process creates.

I have added two versions of an example txt file holding a bunch of commands.  The cmdrFull.txt will print the state of the machine before and after each and every operation.  Whereas the cmndrSparse.txt only prints the state of the system at certain points.

Note that the commander is currently sleeping for two (2) seconds before writing to the pipe this was a design requirement it can easily be turned off by simply removing or commenting out the 'sleep(2)' lines in the commander.cpp.

The code is heavily commentated but I will give a cursory explanation here as well:

**What do the fcuntions do and how are they called?**

Function #1: 'S'   The 'S' takes three (3) parameters ONLY: The process ID, the Value stored in the process & the Total Process Runtime.   It creates a new process (as a PCB object) and reads the required values from the command pipe then it sets all other values to their defaults it then pushes the process into the PCB table and: If there is NOT a process currently running it sets the newly created process as running.  If their IS a process currently running it pushes the current process' PID into the ReadyQueue (i.e. processes ready to execute queue)

Function #
