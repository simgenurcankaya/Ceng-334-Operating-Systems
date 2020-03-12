#include<stdio.h> 
#include<stdlib.h> 


int main(int argc, char *argv[]){

    printf("\n --- Now inside Simge --- \n");

    printf("Number of arguments :  %d \n", argc);
    
    printf("First argument is : %s ", argv[1]);

    printf("\n --- Leaving Simge --- \n");
    return 1;
}
