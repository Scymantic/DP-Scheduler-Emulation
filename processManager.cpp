

// Adrian Barberis
// Process Manager

// Last Edit: Wed. FEB 22, 2017


/*----------------------------- PREPROCESSORS -----------------------------*/


#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include "ProcessManager.h"



#define READ_END 0
#define WRITE_END 1

#define RESOURCE_1 0
#define RESOURCE_2 1
#define RESOURCE_3 2




/*--------------------------- GLOBALS ----------------------------------*/


//Number of processes that have finished
int numFinished = 0;

//Total time_ taken by all finished processes
int finishedTime = 0;

//Total number of processes
int numProcesses = 0;





/*--------------------------- BEGIN MAIN ----------------------------------*/

// A return value of 0 == All is Well 

// A return value of 1 == something whent wrong 




int main(int argc, char** argv)
{

	//pipes
	int cmdp[2];

	//convert argv to commander pipe values
	cmdp[READ_END] = atoi(argv[1]);
	cmdp[WRITE_END] = atoi(argv[2]);

  	//used to read in function call from pipe
	char function;

  	//used for error check
	int status;

  	//Stuff
	Time time_;
	PCBTable pcbt;
	BlockedState bstate;
	ReadyState restate;
	RunningState rustate;

	//  Give initital values to the parameters in the various structs
	initialize(&time_, &rustate, &restate, &bstate);


  	//Close unused pipe ends
	close(cmdp[WRITE_END]);


	//  Begin read loop
	while(true)
	{

		//read in which function to execute
		status  = read(cmdp[READ_END], &function, sizeof(char));

  		//check for failure or eof
		if(status == 0 || status == -1)
		{
			if(status == 0)
			{
				printf("PRMAN: Reached End of File.\nExiting...\n");
				return 0;
			}
			else
			{
				printf("PRMAN: Read Error!\nExiting...\n");
				return 1;
			}
		}

		//  Do stuff based on the char read into 'funtion'
		switch(function)
		{

			case 'S':  // Make a new process and add it to the PCB table
			{
				start(cmdp, &pcbt, &rustate, &restate, &time_);
				break;
			}
			case 'B':
			{
				//  Read in the resource ID that you want to block for
				int rid = 0;
				read(cmdp[READ_END], &rid, sizeof(int));


				//  Set the _resourceID value in the BlockedState struct equal to, 
				// the rid you read in
				bstate._resourceID = rid;


				//  Block
				block(&rustate, &pcbt, &bstate, &restate, rid);
				break;
			}
			case 'U':
			{
				//  Read in the resource ID that you want to unblock for
				int rid = 0;
				read(cmdp[READ_END], &rid, sizeof(int));


				// Unblock
				unblock(&bstate, &restate, &pcbt, &rustate, rid);
				break;
			}
			case 'C':
			{
				//  Change the value of the current process based on a specific command
				calculate(cmdp, &rustate, &pcbt, &restate, &time_);
				break;
			}
			case 'P':
			{	
				//  This method forks a child and the child calls the reporter() method
				//  Pretty standard stuff here
				int p = fork();
				if( p > 0)
				{
					//  Wait for the child to finish
					wait(NULL);
				}
				else if( p == 0)
				{
					reporter(&pcbt, &time_, &rustate, &bstate, &restate);
					return 0;
				}
				else
				{
					std::cerr << "Fork Failed\n" << std::endl;
					return 1;
				}

				break;
			}
			case 'Q':
			{
				//  Run the Scheduler
				scheduler(&time_, &rustate, &pcbt, &restate);
				break;
			}
			case 'T':
			{
				//  Calculate the AVERAGE turnaround time_
				turnaround(&pcbt);
				return 0;
			}

		}

	}

	//  close the pipe read end and quit
	close(cmdp[READ_END]);
	return 0;
}


/* END MAIN */

















/* ------------------------------------- FUNCTION DEFINITIONS -------------------------------------*/







/*  START  */

