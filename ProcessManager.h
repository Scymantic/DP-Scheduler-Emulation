

// Process Manager.h
// Structures and Definitions
// Adrian Barberis

//Last Edit: Wed FEB 22, 2017

#include "QueueArray.h"


/*--------------------------- STRUCTURES --------------------------*/



/*  TIME  */
struct Time
{
	int _time;
};



/*  BLOCKED STATE  */
struct BlockedState
{
	int _resourceID;
	QueueArray<int> *_resource1;
	QueueArray<int> *_resource2;
	QueueArray<int> *_resource3;
};



/*  READY STATE  */
struct ReadyState
{
	QueueArray<int> *_queue;
};



/*  PCB INFO  */
struct PCB
{
	PCB(){}

	//  Copy constructor
	PCB(const PCB &other)
	{
		this->_pid = other._pid;
		this->_priority = other._priority;
		this->_value = other._value;
		this->_start_time = other._start_time;
		this->_run_time = other._run_time;
		this->_cpu_time = other._cpu_time;
		this->_turnaround = other._turnaround;
		this->_qcounter = other._qcounter;
		this->_quantum = other._quantum;
	}


	int _pid;		//Process' ID
	int _priority;		//Process' priority
	int _value;		//Process' value
	int _start_time;	//Process' start time
	int _run_time;		//Process' runtime
	int _cpu_time; 		//Increment when a time cycle is done
	int _turnaround;	//Process' turnaround time
	int _qcounter;		//Increment when a quantum is used up
	int _quantum;		//Process' quantum
};



/*  PCB TABLE  */
struct PCBTable
{
	std::vector<PCB> _pcb_table;
};



/*  RUNNING STATE  */
struct RunningState
{
	PCB _process;
	bool _hasrunning; // True if the running state has a running process

};



/*---------------------------------- DEFINITIONS ----------------------------------*/


void start(int* cmdp, PCBTable *pcbt, RunningState *rustate, ReadyState *restate, Time *time);
//  Reads in 3 values from commander.cpp pipe then creates a new PCB object intitalizes it with those values
//      and stores it into the PCB Table.
//
//  If there are no currently running processes then the newly created process becomes the running process.
//  Otherwise the newly created process PID is pushed onto the ready queue.



void calculate(int* cmdp, RunningState *rustate, PCBTable *pcbt, ReadyState *restate, Time *time);
//  Reads 2 values from the commander.cpp pipe; A 'char' (the command to execute) and,
//  	 an 'int' (the number to use in the arithmetic operation)
//
//  Performs an arithmetic operation using the value of the currently running process and the number read
//  The operation performed corresponds to the character that was read in:
//
//	A:  Add
//  	S:  Subtract
// 	M:  Multiply
//  	D:  Divide (if there is a divide by zero error it will say so and not change the value)



void unblock(BlockedState *bstate, ReadyState *restate, PCBTable *pcbt, RunningState *rustate, int rid);
//  Moves a process from one of the BlockedState queues into the ReadyState queue based on the resource ID it is given
//  If there is nothing currently running: put that process onto the CPU instead of the Ready State queue 



void block(RunningState *rustate,  PCBTable *pcbt, BlockedState *bstate, ReadyState *restate, int rid);
//  Moves a process from the CPU into one of the BlockedState resource queues based on a resource ID
//  Then if there exist processes in the ready queue: get one (with highest priority) and put it onto the CPU 



const void reporter(PCBTable *pcbt, Time *time, RunningState *rustate, BlockedState *bstate, ReadyState *rstate);
//  Prints the current system state



void scheduler(Time *time, RunningState *rustate, PCBTable *pcbt, ReadyState *restate);
//  1. Increments the system time
//
//  2. If there isn't a running process and there are no processes in the ready queue; return
//
//  3. If there isn't a running process but there ARE processes in the ready queue pull a process (with highest priority)
//		from the ready queue and put it onto the CPU
//
//  4. If you are not in one of the two cases above:
//		a. Increment the current process' quantum and cpu time used so far
//		b. Update the PCB table
//		c. If the process' quantum counter is equal to its alloted quantum i.e. its time slice is over:
//			i. Increase the current process' priority then if it's greater than 3 set it to 3
//			ii. Increase the current process' quantum by 2x its previous value
//			iii. Reset the current process' quantum counter
//			iv. Update the PCB table
//			v. Move the current process from the CPU to the ready queue.  Its location will be equal to its priority
//			vi. If the ready queue is NOT empty move a process (with highest priority) from the ready queue onto the CPU
//				- If the ready queue is empty, say so and return
//
//  5. If the current process' cpu time is equal to its total run time, kill it
//		a. Calculate the process' turnaround time
//		b. Update the PCB Table
//		c. If the ready queue is NOT empty move a process (with highest priority) from the ready queue onto the CPU
//				- If the ready queue is empty, say so and return
//
//	6. If none of the above are true then the current process has not used up its quantum or CPU time, 
//		 and is welcome to remain on the CPU until it does so.



const void turnaround(PCBTable *pcbt);
//  Calculate the average turnaround time for the system
//  Print some other miscellanious information such as:
//	a.  number of processes that have completed
//  	b.  total time taken for all completed processes
//  	c.  total number of processes in the system
//  	d.  total time taken by all processes in the system


void initialize(Time *time, RunningState *rustate, ReadyState *restate, BlockedState *bstate );
//  Initialize the values in the various structures above











