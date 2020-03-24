#include<stdio.h> 
#include<stdlib.h> 


int main(int argc, char* argv[]){

    if(argc != 3){
        printf("Wrong number of arguments. Terminating...");
        return 0;
    }

    int a,b;

    printf("Arg1 : %s , Arg2 : %s \n",argv[1],argv[2]);

    a = atoi(argv[1]);
    b = atoi(argv[2]);

    printf("Summation is %d. Terminating...",a+b); 
    return 1;
}