void start(int* cmdp, PCBTable *pcbt, RunningState *rustate, ReadyState *restate, Time *time_)
{
	PCB process;

	//  There are only 3 parameters for the S function ergo read only 3 times
	for(int i=0; i<3; i++)
	{
		if(i == 0)
		{ 
			//  The first parameter read is the PID
			int rs = read(cmdp[READ_END], &process._pid, sizeof(int));

			if(rs < 0)
			{
				fprintf(stderr, "ERROR: Start process read error!\n");
				return;
			}
		}
		else if(i == 1)
		{ 
			//  The second parameter read is the VALUE
			int rs = read(cmdp[READ_END], &process._value, sizeof(int)); 

			if(rs < 0)
			{
				fprintf(stderr, "ERROR: Start process read error!\n");
				return;
			}
		}
		else if(i == 2)
		{
			// The last parameter read is the RUN TIME
			int rs = read(cmdp[READ_END], &process._run_time, sizeof(int));

			if(rs < 0)
			{
				fprintf(stderr, "ERROR: Start process read error!\n");
				return;
			}
		}
	}


	//  Initialize the rest of the PCB
	process._start_time = time_->_time;
	process._quantum = 1;
	process._qcounter = 0;
	process._priority = 0;
	process._cpu_time = 0;
	process._turnaround = 0;



	//  push the newly made process onto the PCB Table
	pcbt->_pcb_table.push_back(process);


	//  If there is no running process make the, 
	//      newly created process the running process
	if(!rustate->_hasrunning)
	{
		rustate->_process = process;
		rustate->_hasrunning = true;
	}
	else
	{
		//  If a running process exists,
		// add the newly created process to the ready queue
		restate->_queue->Enqueue(process._pid, process._priority);
	}

	//  Increase the total process count by 1
	numProcesses++;
}


//==================================================================================================||









/*  CALCULATE  */

void calculate(int* cmdp, RunningState *rustate, PCBTable *pcbt, ReadyState *restate, Time *time_)
{
	//  The C method has only 2 parameters a Char which is the command to be executed and, 
	//  A number which is the value to be used in the command that we need to execute

	//  Read in only 2 parameters.  Read the first as sizeof(char) and the second as sizeof(int)
	char cmd;
	int  num;
	for(int i=0; i<2; i++)
	{		
		if(i == 0){ read(cmdp[READ_END], &cmd, sizeof(char)); }
		else if(i == 1){ read(cmdp[READ_END], &num, sizeof(int)); }
	}

	//  Switch based on the command (cmd) read in and do the appropriate arithmetic operation

	switch(cmd)
	{
		case 'A':
		{	
			//  Add Num to the current process' _value, then store it back into _value
			rustate->_process._value += num;
			break;
		}
		case 'S':
		{
			//  Subtract Num from the current process' _value, then store it back into _value
			rustate->_process._value -= num;
			break;
		}
		case 'M':
		{
			//  Multiply the current process' _value by Num, then store it back into _value
			rustate->_process._value *= num;
			break;
		}
		case 'D':
		{
			if(num == 0)
			{
				//  If there is a division by zero error say so and continue without changing the value
				//  this could also become a return statement rather than a break.
				printf("PRMAN: Divide by Zero Error!\n");
				break;
			}

			//  Divide the current process' _value by Num, then store it back into _value
			rustate->_process._value /= num;
			break;
		}
	}


	//  Update the PCB Table
	pcbt->_pcb_table[rustate->_process._pid-1] = rustate->_process;

	//  Make the Sheduler do some work since you "Used the CPU"
	scheduler(time_, rustate, pcbt, restate);

}


//==================================================================================================||










/*   BLOCK   */

