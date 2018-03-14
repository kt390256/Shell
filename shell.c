#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>


#define bufferSize 256
#define maxArguments 8


//==============================Print & Split===============================//
void printCMP(){
  char currentDir[bufferSize];
  getcwd(currentDir, bufferSize);
  printf("MyShell~%s >>", currentDir);
}

void splitCMD(char* userCommand, char** commandParams, int *argc, int *singleArrow, int *doubleArrow, int *background, int *reverseArrow, int *pipe){


	for(int i = 0; i<maxArguments;i++){

		//getting first argument, and extract that from "userCommand" before the first space
		commandParams[i] = strsep(&userCommand, " ");
		*argc = i;//keeps counting

		
		//there is nothing to get from the commandParams, or if there is nothing inside commandParams
		if(commandParams[i]==NULL)
			break;//break the loop

		//===================================external symbols=====================================//
		if(strcmp(commandParams[i],">")==0){ //if ">" is detected, set singleArrow to 1, do something
 		*singleArrow = 1;
 		//*doubleArrow = 0;
 		}

 		 else if(strcmp(commandParams[i], ">>")==0) //if ">>" is detected, set sdoubleArrow to 1, do something
		  {
		 	//*singleArror = 0;
		 	*doubleArrow = 1;
		  }

		else if(strcmp(commandParams[i], "<")==0)
		 {
		 	*reverseArrow = 1;
		 }

		 else if(strcmp(commandParams[i], "|")==0)
		 {
		 	*pipe = 1;
		 }

		else if(strcmp(commandParams[i], "&")==0)
		{
 			*background = 1;
 		}
 		//========================================================================================//
		if(strlen(commandParams[i])==0){
			*argc--;
			i--;
		}
	}
}
//=====================================================================//

//================execute command====================================//
int execArgs(char** commandParams)
{

	printf("You are inside execArgs\n");
  //forking a child, pid is process identifier
  	pid_t pid = fork();
	char currentDir[bufferSize];

  if(pid < 0){
    fprintf(stderr, "Fork Failed");
    return 1;
  }

	 if(pid == 0)//child process
  	{
  		if(strcmp(commandParams[0],"cd")==0)
    {
    	printf("You are cding from here !\n");
      chdir(commandParams[1]);//change directory to whatever
    }
    else if(strcmp(commandParams[0], "pwd")==0)
    {
    	  
  			getcwd(currentDir, bufferSize);
  		printf("You are in exec >> %s ", currentDir);
    }
    //execute command
    execvp(commandParams[0], commandParams);

    //added "cd" command

  }
  else{//parent process
   	 wait(NULL);
  }
  return 0;
}
//====================================================================================//
int execArgsSingleArrowRedirect(char** commandParams, char** fileName)
{
	printf("You are insdie SingleexecRedirect\n");

  //forking a child, pid is process identifier
  pid_t pid = fork();


  if(pid < 0){
    fprintf(stderr, "Fork Failed");
    return 1;
  }

  	int out = open(*fileName, O_RDWR | O_CREAT | O_TRUNC, 0644);//create file

	 if(pid == 0)//child process
  	{
  		close(1);//close standard output
  		dup(out);//creates a a copy of a descriptor, writes to file

    execvp(commandParams[0], commandParams);

    //added "cd" command
    if(strcmp(commandParams[0],"cd")==0)
    {
      chdir(commandParams[1]);//change directory to whatever
    }
  }

  else{//parent process
  	close(out);
   	wait(NULL);
  }
  return 0;
}
//=================================================================================//
int execArgsDoubleArrowRedirect(char** commandParams, char** fileName)
{
	printf("You are insdie DoubleExecRedirect\n");

  //forking a child, pid is process identifier
  pid_t pid = fork();


  if(pid < 0){
    fprintf(stderr, "Fork Failed");
    return 1;
  }

  	int out = open(*fileName, O_RDWR | O_CREAT | O_APPEND, 0644);//create file
	 if(pid == 0)//child process
  	{
	close(1);//close standard output
	dup(out);
    //execute command
    execvp(commandParams[0], commandParams);


    //added "cd" command
    if(strcmp(commandParams[0],"cd")==0)
    {
      chdir(commandParams[1]);//change directory to whatever
    }
  }

  else{//parent process
  	close(out);
   	wait(NULL);
  }
  return 0;
}
//======================================================================
int execArgsBackGround(char** commandParams)
{
	//printf("You are inside execArgs background!\n");
  //forking a child, pid is process identifier
  pid_t pid = fork();
  int* status;

  if(pid < 0){
    fprintf(stderr, "Fork Failed");
    return 1;
  }

	 if(pid == 0)//child process
  	{

  	setsid();
    //execute command
    execvp(commandParams[0], commandParams);


    //added "cd" command
    if(strcmp(commandParams[0],"cd")==0)
    {
      chdir(commandParams[1]);//change directory to whatever
    }

  
}
   else{//parent process
   	
   		//printf("You are in parent process!");
    waitpid(-1, NULL, WNOHANG);//-1 means wait for any but not all child, status is NULL, WNOHANG means parent returns immediately even if child is still doing something
   	//signal(SIGCHLD, SIG_IGN);
   		
   }
  return 0;
}

