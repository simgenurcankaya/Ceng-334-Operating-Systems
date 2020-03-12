#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<sys/socket.h>
#include "logging.h"
#include "message.h"
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)


typedef struct input
{
    char executable[20];
    int numberOfArgs;
    int args[20];
    /* data */
}input;

  
int main() 
{ 

    int starting_bid, minimum_increment,number_of_bidders;
    scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);

    printf("1 : %d, 2 : %d , 3: %d \n",starting_bid, minimum_increment, number_of_bidders);

    input bidders[number_of_bidders];
    
    for( int i = 0 ; i< number_of_bidders; i++){
        scanf("%s", (bidders[i].executable));
        scanf("%d",&bidders[i].numberOfArgs);
        for(int j = 0 ; j< bidders[i].numberOfArgs; j++){
            int dummy;
            scanf(" %d" ,&dummy);
            bidders[i].args[j] = dummy;
        }
    }

    for( int i = 0 ; i< number_of_bidders; i++){
        printf("%d :::  %s  ,, %d ,, %d\n",i,bidders[i].executable, bidders[i].numberOfArgs, bidders[i].args[0]);


    printf("NELER OLUYOR ayw");
    }


    printf("NELER OLUYOR");
    int fd[2]; //pipe
    pid_t p; 
    
    if (PIPE(fd)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 

    dup2(fd[0],0);
    dup2(fd[1],1);
  
    printf("Before fork \n");
    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process - Server
    else if (p > 0) 
    { 
        int cont = 1;
        // Write input string and close writing end of first 
        // while(cont){
        printf("Parent process \n");
        // }
        // // Wait for child to send a string 
        wait(NULL); 
  
        // Read string from child, print it and close 
        // reading end. 
    } 
  
    // child process 
    else
    { 
        execv(bidders[0].executable,bidders[0].args);
        printf("Called execv function");
        exit(0); 
    } 
} 