void block(RunningState *rustate,  PCBTable *pcbt, BlockedState *bstate, ReadyState *restate, int rid)
{
	//  Need to differentiate between wether there is a a currently running process or not
	if(rustate->_hasrunning)
	{
		//  If there is a process currently running:

		//  Store the current PID for later use
		int cpid = rustate->_process._pid;

		//  Get the rid you want to block for
		//int rid = bstate->_resourceID;

		//  IMPORTANT NOTE:  The RunningState struct stores a full on PCB object not just a PID
		//  	This I beleive makes it easier to get accurate information without having to
		//      continually access the PCB Table.  Note to that I don't store multiple copies,
		//		which would bloat things instead I pull a PCB from the PCB Table and set the RunningState,
		//      PCB equal to that.  I then do the reverse when I want to update the PCB Table
		//      Thus I have a relatively constant amount of PCB objects.

		//  Zero out the current process' quantum counter
		//  Decrease the current process' priority by 1;
		rustate->_process._qcounter = 0;
		rustate->_process._priority -= 1;

		//  If the current process' priority is  < 0 set it to 0
		if(rustate->_process._priority < 0){ rustate->_process._priority = 0; }

		//Decrease the quantum
		rustate->_process._quantum = pow(2, rustate->_process._priority);
		if(rustate->_process._quantum < 1){ rustate->_process._quantum = 1;}

		//  //Update the PCB Table
		pcbt->_pcb_table[cpid-1] = rustate->_process;

		//  Based on the resource ID enqueue the current process PID onto,
		//   the appropriate resource queue.  The location of the Enqueue() is given,
		//   by the current process' priority.
		//    E.G.:  priority 0  =  the queue at resource 1 array position 0
		if(rid == RESOURCE_1)
		{
			bstate->_resource1->Enqueue(cpid, rustate->_process._priority);
		}
		else if( rid == RESOURCE_2)
		{
			bstate->_resource2->Enqueue(cpid, rustate->_process._priority);
		}
		else if(rid == RESOURCE_3)
		{
			bstate->_resource3->Enqueue(cpid, rustate->_process._priority);
		}

		//  Once youve taken something off of the CPU you need to put something back on 
		//  If the ready queue has something in it:
		//  1. Dequeue to get the PID
		//  2. Access the PCB Table at location [PID - 1] (Array/vector indecies start at 0)
		//  3. Set the RunningState PCB object equal to the PCB object at PCB Table location [PID - 1]
		if(!restate->_queue->isEmpty())
		{
			rustate->_process = pcbt->_pcb_table[(restate->_queue->Dequeue())-1];
		}
		else
		{
			//  If there is nothing in the ready queue:
  			//	Zero out the RunningState
			//rustate->_hasrunning = false;
		}
	}
	else
	{
		//  If there is no process currently running then you can't actually block anything
		fprintf(stderr, "No Running Process; Nothing to Block!\n");
		return;
	}
}


//==================================================================================================||











/*   UNBLOCK   */

void unblock(BlockedState *bstate, ReadyState *restate, PCBTable *pcbt, RunningState *rustate, int rid)
{
	// Need to differentiate between wether there is a a currently running process or not
	if(rustate->_hasrunning == true)
	{
		//  If there is a process currently running:

		//  Move a process from the BlockedState queue to the ReadyState queue based,
		//  on the resource ID that you read in
		//  Note that we also check to make sure that there actually IS something in the BlockedState queue
		//  For us to unblock
		//  We enqueue at location equal to the dequeued PID's priority
		//  Location of the PID's info in the PCB Table is at [PID -1] (array/vector indicies start at 0)
		if(rid == RESOURCE_1 && !bstate->_resource1->isEmpty())
		{
			int pid = bstate->_resource1->Dequeue();
			restate->_queue->Enqueue(pid, pcbt->_pcb_table[pid-1]._priority);
		}
		else if(rid == RESOURCE_2 && !bstate->_resource2->isEmpty())
		{
			int pid = bstate->_resource2->Dequeue();
			restate->_queue->Enqueue(pid, pcbt->_pcb_table[pid-1]._priority);
		}
		else if(rid == RESOURCE_3 && !bstate->_resource3->isEmpty())
		{
			int pid = bstate->_resource3->Dequeue();
			restate->_queue->Enqueue(pid, pcbt->_pcb_table[pid-1]._priority);
		}
		else
		{
			//  If the BlockedState is empty say so and return
			fprintf(stderr, "A: Block State is empty; There is nothing to unblock!\n");
			return;
		}

	}
	else if(rustate->_hasrunning == false) 
	{
		//  If there is nothing running rather than move the unblocked process to the ReadyQueue,
		//  we put it directly onto the CPU

		//  Move a process from the BlockedState to the CPU:
		//	1. Make sure to check that the resource from which you are trying to unblock is NOT empty
		//  2. Get the PID by dequeueing from the required resource queue
		//  3. Set the currently running process PCB equal to the PCB at location [PID-1] in the PCB Table
		//  4. Set the running state flag to true
		if(rid == RESOURCE_1 && !bstate->_resource1->isEmpty())
		{
			int pid = bstate->_resource1->Dequeue();
			rustate->_process = pcbt->_pcb_table[pid-1];
			rustate->_hasrunning = true;

		}
		else if(rid == RESOURCE_2 && !bstate->_resource2->isEmpty())
		{
			int pid = bstate->_resource2->Dequeue();
			rustate->_process = pcbt->_pcb_table[pid-1];
			rustate->_hasrunning = true;
		}
		else if(rid == RESOURCE_3 && !bstate->_resource3->isEmpty())
		{
			int pid = bstate->_resource3->Dequeue();
			rustate->_process = pcbt->_pcb_table[pid-1];
			rustate->_hasrunning = true;
		}
		else
		{
			//  If the BlockedState is empty say so and return
			fprintf(stderr, "B: Block State is empty; There is nothing to unblock!\n");
			return;
		}

	}
	else
	{
		//  If this EVER triggers something whent seriously wrong
		fprintf(stderr, "Something whent wrong in the Unblocker!\n");
		return;
	}
}


