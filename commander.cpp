

// Adrian Barberis
// Commander Process

//Last Edit: Fri MAR 3, 2017


/*----------------------------- PREPROCESSORS -----------------------------*/


#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <iostream>

#define READ_END 0
#define WRITE_END 1




/* ---------------------------- BEGIN MAIN -------------------------------- */



// A return value of 0 == All is Well 

// A return value of 1 == something whent wrong



int main(int argc, char** argv)
{

	if(argc > 1 || argc < 1)
	{
		fprintf(stderr, "Error: Invalid argument count!\n");
		return 1;
	}
	//Comander Prcess PIpe
	int cmdp[2];

	//Store Address of Commander Pipe
	char mc0[10], mc1[10];

	//Important variables for getline
	char* line = NULL;
	ssize_t leng = 0;
	size_t cap = 0;

	//Token storage for strtok() and sscanf()
	char* token = 0;

	//Keep track of "Initial Line Read" for strtok()
	int init = 0;

	if(pipe(cmdp))
	{
		perror("CMNDR: Pipe Creation Failed!"); 
		return 1;
	}
	
	//create a string value, to pass the child process, that is the address of pipe
	sprintf(mc0, "%d", cmdp[0]);
	sprintf(mc1, "%d", cmdp[1]);


	pid_t pid = fork();
	if(pid < 0)
	{
		perror("CMNDR: Fork Failed!");
		return 1;
	}
	else if(pid > 0)
	{
		/* IS PARENT */


		//close pipe read end
		close(cmdp[READ_END]);

		while((leng = getline(&line, &cap, stdin)) > 0)
		{

			//get intial strtok read
			if(init == 0)
			{
				token = strtok(line, " ");
			}

			//set init to 1 in order to NOT RE-READ "line" into strtok
			init = 1;
			switch(*token)
			{

				case 'S':
				{
					sleep(2);
					//write function identifier to pipe
					write(cmdp[WRITE_END], token, strlen(token));

					//storage for sscanf
					int num = 0;

					//argument count check
					int argcount = 0;

					//Read from stdin and parse
					while(token != NULL)
					{
						//If S has more than 3 arg quit
						if(argcount > 3)
						{
							printf("ERROR: Too many(> 3) Arguments for S()!\nExiting...\n");
							return 1;
						}
						
						//Tokenize the current line
						token = strtok(NULL," ");
						if(token != NULL)
						{
							//convert char read from strtok() into integer
							sscanf(token, "%d", &num);
							write(cmdp[WRITE_END], &num, sizeof(int));

							//Increment argcount for check
							argcount++;
						}
					}

					//Reset the inti value in order to read next line
					init = 0;

					break;
				}
				case 'B':
				{
					sleep(2);
					//Write function identifier to pipe
					write(cmdp[WRITE_END], token, strlen(token));

					//storage for sscanf()
					int num = 0;

					//argument count check
					int argcount = 0;

					//read in from stdin and parse
					while(token != NULL)
					{
						//If more than 1 argument; quit
						if(argcount > 1)
						{
							printf("ERROR: Too many(> 1) Arguments for B()!\nExiting...\n");
							return 1;
						}

						//Tokenize current line
						token = strtok(NULL," ");
						if(token != NULL)
						{
							//convert token read to int
							sscanf(token, "%d", &num);
							write(cmdp[WRITE_END], &num, sizeof(int));

							//increment argcount for check
							argcount++;
						}
					}

					//reset init to read next line
					init = 0;
					break;
				}
				case 'U':
				{

					sleep(2);
					//write function identifier to pipe
					write(cmdp[WRITE_END], token, strlen(token));

					//storage for sscanf()
					int num = 0;

					//argument count check
					int argcount = 0;

					//read from stdin and parse
					while(token != NULL)
					{
						//if more than 1 arg; quit
						if(argcount > 1)
						{
							printf("ERROR: Too many(> 1) Arguments for U() !\nExiting...\n");
							return 1;
						}

						//tokenize line
						token = strtok(NULL," ");
						if(token != NULL)
						{
							//convert current token into integer
							sscanf(token, "%d", &num);
							write(cmdp[WRITE_END], &num, sizeof(int));

							//increment argcount for check
							argcount++;
						}
					}

					//reset init to read next line
					init = 0;
					break;
				}
				case 'C':
				{
					sleep(2);
					//write function identifier to pipe
					write(cmdp[WRITE_END], token, sizeof(char));

					//storage for sscanf()
					int num = 0;

					//read and parse ONLY 2 args
					for(int i=0; i<2; i++)
					{
						//First arg should be a letter i.e. char
						if(i == 0 && token != NULL)
						{
							//tokenize line
							token = strtok(NULL," ");
							write(cmdp[WRITE_END], token, sizeof(char));
						}
						else if(i == 1 && token != NULL)  //second arg should be number
						{
							//tokenize line
							token = strtok(NULL, " ");
							sscanf(token, "%d", &num);
							write(cmdp[WRITE_END], &num, sizeof(int));
						}
					}

					//check for argcount > 2
					token = strtok(NULL, " ");
					if(token != NULL)
					{
						fprintf(stderr, "ERROR: Too many(> 2) Arguments for C()!\nExiting...\n");
						return 1;
					}

					//reset init for next line
					init = 0;
					break;
				}
				case 'P':
				{
					sleep(2);
					//write function identifier to pipe
					write(cmdp[WRITE_END], token, sizeof(char));

					//if next read token is not NULL then too many args; quit
					token = strtok(NULL," ");
					if(token != NULL)
					{
						printf("ERROR: Too many(> 0) Arguments for P()\nExiting...\n");
						return 1;
					}

					//reset init for next line
					init = 0;
					break;
				}
				case 'T':
				{
					sleep(2);
					//write function identifier to pipe
					write(cmdp[WRITE_END], token, sizeof(char));

					//If a token beyond the Identifier is read; too many args, quit
					token = strtok(NULL," ");
					if(token != NULL)
					{
						printf("ERROR: Too many(> 0) Arguments for T()\nExiting...\n");
						return 1;
					}

					//reset init for next line
					init = 0;

					//  close the pipe and wait for child
					close(cmdp[WRITE_END]);
					wait(NULL);
					return 0;
					
				}
				case 'Q':
				{
					sleep(2);
					//Write function identifier to pipe
					write(cmdp[WRITE_END], token, sizeof(char));

					//If a token beyond the Identifier is read; too many args, quit
					token = strtok(NULL," ");
					if(token != NULL)
					{
						printf("ERROR: Too many(> 0) Arguments for Q()\nExiting...\n");
						return 1;
					}

					//reset init for next line
					init = 0;
					break;
				}
				default:
				{
					printf("CMNDR: Unidentified Command!\nExiting...\n");
					close(cmdp[WRITE_END]);
					return 1;
				}
			}

			//Reset for getline()
			line = NULL;
			cap = 0;

		}

		//close pipe and wait for child
		close(cmdp[WRITE_END]);
		wait(NULL);
		return 0;
	}
	else
	{
		/* IS CHILD */

		//Execute processManager
		size_t stat = execl("procMan", "procMan", mc0, mc1, NULL);

		//if execution of processManager fails; quit
		if(stat == -1)
		{
			printf("CMNDR: Error Excuting Process Manager!\nExiting...\n");
			return 1;
		}
		return 0;
	}


	return 0;
}



/*  END MAIN */