int execArgsReverseArrow(char** commandParams, char** fileName)
{
	printf("You are insdie ReverseArrowexec\n");

  //forking a child, pid is process identifier
  pid_t pid = fork();


  if(pid < 0){
    fprintf(stderr, "Fork Failed");
    return 1;
  }

  	
  	//open a file
  	int source = open(*fileName, O_RDWR , 0644);//create file

	 if(pid == 0)//child process
  	{
  	dup2(source, 0);//(oldFile, newfile) - 0 is standard input, so basically im displaying whats inside the first "file descriptor", in standard input which is cmd here

    execvp(commandParams[0], commandParams);

    //added "cd" command
    if(strcmp(commandParams[0],"cd")==0)
    {
    	printf("You are in cd");
      chdir(commandParams[1]);//change directory to whatever
    }
  }

  else{//parent process
  	close(source);
   	wait(NULL);
  }
  return 0;
}

int execArgsPiPe(char** commandParams, char** pipedParsedParams, char** fileName){


	printf("You are inside piped\n");

	pid_t pid = fork();
	int fileDescriptor[2];

	if(pipe(fileDescriptor != 0))
		printf("Error, can't create pipe!\n");

	if(pid<0)
		printf("Forking failed!\n");

	//create piping
	if(pid==0)
	{
		dup2(fileDescriptor[1], 1);
		close(fileDescriptor[0]);
		close(fileDescriptor[1]);
		execvp(commandParams[0], commandParams);
		error("Error with command 1");
		return 0;
	}
	else
	{
		dup2(fileDescriptor[0], 0);
		close(fileDescriptor[0]);
		close(fileDescriptor[1]);
		evecvp(pipedParsedParams[0], pipedParsedParams);
		error("Error with command 2");
		return 0;
	}

	return 0;
}

//====================================================================//
int main(){


  char userCommand[bufferSize];//array stores the whole command line
  char* commandParams[8];//array to store splited command
  char* pipedParsedParams[bufferSize];
  int singleArrow, doubleArrow, background, reverseArrow, pipe; 
  int argc;

  while(1){

    printCMP();

    //takes standard input and store it into userCommand, if this equals nothing
    //break the loop
    //Control C or Control D to exit
    if(fgets(userCommand, sizeof(userCommand), stdin)==NULL)
  	break;

    //fgets keeps "\n" when hit enter, so we have to
    if(userCommand[strlen(userCommand)-1]=='\n'){
    userCommand[strlen(userCommand)-1] = '\0'; 
	}

    if(strcmp(userCommand, "exit")==0){
      exit(0);
    }

    /////////////////////////////////parsing///////////////////////////////
   
    splitCMD(userCommand, commandParams, &argc, &singleArrow, &doubleArrow, &background, &reverseArrow, &pipe);

    //printf("Single Arrow is %d\n, Double Arrow is %d\n, Background process is %d\n, Reverse Arrow is %d\n, pipe is %d\n", singleArrow, doubleArrow, background, reverseArrow, pipe);

    //====================Output redirecting : Single Arrow and Double Arrow =======================================================//
    if(singleArrow==1)//single arrow detected
    {
    	char* fileName;
    	for(int i = 0;i < argc;i++){
    	if(strcmp(commandParams[i],">")==0){//if found ">"
    	commandParams[i]='\0'; 				//remove ">" so the child doesn't see this and cry 
 		fileName = commandParams[i+1];		//set the next element next to ">" as the output filename
 		//printf("File name is %s, in main\n", commandParams[i+1]);
 		singleArrow = 0;//reset the singleArrow boolean
 		break;
    		}
    	}
    	execArgsSingleArrowRedirect(commandParams, &fileName);
    }
    //=====================================================
    else if(doubleArrow==1){//double arrow detected

    	char* fileName;
    	for(int i = 0;i < argc;i++){
    	if(strcmp(commandParams[i],">>")==0){
    	commandParams[i]='\0';
    	fileName = commandParams[i+1];	
    	doubleArrow = 0;//set back to 0
    		}//end of if
    	}//end of for
    	execArgsDoubleArrowRedirect(commandParams, &fileName);
    }
    //===================================================
    else if(background==1){
    	for(int i = 0;i < argc;i++){
    	if(strcmp(commandParams[i],"&")==0){
    		commandParams[i]='\0';
    		background = 0;
    	}//end of if
    }//end of for
    execArgsBackGround(commandParams);
	}
	
	else if(reverseArrow==1){
		char* fileName;
    	for(int i = 0;i < argc;i++){
    	if(strcmp(commandParams[i],"<")==0){
    		commandParams[i]='\0';
    		fileName = commandParams[i+1];
    		reverseArrow = 0;//reset reverseArrow boolean
    	}//end of if
    }//end of for

    execArgsReverseArrow(commandParams, &fileName);
	}

	else if(pipe==1){
		char* fileName;

		//scenrio 1 ls/cat first



		//scenrio 2  > first
    	for(int i = 0;i < argc;i++){
    	if(strcmp(commandParams[i],"")==0){
    		commandParams[i]='\0';
    		fileName = commandParams[i+1];
    		reverseArrow = 0;//reset reverseArrow boolean

   		//scenrio 3 >> first



    	//secnrio 4 < first




    	}//end of if
    }//end of for




 execArgsReverseArrow(commandParams, &fileName);
	}



    else{//no arrow dected
    	execArgs(commandParams);
    }
    //=================================================================================================================================//

 	//showing input arguments
    for(int i =0;i<argc;i++)
     {
     	//if(commandParams[i]=="&");
       printf("%s", commandParams[i]);
     }

    
  }

  return 0;
}
