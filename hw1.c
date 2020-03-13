#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<sys/socket.h>
#include<poll.h>
#include "logging.h"
#include "message.h"

#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)


int starting_bid, minimum_increment,number_of_bidders;
int pr1,pr2;    

typedef struct input
{
    char executable[20];
    int numberOfArgs;
    char args[20];
    /* data */
}input;

int execute(input);
int child(input x, int pipe[]);
int parent(int p1[],int p2[]);


int main(){
    scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);

    printf("1 : %d, 2 : %d , 3: %d \n",starting_bid, minimum_increment, number_of_bidders);

    input bidders[number_of_bidders];
    for(int i = 0 ; i<number_of_bidders; i++){
        printf("Scanning...\n");
        scanf(" %s", bidders[i].executable);
        scanf(" %[^\n]s",bidders[i].args);
    }

    printf("Scanning ended\n");
    int pipe1[2];
    int pipe2[2];
    
    int w;

    

    for( int i = 0 ; i< number_of_bidders; i++){
        printf("%d ::  %s ,, %s \n",i,bidders[i].executable, bidders[i].args);
    }

    PIPE(pipe1);
    PIPE(pipe2);

    printf("Pipe created & dupped \n");

    
    printf("Starts forking...\n");
    
    
    if (pr1 = fork()) {
		if (pr2 = fork()) {
            printf("Parent waiting... \n");

            parent(pipe1,pipe2);
			wait(&w);
			wait(&w);

            printf("Parent waiting ended...\n Exterminating.");
		} else {
            printf("Child1 \n");
			return child(bidders[0],pipe1);
		}
	} else {
            printf("Child2\n");
			return child(bidders[1], pipe2);
	}

    return 0;
}

int execute(input x){
    char* args[3];
    args[0] = x.executable;
    args[1] = "2000";
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

int parent(int p1[],int p2[]){

	struct pollfd pfd[2] = {{ p1[0], POLLIN, 0}, { p2[0], POLLIN, 0}} ;
    cm* client_message;
    client_message = (cm*) malloc(sizeof(cm));
    int n = 2,r;
    
    int current_bid = starting_bid;
    int current_bidder = -1;
    int counter = n;

    while (counter > 0){ // (pfd[0].fd >= 0 || pfd[1].fd >= 0 ){ /* one still open */

		poll(pfd, n, 0);  /* no timeout*/
		for (int i = 0; i < n; i++){
			if (pfd[i].revents && POLLIN) {
				r = read(pfd[i].fd, client_message, sizeof(cm));                
				if (r == 0){
                    counter--;		/* EOF */
					pfd[i].fd = -1;   /* poll() ignores pollfd item if fd is negative */
                }
                else
                {
                    sm * server_message =(sm*) malloc(sizeof(sm));

                    if(client_message->message_id == 1 ){ //client connect
                            ii * in_info = (ii*) malloc(sizeof(ii));
                            in_info->pid = pr1;
                            in_info->type = 1;
                            cmp * client_parameters = (cmp*) malloc(sizeof(cmp));
                            client_parameters ->delay = 2000;
                            in_info ->info = * client_parameters;
                            print_input(in_info,i);

                            server_message->message_id = 1;
                            smp * server_parameters = (smp*) malloc(sizeof(smp));
                            cei * connection_info = (cei*) malloc(sizeof(cei));
                            connection_info->client_id = i;
                            connection_info->current_bid = current_bid;
                            connection_info->minimum_increment = minimum_increment;
                            connection_info->starting_bid = starting_bid;
                            server_parameters->start_info = *connection_info;
                            server_message->params  = *server_parameters;

                            write(pfd[i].fd,server_message,sizeof(sm));
                            
                            oi * out_info = (oi*) malloc(sizeof(oi));
                            out_info ->pid  = pr1;
                            out_info->type = 1;
                            out_info ->info = * server_parameters;
                            print_output(out_info,i);
                    }

                    else if(client_message->message_id ==2){
                        


                        int incoming_bid = client_message->params.bid;
                        server_message->message_id = 2;
                        smp * server_parameters = (smp*) malloc(sizeof(smp));
                        bi* bid_info = (bi*) malloc(sizeof(bi));
                        bid_info->current_bid = current_bid;
                        if(incoming_bid < starting_bid) // lower than starting
                            bid_info->result = 1;
                        else if(incoming_bid < current_bid)
                            bid_info->result = 2;
                        else if(current_bid - incoming_bid < minimum_increment)
                            bid_info->result = 3;
                        else if(incoming_bid > current_bid)// güzel teklif
                            bid_info->result = 0;        
                        else
                        {
                            printf("Error on client2");
                            exit(1);
                        }
                                                        
                        server_message->params = *server_parameters;
                        server_parameters->result_info = *bid_info;
                        write(pfd[i].fd,server_message,sizeof(sm));

                    }

                    else if(client_message->message_id = 3){
                        counter--;
                        smp * server_parameters = (smp*) malloc(sizeof(smp));
                        wi * winner_info = (wi*) malloc(sizeof(wi));
                        winner_info->winner_id = current_bidder;
                        winner_info->winning_bid = current_bid;
                        server_parameters->winner_info   = * winner_info;
                        server_message ->params = *server_parameters;

                        if(counter == 0){ // time to finish
                            for(int x = 0 ; x<n ; x++){
                                write(pfd[i].fd,server_message,sizeof(sm));
                            }
                        }
                    }    
			    }
            }
        }
	}
}