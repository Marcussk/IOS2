/*
*IOS projekt c.2
*Marek Beno,xbenom01 
*4.5.2014
* Program sa zaobera synchronizacnym problemom River Crossing
*/

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

//Struktury
typedef struct {
  int p;        //number of persons generated
  int h;		//delay between creation of new hacker
  int s;		//delay between creation of new serf
  int r;		//length of rivercrossing
} Arguments;

typedef struct {
	int boardingHack;
	int boardingSerf;
	int actionindex;
	int peopleboarded;
	int peopletransported;
	int peopleunloaded;
	int membersleepers;
	} shared;


//Prototypes
bool checkarg(int argc,char *argv[]);
bool loadsem();
bool hacker(int hackerID);
bool serf(int serfID);
void closesemaphores();
void shutdown();
void parenthandler();
void genhandler();
void hackerhandler();
void serfhandler();


//Global variables
sem_t *boardingpassH;
sem_t *boardingpassS;
sem_t *moloqueue;
sem_t *boatvoyage;
sem_t *beacharrival;
sem_t *graveyardfinish;
sem_t *memprotect;
sem_t *captaindreaming;
pid_t HackergenPID;
pid_t SerfgenPID;  
int shm_fd;
int shm_df;
shared *Memstruct;
Arguments *args;
FILE *out = NULL;

//Implementation
int main(int argc,char *argv[])
{
	//handlers for signals received by parent
	signal(SIGTERM, parenthandler);
	signal(SIGINT, parenthandler);
	
	//load semaphores and shared memory
	if(loadsem() != 0)
		{
		fprintf(stderr, "Error setting up semaphores and shared memory\n");
		shutdown();
		return 1;
		}
	
	//check arguments of program according to specification
    if(checkarg(argc,argv) != 0)
		{
		fprintf(stderr, "Incorrect number of arguments or argument in wrong format\n");
		shutdown();
		return 1;
		}
		
	//open output file and handle errors	
	if( (out = fopen("rivercrossing.out","w+") ) == NULL)
		{
		fprintf(stderr,"Error opening file\n");
		shutdown();
		return 1;
		}
	setbuf(out, NULL); 	
	
		
    //create process which will generate hackers    
    HackergenPID = fork();
    if(HackergenPID >= 0) // fork was successful Hackergen created
    {
        
        if(HackergenPID == 0) // code for Hackergen
        {
			//signal handlers for hackergenerator
			signal(SIGTERM, genhandler);
			signal(SIGINT, genhandler);
			//random time seed
			srand(time(NULL) * getpid()); 
			//create hackers according to parameter p
			for(int i = 1; i < (args->p + 1);i++)
			{
				//delay
				usleep(rand() % ((args->h-0)+1));
				//create hacker
				pid_t HackerPID = fork();
				if(HackerPID >= 0) //fork successful Hacker created
				{
					if(HackerPID == 0) //code for Hacker
					{
					//signal handlers for hacker
					signal(SIGTERM, hackerhandler);
					signal(SIGINT, hackerhandler);
					//hackers activities
					hacker(i);
					//close all semaphores before ending
					closesemaphores();
					exit(0);	
					}		
					
				}
				else //fork failed Hacker not created
				{
					fprintf(stderr, "Error when creating hacker procces\n");
					//stop and terminate all children
					kill(0,SIGSTOP);
					kill(0,SIGTERM);
					exit(2);
				}	
			}
			//wait for all the children
			for(int i = 1;i < (args->p + 1);i++){wait(NULL);}
			exit(0);
        }
        else //code for Parent
        {
            //Parent of Hackergen
        }
    }
    //Code for Serfgen is symmetric to Hackergen
    //for additional comments see Hackergen
    else // fork failed Hackergen not created
    {
        fprintf(stderr, "Error when creating hacker process generator\n");
        kill(SerfgenPID,SIGSTOP);
        kill(SerfgenPID,SIGINT);
        shutdown();
        return 2;
    }
    
    SerfgenPID = fork();
    if(SerfgenPID >= 0) // fork was successful Serfgen created
    {
        if(SerfgenPID == 0) // code for Serfgen
        {
            
            signal(SIGTERM, genhandler);
			signal(SIGINT, genhandler);
            
            srand(time(NULL) * getpid()); 	
			for(int j = 1; j < (args->p + 1);j++)
			{
				usleep(rand() % ((args->s-0)+1));
				pid_t SerfPID = fork();
				if(SerfPID >= 0) //fork successful Serf created
				{
					if(SerfPID == 0) //code for Serf
					{
					signal(SIGTERM, serfhandler);
					signal(SIGINT, serfhandler);
					serf(j);
					closesemaphores();
					exit(0);	
					}		
					
				}
				else //fork failed Serf not created
				{
					fprintf(stderr, "Error when creating serf\n");
					kill(0,SIGSTOP);
					kill(0,SIGTERM);
					exit(2);
				}	
			}
			for(int j = 1;j < (args->p + 1);j++){wait(NULL);}
			exit(0);
        }
        else //code for Parent
        {
            
        }
    }
    else // fork failed Serfgen not created
    {
        fprintf(stderr, "Error when creating Serf generator\n");
        kill(HackergenPID,SIGSTOP);
        kill(HackergenPID,SIGINT);
        shutdown();
        return 2;
    }
    
    //wait for both generators and end safely
	wait(NULL);
	wait(NULL);
	shutdown();
    
    return 0;
}

