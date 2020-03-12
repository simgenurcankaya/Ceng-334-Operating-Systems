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
    char args[20];
    /* data */
}input;

int execute(input);
int child(input x, int pipe[]);

int main(){
    int starting_bid, minimum_increment,number_of_bidders;
    scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);

    printf("1 : %d, 2 : %d , 3: %d \n",starting_bid, minimum_increment, number_of_bidders);

    input bidders[number_of_bidders];
    for(int i = 0 ; i<number_of_bidders; i++){
        printf("Scanning...\n");
        scanf(" %s", bidders[i].executable);
        scanf(" %[^\n]s",bidders[i].args);

    }

    int pipe1[2];
    int pipe2[2];
    int w;

    for( int i = 0 ; i< number_of_bidders; i++){
        printf("%d ::  %s ,, %s \n",i,bidders[i].executable, bidders[i].args);
    }

    if (fork()) {
		if (fork()) {
            printf("Parent waiting... \n");
			wait(&w);
			wait(&w);

            printf("Parent waiting ended...\n Exterminating.");
		} else {
			return child(bidders[0],pipe1);
		}
	} else {
			return child(bidders[1], pipe2);
	}

    return 0;
}

int execute(input x){
    char* args[3];
    args[0] = x.executable;
    args[1] = x.args;
    args[2] = NULL;

    printf("now executing %s \n",args[0]);
    execv(args[0],args); 
    printf("execution ended");
    return 0;
}


int child(input x, int pipe[] ){
    printf("Inside child \n" );
    execute(x);

}