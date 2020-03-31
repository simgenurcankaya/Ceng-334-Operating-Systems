#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include <string.h>
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
    char exec[20];
    int number_of_inp;
    int arg[20];

}input;


 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}  

 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}  

int main(){

    scanf("%d %d %d", &starting_bid, &minimum_increment, &number_of_bidders);

    printf("start : %d, increment : %d , #bidders: %d \n",starting_bid, minimum_increment, number_of_bidders);
 
    input bidders[number_of_bidders];
     for(int i = 0 ; i<number_of_bidders; i++){
        printf("Scanning...\n");
        input temp;
        int s;
        scanf(" %s", temp.exec);
        scanf(" %d",&temp.number_of_inp);
        
        s =  temp.number_of_inp ;
        for(int j = 0; j<s;j++){
            scanf("%d" , &(temp.arg[j]));
        }
        //scanf(" %[^\n]s",temp.input);
        bidders[i] = temp;
    }
 
    printf("Bidders ended\n");

    for(int i = 0 ; i< number_of_bidders; i++){
        printf("Bidder %i : exec: %s , inp : '%d'\n",i,bidders[i].exec,bidders[i].arg[0]);
    }


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
        //     for(int j  = 0; j<i;j++)
        //         close(pipes[j][1]);

            printf("----------------- Inside child i : %d , bidders[i] : %d \n", i , bidders[i].number_of_inp);

            int number_of_args = bidders[i].number_of_inp +2;
            char* args[number_of_args];
            args[0] = bidders[i].exec;
            printf(" ------------------ ? ---------------- \n");
            for(int ttyx = 1; ttyx <= bidders[i].number_of_inp; ttyx++){
                printf("Stringe cevriliyor : %d\n", bidders[i].arg[ttyx-1]);
               // sprintf( args[ttyx],"%d", bidders[i].arg[ttyx-1]);
                char* f;
                itoa(bidders[i].arg[ttyx-1],f);
                printf("scanned : %s \n",f);
                args[ttyx] = f;
            }
            args[number_of_args-1] = NULL;
            
            
            printf("now executing %s \n",args[0]);
            printf("----------------- you will never be alone \n");
            close(pipe[0]);
            dup2(pipe[1],0);
            dup2(pipe[1],1);
		    close(pipe[1]);
            execv(args[0],args); 
            exit(1);
                
        }

        printf("Child %d 's PID : %d \n",i,pid);


        //parent does nothing in the loop
    }

    // only parent continues here
    pid_t pidx = getpid();
    printf("\n\ninside parent \n");

   

    struct pollfd read_pipes[number_of_bidders];
    struct pollfd write_pipes[number_of_bidders];
    int isFirst[number_of_bidders];
    for(int x = 0; x<number_of_bidders; x++){
        //struct pollfd pdf ;
        // pdf.fd = pipes[x][0];
        // pdf.events = POLLIN;
        // pdf.revents = 0;
        struct pollfd pdf = {pipes[x][0],POLLIN,0};
        struct pollfd pfd = {pipes[x][1],POLLIN,0};
        read_pipes[x] = pdf;
        write_pipes[x] = pfd;
        isFirst[x]= 0;
        
    }

     printf("before pipe stuff \n");

    for(int x = 0; x < number_of_bidders; x++){
        close(pipes[x][1]);
		//dup2(pipes[x][0],0);
		//dup2(pipes[x][0],1);
		//close(pipes[x][0]);
    }


    printf("After pipe stuff \n");

    
    //client_message = (cm) malloc(sizeof(cm));
    int n ,r;
    n = number_of_bidders;
    int current_bid = starting_bid;
    int current_bidder = -1;
    int counter = n;
    int simge;

  //  sleep(2);


    //TODO send message correctly
    
    while (counter > 0){ // (pfd[0].fd >= 0 || pfd[1].fd >= 0 ){ /* one still open */

        //printf("Polling...\n");
		poll(read_pipes, n, 0);  /* no timeout*/
		for (int i = 0; i < n; i++){
			if (read_pipes[i].revents && POLLIN) {

                printf("i = %d ,current bet %d, current bidder %d, n %d, counter %d \n", i ,current_bid, current_bidder, n, counter);
                
                cm clientmessage;

                r = read(read_pipes[i].fd, &clientmessage, sizeof(cm));
                if(r<0){
                    printf("r read error %d \n",r);
                    exit(1);
                }  

                if (r == 0){
                    printf("r == 0 \n");
                    counter--;		/* EOF */
					read_pipes[i].fd = -1;   /* poll() ignores pollfd item if fd is negative */
                }
                else
                {
                    printf("r = %d \n",r);
                    sm * server_message =(sm*) malloc(sizeof(sm));

                    if(clientmessage.message_id == 1 ){ //client connect
        
                            printf("connect message received\n");
                            ii * in_info = (ii*) malloc(sizeof(ii));
                            in_info->pid = pr1;
                            in_info->type = 1;
                            cmp * client_parameters = (cmp*) malloc(sizeof(cmp));
                            client_parameters ->delay = bidders[i].arg[0];
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

                    else if(clientmessage.message_id ==2){
                        
                        printf("client bid recived\n");


                        int incoming_bid = clientmessage.params.bid;
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

                    else if(clientmessage.message_id == 3){
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
                        printf("Error occured. Message id received : %d\n", clientmessage.message_id);
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