//==================================================================================================||










/*   QUANTUM   */

void scheduler(Time *time_, RunningState *rustate, PCBTable *pcbt, ReadyState *restate)
{

	//  At the very least the scheduler should increment the time_
	time_->_time += 1;

	//  If there is nothing running and nothing in the ready queue then, 
	//  say so, increment the time_( above ), and return
	if(!rustate->_hasrunning && restate->_queue->isEmpty())
	{ 
		fprintf(stderr, "No running or ready processes!\n");
		return;
	}
	else if(!rustate->_hasrunning && !restate->_queue->isEmpty())
	{
		//  If there is nothing running but there is something in the reaady queue:
		//	1.  get the PID by Dequeueing from the ready queue
		//  2.  get the PCB from the PCB Table at location [PID - 1]
		//  3.  Set the RunningState PCB object equal to the PCB you got from the PCB Table
		//  4.  Set the boolean "there is something running" flag to true
		int pid = restate->_queue->Dequeue();
		rustate->_process = pcbt->_pcb_table[pid-1];
		rustate->_hasrunning = true;
		return;
	}
	else
	{
		//  This is the normal functionality section

		//  Get the PID of the currently running process for later use
		int cpid = rustate->_process._pid;

		//  Increment the current process' quantum counter as well as cpu time_ used so far
		rustate->_process._qcounter += 1;
		rustate->_process._cpu_time += 1;

		//  Update the PCB Table
		pcbt->_pcb_table[cpid-1] = rustate->_process;
		if(rustate->_process._cpu_time == rustate->_process._run_time)
		{
			//  This is what happens when a process gets killed ( runs until its runtime )

			//  Calculate the turnaround time_
			int turnaround = time_->_time - rustate->_process._start_time;

			//  set the current process' turnaround time_ equal to the one you calculated above
			rustate->_process._turnaround = turnaround;
			
			//  Update the PCB Table
			pcbt->_pcb_table[cpid-1] = rustate->_process;

			//  a process has died (we killed it, mercylessly, it was very bloody, I laughed...) 
			//  which means that the CPU is empty and we can't have that so if the ready queue is NOT empty:
			//  1.  Get the PID by Dequeueing from the reaady queue
			//  2.  Get the PCB object from the PCB Table at location [PID-1]
			//  3.  Set the currently running's PCB object equal to the PCB you got from the PCB Table
			//  4.  Set the bollean "yes there is something running" flag to true
			if(!restate->_queue->isEmpty())
			{
				int pid = restate->_queue->Dequeue();
				rustate->_process = pcbt->_pcb_table[pid-1];
				rustate->_hasrunning = true;
			}
			else
			{
				//  If there is nothing in the ready queue say so and return
				fprintf(stderr, "B: Ready queue is empty; Nothing to put on CPU!\n");
				return;
			}
			numFinished++;
			finishedTime += turnaround;
			return;
		}
		else if(rustate->_process._qcounter == rustate->_process._quantum)
		{
			//  If the currently running process has used up its quantum AND, 
			// its cpu time_ so far is NOT equal to its total run time_ then do things:
			//  Increase the currently running process' priority by 1 then if it's greater than 3 set it equal to 3
			rustate->_process._priority += 1;
			if(rustate->_process._priority > 3){ rustate->_process._priority = 3;}

			//  Increase the currently running process' quantum by 2x its previous value then, 
			//   if it's greater than 8 set it equal to 8
			rustate->_process._quantum = pow(2, rustate->_process._priority);
			if(rustate->_process._quantum > 8){ rustate->_process._quantum = 8;}

			//  Reset the current process' quantum counter
			//  If this is not done the next time_ the process comes on the CPU it will start from X rather than 0
			rustate->_process._qcounter = 0;

			//  Update the PCB Table
			pcbt->_pcb_table[cpid-1] = rustate->_process;

			//  Push the PID of the currently running process onto the ready queue
			//  Its location will be equal to the currently running process' priority
			restate->_queue->Enqueue(cpid, rustate->_process._priority);

			//  If the ready queue is NOT empty:
			//  1.  Get the PID by Dequeueing from the reaady queue
			//  2.  Get the PCB object from the PCB Table at location [PID-1]
			//  3.  Set the currently running's PCB object equal to the PCB you got from the PCB Table
			//  4.  Set the bollean "yes there is something running" flag to true
			if(!restate->_queue->isEmpty())
			{
				int pid = restate->_queue->Dequeue();
				rustate->_process = pcbt->_pcb_table[pid-1];
				rustate->_hasrunning = true;
				return;
			}
			else
			{
				//  If there is nothing in the ready queue then say so and return
				fprintf(stderr, "A: Ready queue is empty; Nothing to put on CPU!\n");
				return;
			}

		}
		else{ return; } //  If you get here it means the process' quantum is not done and it will simply keep using the CPU

	}
}

