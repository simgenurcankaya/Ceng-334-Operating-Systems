#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
#include "monitor.h"

class ElevatorMonitor:public Monitor{
    
    int currentWeight;
    int direction; // 1 up -1 down 0 idle
    int peopleinQ;
    int currentFloor;
    


    Condition canUse;

    public:
    ElevatorMonitor(): canUse(this) {
        currentWeight = 0;
        direction = 0 ;
        peopleinQ = 0;
        currentFloor = 0;

    }
};


typedef struct Person
{ 
    int weight;
    int initialFloor;
    int destinationFloor;
    int priority;
};


ElevatorMonitor elMon;

int num_floors,num_people, weight_capacity,person_capacity, travelTime, idleTime,inoutTime;
Person *people;

void elevatorController(){
    Person *peopleQ;
    int peopleServed = 0;


    while(peopleServed < num_people){
        if(elMon.)
    }


}


int main(){

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

    printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

    Person people[num_people];

    for(int i = 0 ; i<num_people; i++){
        Person temp;
        scanf("%d %d %d %d ", &temp.weight,&temp.initialFloor, &temp.destinationFloor, &temp.priority);
        people[i] = temp;
    }

    for(int i = 0 ; i<num_people; i++){
        Person temp = people[i];
        printf("%d %d %d %d\n", temp.weight,temp.initialFloor, temp.destinationFloor, temp.priority);
    }

    elevatorController();


    return 1;
}
