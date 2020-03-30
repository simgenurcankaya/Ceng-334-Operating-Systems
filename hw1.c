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
/* 
    input bidders[number_of_bidders];
    for(int i = 0 ; i<number_of_bidders; i++){
        //printf("Scanning...\n");
        scanf(" %s", bidders[i].executable);
        scanf(" %[^\n]s",bidders[i].args);
    }
 */
    printf("Bidders ended\n");

    pid_t pid = getpid();

  printf("Parent pid: %d \n", pid);

    int ** pipes = malloc(number_of_bidders* sizeof(int*));

    for(int i = 0 ; i< number_of_bidders ; i++){

        pid_t pidx = getpid();

        printf("inside for, i = %d , pid: %d \n",i, pidx);

        int * pipe = malloc(2* sizeof(int));
        pipes[i] = pipe;


        PIPE(pipe);

        int pid = fork();

        if (pid == -1){
            perror("fork error");
            exit(0);
        }

        if(pid == 0){ // Child
            for(int j  = 0; j<i;j++)
                close(pipes[j][1]);

            printf("executing...\n");  
            dup2(pipe[0],0);
            dup2(pipe[1],1);
//            close(pipe[0]);
            char* args[3];
            args[0] = "./PatternBidder";
            args[1] = "4 100 0 1 2";
            args[2] = NULL;

            printf("now executing %s \n",args[0]);
            execv(args[0],args); 
                
        }

        printf("Child %d 's PID : %d \n",i,pid);


        //parent does nothing in the loop
    }

    // only parent continues here
    pid_t pidx = getpid();
    printf("\n\ninside parent \n");
    struct pollfd read_pipes[number_of_bidders];
    struct pollfd write_pipes[number_of_bidders];
    for(int x = 0; x<number_of_bidders; x++){
        //struct pollfd pdf ;
        // pdf.fd = pipes[x][0];
        // pdf.events = POLLIN;
        // pdf.revents = 0;
        struct pollfd pdf = {pipes[x][0],POLLIN,0};
        struct pollfd pfd = {pipes[x][1],POLLIN,0};
        read_pipes[x] = pdf;
        write_pipes[x] = pfd;
    }

    
    //client_message = (cm) malloc(sizeof(cm));
    int n ,r;
    n = number_of_bidders;
    int current_bid = starting_bid;
    int current_bidder = -1;
    int counter = n;
    int simge;
    while (counter > 0){ // (pfd[0].fd >= 0 || pfd[1].fd >= 0 ){ /* one still open */

		//poll(read_pipes, n, 0);  /* no timeout*/
		for (int i = 0; i < n; i++){
			if (1){//read_pipes[i].revents && POLLIN) {
                printf("i = %d ,current bet %d, current bidder %d, n %d, counter %d \n", i ,current_bid, current_bidder, n, counter);
                
                cm batu;
				r = read(pipes[0][0] , &batu, sizeof(cm));     
                printf("Client message id:  %d\n",batu.message_id); 
                if (r == 0){
                    printf("r == 0 \n");
                    counter--;		/* EOF */
					read_pipes[i].fd = -1;   /* poll() ignores pollfd item if fd is negative */
                }
                else
                {
                    printf("r = %d \n",r);
                    sm * server_message =(sm*) malloc(sizeof(sm));

                    if(batu.message_id == 1 ){ //client connect
        
                            printf("connect message received\n");
                            ii * in_info = (ii*) malloc(sizeof(ii));
                            in_info->pid = pr1;
                            in_info->type = 1;
                            cmp * client_parameters = (cmp*) malloc(sizeof(cmp));
                            client_parameters ->delay = 500;
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

                            write(pipes[i][1],server_message,sizeof(sm));
                            
                            oi * out_info = (oi*) malloc(sizeof(oi));
                            out_info ->pid  = pr1;
                            out_info->type = 1;
                            out_info ->info = * server_parameters;
                            print_output(out_info,i);
                    }

                    else if(batu.message_id ==2){
                        
                        printf("client bid recived\n");


                        int incoming_bid = batu.params.bid;
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
                            //printf("Error on client2");
                            exit(1);
                        }
                                                        
                        server_message->params = *server_parameters;
                        server_parameters->result_info = *bid_info;
                        write(pipes[i][1],server_message,sizeof(sm));

                    }

                    else if(batu.message_id == 3){
                        printf("client finish received\n");
                        counter--;
                        smp * server_parameters = (smp*) malloc(sizeof(smp));
                        wi * winner_info = (wi*) malloc(sizeof(wi));
                        winner_info->winner_id = current_bidder;
                        winner_info->winning_bid = current_bid;
                        server_parameters->winner_info   = * winner_info;
                        server_message ->params = *server_parameters;

                        if(counter == 0){ // time to finish
                            for(int x = 0 ; x<n ; x++){
                                write(pipes[i][1],server_message,sizeof(sm));
                            }
                        }
                    }
                    else{
                        printf("Error occured. Message id received : %d\n", batu.message_id);
                    }    
			    }
            }
        }
	}

    printf("counter ended. waiting for child \n");

    for(int i = 0 ; i < number_of_bidders ; i++)
        wait(&simge);

    printf("Child ended \n");
    exit(1);

}




int execute(input x){

    //printf("execution ended");
    return 0;
}