//==================================================================================================||








/*   TURNAROUND   */

const void turnaround(PCBTable *pcbt)
{
	double total = 0;

	for(int i = 0; i < pcbt->_pcb_table.size(); i++)
	{
		
		//  total up all of the turnaround times
		total += pcbt->_pcb_table[i]._turnaround;
	}

	//  We don't want to divide by zero
	if(numFinished == 0){ numFinished = 1;}

	//  Print the total turnaround time_ devided by the number of processes
	printf("The Average Turnaround Time Is: %f\n\n", total/numFinished);

	printf("Extra stuff you might want to know:\n");
	printf("%d processes finished in a total of %d seconds.\n", numFinished, finishedTime);
	printf("Total number of processes: %d\n\n", numProcesses);
}

//==================================================================================================||







const void reporter(PCBTable *pcbt, Time *time_, RunningState *rustate, BlockedState *bstate, ReadyState *restate)
{
	
	//  This one is big and kind of ugly all it does is print stuff out according to specifications

	//  Print the current time_

	printf("****************************************************************\n");
	printf("The current state is as follows:\n");
	printf("****************************************************************\n\n");

	printf("CURRENT TIME: %d\n\n", time_->_time);


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\







	//  Print the currently running process

	if(rustate->_hasrunning)
	{
		printf("RUNNING PROCESS:\n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		printf("< %2d%10d%13d%13d%14d >\n", rustate->_process._pid, rustate->_process._priority, 
			rustate->_process._value, rustate->_process._start_time, rustate->_process._cpu_time);
	}
	else
	{
		//  If there is no process currently running say so
		printf("RUNNING PROCESS: NONE\n");
	}

	printf("\n\n");


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\






	//  Print all process that are currently blocked for a particular resource ID





	//  RESOURCE 1  AKA RESOURCE ID = 0
	printf("BLOCKED PROCESSES: \n");
	if(!bstate->_resource1->isEmpty())
	{
		printf("Queue of processes blocked for resource 0: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int i=0; i<bstate->_resource1->Asize(); i++)
		{
			int* q = bstate->_resource1->Qstate(i);
			int size = bstate->_resource1->Qsize(i);
			for(int j = 0; j<size; j++)
			{
				printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
					pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
			}
		}
	}
	else
	{
		printf("Queue of processes Blocked for resource 0 is empty: \n");
	}

	printf("\n");


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\







	//RESOURCE 2  AKA RESOURCE ID = 1
	if(!bstate->_resource2->isEmpty())
	{
		printf("Queue of processes blocked for resource 1: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int i=0; i<bstate->_resource2->Asize(); i++)
		{
			int* q = bstate->_resource2->Qstate(i);
			int size = bstate->_resource2->Qsize(i);
			for(int j = 0; j<size; j++)
			{
				printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
					pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
			}
		}
	}
	else
	{
		printf("Queue of processes Blocked for resource 1 is empty: \n");
	}

	printf("\n");


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\







	//RESOURCE 3  AKA RESOURCE ID = 2
	if(!bstate->_resource3->isEmpty())
	{
		printf("Queue of processes blocked for resource 2: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int i=0; i<bstate->_resource3->Asize(); i++)
		{
			int* q = bstate->_resource3->Qstate(i);
			int size = bstate->_resource3->Qsize(i);
			for(int j = 0; j<size; j++)
			{
				printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
					pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
			}
		}
	}
	else
	{
		printf("Queue of processes Blocked for resource 2 is empty: \n");
	}

	printf("\n\n");


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\








	//  Print the processes in the Ready Queue


	// PRIORITY 0
	printf("PROCESSES READY TO EXECUTE: \n");

	int* q = restate->_queue->Qstate(0);
	int size = restate->_queue->Qsize(0);
	if(size == 0)
	{
		printf("Queue of processes with priority 0 is empty\n");
		printf("\n");
	}
	else
	{
		printf("Queue of processes with priority 0: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int j=0; j<size; j++)
		{
			printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
				pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
		}
		printf("\n");
	}


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\







	//PRIORITY 1
	q = restate->_queue->Qstate(1);
	size = restate->_queue->Qsize(1);
	if(size == 0)
	{
		printf("Queue of processes with priority 1 is empty\n");
		printf("\n");
	}
	else
	{
		printf("Queue of processes with priority 1: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int j=0; j<size; j++)
		{
			printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
				pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
		}
		printf("\n");
	}


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\








	//PRIORITY 2
	q = restate->_queue->Qstate(2);
	size = restate->_queue->Qsize(2);
	if(size == 0)
	{
		printf("Queue of processes with priority 2 is empty\n");
		printf("\n");
	}
	else
	{
		printf("Queue of processes with priority 2:: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int j=0; j<size; j++)
		{
			printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
				pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
		}
		printf("\n");
	}


	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\\




	//PRIORITY 3
	q = restate->_queue->Qstate(3);
	size = restate->_queue->Qsize(3);
	if(size == 0)
	{
		printf("Queue of processes with priority 3 is empty\n");
		printf("\n");
	}
	else
	{
		printf("Queue of processes with priority 3: \n");
		printf("  PID     PRIORITY     VALUE     START TIME     CPU TIME\n");
		for(int j=0; j<size; j++)
		{
			printf("< %2d%10d%13d%13d%14d >\n", pcbt->_pcb_table[q[j]-1]._pid, pcbt->_pcb_table[q[j]-1]._priority, 
				pcbt->_pcb_table[q[j]-1]._value, pcbt->_pcb_table[q[j]-1]._start_time, pcbt->_pcb_table[q[j]-1]._cpu_time);
		}
		printf("\n");
	}



	//  END
	printf("\n");
	printf("****************************************************************\n\n\n");

}










/* ------------------------------------- MISCELLANIOUS -------------------------------------*/



void initialize(Time *time_, RunningState *rustate, ReadyState *restate, BlockedState *bstate )
{
	time_->_time = 0;
	rustate->_hasrunning = false;

	restate->_queue = new QueueArray<int>(4);

	bstate->_resource1 = new QueueArray<int>(4);
	bstate->_resource2 = new QueueArray<int>(4);
	bstate->_resource3 = new QueueArray<int>(4);

}