bool checkarg(int argc,char *argv[])
{
	//checking number of arguments
	if(argc != 5){return 1;}
		
	//checking whether argument is not a number
	char *err;
		args->p = strtol(argv[1], &err,10);
			if (*err != '\0'){return 1;}
		args->h = strtol(argv[2], &err,10);
			if (*err != '\0'){return 1;}
		args->s = strtol(argv[3], &err,10);
			if (*err != '\0'){return 1;}
		args->r = strtol(argv[4], &err,10);
			if (*err != '\0'){return 1;}	
	
	//checking range and format		
	if((args->p < 0) || ((args->p % 2) != 0)){return 1;}
	if((args->h < 0) || (args->h >= 5001)){return 1;}	
	if((args->s < 0) || (args->s >= 5001)){return 1;}
	if((args->r < 0) || (args->r >= 5001)){return 1;}			 
	return 0;	
}

bool loadsem()
{
	//setup semaphore
	boardingpassH = sem_open("/boardingpassH", O_CREAT | O_EXCL, 0644, 0);
	boardingpassS = sem_open("/boardingpassS", O_CREAT | O_EXCL, 0644, 0);
	moloqueue = sem_open("/moloqueue", O_CREAT | O_EXCL, 0644, 1);
	boatvoyage = sem_open("/boatvoyage", O_CREAT | O_EXCL, 0644, 0);
	beacharrival = sem_open("/beacharrival", O_CREAT | O_EXCL, 0644, 0);
	graveyardfinish = sem_open("/graveyardfinish", O_CREAT | O_EXCL, 0644, 0);
	memprotect = sem_open("/memprotect", O_CREAT | O_EXCL, 0644, 1);
	captaindreaming = sem_open("/captaindreaming", O_CREAT | O_EXCL, 0644, 0);
	
	//check for error during semaphores setup
	if(boardingpassH == SEM_FAILED){return -1;}
	if(boardingpassS == SEM_FAILED){return -1;}
	if(moloqueue == SEM_FAILED){return -1;}
	if(boatvoyage == SEM_FAILED){return -1;}
	if(beacharrival == SEM_FAILED){return -1;}
	if(graveyardfinish == SEM_FAILED){return -1;}
	if(memprotect == SEM_FAILED){return -1;}
	if(captaindreaming == SEM_FAILED){return -1;}
	
	//setup and allocate memore, check for errors
	shm_fd = shm_open("/sharedMemspace", O_CREAT | O_EXCL | O_RDWR, 0644);
	shm_df = shm_open("/sharedArgs", O_CREAT | O_EXCL | O_RDWR, 0644);
	if(shm_fd == -1){return -1;}
	if(shm_df == -1){return -1;}
	ftruncate(shm_fd, sizeof(shared));
	ftruncate(shm_df, sizeof(Arguments));
	
	//initialization of shared structure
	Memstruct = mmap(NULL, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	Memstruct->boardingHack = 0;
	Memstruct->boardingSerf = 0;
	Memstruct->actionindex = 1;
	Memstruct->peopleboarded = 0;
	Memstruct->peopletransported = 0;
	Memstruct->peopleunloaded = 0;
	Memstruct->membersleepers = 0;
	
	//initialization of shared structure
	args = mmap(NULL, sizeof(Arguments), PROT_READ | PROT_WRITE, MAP_SHARED, shm_df, 0);
	args->p = 0;
	args->h = 0;
	args->s = 0;
	args->r = 0;
	
	return 0;
}

bool hacker(int hackerID)
{
	int captain = 0;
	//seed for random generation
	srand(time(NULL) * getpid()); 
	
	//Start proccess
	sem_wait(memprotect);
	fprintf(out,"%d: hacker: %d: started\n",Memstruct->actionindex,hackerID);	
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	//wait on beach
	sem_wait(moloqueue);
	
	//wait to board
	sem_wait(memprotect);
	Memstruct->boardingHack++;
	fprintf(out,"%d: hacker: %d: waiting for boarding: %d: %d\n",Memstruct->actionindex,hackerID,Memstruct->boardingHack,Memstruct->boardingSerf);
	Memstruct->actionindex++;
	sem_post(memprotect);
				
	//allow group of people or open for another process			
	sem_wait(memprotect);
	if (Memstruct->boardingHack == 4)
		{
		Memstruct->boardingHack -= 4;
		
		captain = 1;
		sem_post(boardingpassH);
		sem_post(boardingpassH);
		sem_post(boardingpassH);
		sem_post(boardingpassH);
		//sem_wait(moloqueue);
		}
	else if ((Memstruct->boardingHack == 2) && (Memstruct->boardingSerf >= 2))
		{
		Memstruct->boardingSerf -= 2;
		Memstruct->boardingHack -= 2;
		captain = 1;
		sem_post(boardingpassH);
		sem_post(boardingpassH);
		sem_post(boardingpassS);
		sem_post(boardingpassS);	
		}
	else
		{
		sem_post(moloqueue);	
		}		
	sem_post(memprotect);
		
	//wait till group of people is formed	
	sem_wait(boardingpassH);

	//board
	sem_wait(memprotect);
	fprintf(out,"%d: hacker: %d: boarding: %d: %d\n",Memstruct->actionindex,hackerID,Memstruct->boardingHack,Memstruct->boardingSerf);
	Memstruct->peopleboarded++;
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	//if everyone boarded boat open semaphore
	sem_wait(memprotect);
	if(Memstruct->peopleboarded == 4)
		{
		Memstruct->peopleboarded-=4;
		sem_post(boatvoyage);
		sem_post(boatvoyage);
		sem_post(boatvoyage);
		sem_post(boatvoyage);	
		}
	sem_post(memprotect);
	
	//wait before going to boat for everyone
	sem_wait(boatvoyage);
	
	//determine whether process is member or captain
	sem_wait(memprotect);	
	if(captain == 1){fprintf(out,"%d: hacker: %d: captain\n",Memstruct->actionindex,hackerID);Memstruct->actionindex++;}	
	else{fprintf(out,"%d: hacker: %d: member\n",Memstruct->actionindex,hackerID);Memstruct->actionindex++;}
	Memstruct->membersleepers++;
	sem_post(memprotect);	
	
	//wait till everyone announces their status and open semaphore if everyone is here
	sem_wait(memprotect);
	if(Memstruct->membersleepers == 4)
		{
		Memstruct->membersleepers -= 4;
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		}
	sem_post(memprotect);
	
	sem_wait(captaindreaming);
	
	//wait for captain to finish sleeping
	sem_wait(memprotect);
	if(captain == 1)
		{
		usleep(rand() % ((args->r-0)+1));
		sem_post(beacharrival);
		sem_post(beacharrival);
		sem_post(beacharrival);
		sem_post(beacharrival);
		}		
	sem_post(memprotect);
		
	sem_wait(beacharrival);
	
	//announce landing
	sem_wait(memprotect);	
	fprintf(out,"%d: hacker: %d: landing: %d: %d\n",Memstruct->actionindex,hackerID,Memstruct->boardingHack,Memstruct->boardingSerf);	
	Memstruct->actionindex++;
	Memstruct->peopleunloaded++;
	sem_post(memprotect);	
	
	//open molo if everyone landed	
	sem_wait(memprotect);
	if(Memstruct->peopleunloaded == 4){sem_post(moloqueue);Memstruct->peopleunloaded-=4;}
	sem_post(memprotect);
	
	//if everyone is here let everyone die
	sem_wait(memprotect);
	Memstruct->peopletransported++;
	if(Memstruct->peopletransported == (2*args->p))
		{
		for(int k = 0; k < (2*args->p);k++)
			{
			sem_post(graveyardfinish);	
			}	
		}	
	sem_post(memprotect);

	sem_wait(graveyardfinish);
	
	//announce death of process
	sem_wait(memprotect);
	fprintf(out,"%d: hacker: %d: finished\n",Memstruct->actionindex,hackerID);
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	return 0;	
}

//Code for serf is symmetric to hacker see hacker for additional comments
bool serf(int serfID)
{
	srand(time(NULL) * getpid()); 
	int captain = 0;
	
	sem_wait(memprotect);
	fprintf(out,"%d: serf: %d: started\n",Memstruct->actionindex,serfID);
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	sem_wait(moloqueue);
	
	sem_wait(memprotect);
	Memstruct->boardingSerf++;
	fprintf(out,"%d: serf: %d: waiting for boarding: %d: %d\n",Memstruct->actionindex,serfID,Memstruct->boardingHack,Memstruct->boardingSerf);
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	sem_wait(memprotect);
	if (Memstruct->boardingSerf == 4)
		{
		Memstruct->boardingSerf -= 4;
		captain = 1;
		sem_post(boardingpassS);
		sem_post(boardingpassS);
		sem_post(boardingpassS);
		sem_post(boardingpassS);
		}
	else if ((Memstruct->boardingSerf == 2) && (Memstruct->boardingHack >= 2))
		{
		Memstruct->boardingSerf -= 2;
		Memstruct->boardingHack -= 2;
		captain = 1;
		sem_post(boardingpassH);
		sem_post(boardingpassH);
		sem_post(boardingpassS);
		sem_post(boardingpassS);
		}
	else
		{
		sem_post(moloqueue);
		}				
	sem_post(memprotect);
	
	
	sem_wait(boardingpassS);
	
	sem_wait(memprotect);
	fprintf(out,"%d: serf: %d: boarding: %d: %d\n",Memstruct->actionindex,serfID,Memstruct->boardingHack,Memstruct->boardingSerf);
	Memstruct->peopleboarded++;
	Memstruct->actionindex++;
	sem_post(memprotect);
	
	sem_wait(memprotect);
	if(Memstruct->peopleboarded == 4)
		{
		Memstruct->peopleboarded-=4;
		sem_post(boatvoyage);
		sem_post(boatvoyage);
		sem_post(boatvoyage);
		sem_post(boatvoyage);	
		}
	sem_post(memprotect);
	
	sem_wait(boatvoyage);
	
	sem_wait(memprotect);
	if(captain == 1){fprintf(out,"%d: serf: %d: captain\n",Memstruct->actionindex,serfID);Memstruct->actionindex++;}	
	else{fprintf(out,"%d: serf: %d: member\n",Memstruct->actionindex,serfID);Memstruct->actionindex++;}
	Memstruct->membersleepers++;
	sem_post(memprotect);
	
	sem_wait(memprotect);
	if(Memstruct->membersleepers == 4)
		{
		Memstruct->membersleepers -= 4;
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		sem_post(captaindreaming);
		}
	sem_post(memprotect);
	
	sem_wait(captaindreaming);
	
	
	sem_wait(memprotect);
	if(captain == 1)
		{
		usleep(rand() % ((args->r-0)+1));
		sem_post(beacharrival);
		sem_post(beacharrival);
		sem_post(beacharrival);
		sem_post(beacharrival);
		}		
	sem_post(memprotect);
		
	sem_wait(beacharrival);
	
	sem_wait(memprotect);
	fprintf(out,"%d: serf: %d: landing: %d: %d\n",Memstruct->actionindex,serfID,Memstruct->boardingHack,Memstruct->boardingSerf);
	Memstruct->actionindex++;
	Memstruct->peopleunloaded++;
	sem_post(memprotect);	
		
	sem_wait(memprotect);
	if(Memstruct->peopleunloaded == 4){sem_post(moloqueue);Memstruct->peopleunloaded-=4;}
	sem_post(memprotect);
	
	sem_wait(memprotect);
	Memstruct->peopletransported++;
	if(Memstruct->peopletransported == (2*args->p))
		{
		for(int l = 0; l < (2*args->p);l++)
			{
			sem_post(graveyardfinish);	
			}	
		}	
	sem_post(memprotect);

	sem_wait(graveyardfinish);
	
	sem_wait(memprotect);
	fprintf(out,"%d: serf: %d: finished\n",Memstruct->actionindex,serfID);
	Memstruct->actionindex++;
	sem_post(memprotect);
	return 0;		
}

//exit safely
void shutdown()
{
	//unlink all semaphore if their creation was not failed
	//if(boardingpassH != SEM_FAILED)
	{sem_unlink("/boardingpassH");}
	//if(boardingpassH != SEM_FAILED)
	{sem_unlink("/boardingpassS");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/memprotect");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/moloqueue");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/boatvoyage");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/beacharrival");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/graveyardfinish");}
    //if(boardingpassH != SEM_FAILED)
    {sem_unlink("/captaindreaming");}
    
    //unlink memory
    munmap(Memstruct, sizeof(int));
    shm_unlink("/sharedMemspace");
    shm_unlink("/sharedArgs");
    
    //close shared memory
    close(shm_fd);
    close(shm_df);
    if(out != NULL){fclose(out);}
}	

void closesemaphores()
{
	//close all semaphores used by process
	sem_close(boardingpassH);
	sem_close(boardingpassS);	
	sem_close(moloqueue);
	sem_close(boatvoyage);
	sem_close(beacharrival);
	sem_close(graveyardfinish);
	sem_close(memprotect);
	sem_close(captaindreaming);
}	

//Handlers for every type of process
//upon receiving signal MAIN process terminates generators then exits safely
//GEN process kills all children and exits
//CHILDREN send signal to GEN to terminate and exit
void parenthandler()
{
		kill(0,SIGTERM);
		shutdown();
		exit(1);
}	

void genhandler()
{
		kill(0,SIGTERM);
		parenthandler();
		exit(1);
}

void hackerhandler()
{
		parenthandler();
		closesemaphores();
		exit(1);
}

void serfhandler()
{
	parenthandler();
	closesemaphores();
	exit(1);
}

