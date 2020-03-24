#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/wait.h> 


int main(int argc, char* argv[]){

    int x;
    if(fork()){//parent
        wait(&x);
        printf("\nParent & child terminated.");

    }
    else{
        execv("./execute",argv); 
    }

}
