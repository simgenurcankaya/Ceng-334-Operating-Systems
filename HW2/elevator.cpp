#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
#include "monitor.h"


    struct Person
{ 
    int weight;
    int initialFloor;
    int destinationFloor;
    int priority;
//    struct Person *nextPerson;
};

int num_floors,num_people, weight_capacity,person_capacity, travelTime, idleTime,inoutTime;
Person *people;

class ElevatorMonitor:public Monitor{
    
    int currentWeight;
    int direction; // 1 up -1 down 0 idle
    int npeopleinQ;
    int * destinations;
    int currentFloor;
    


    Condition canUse;
    Condition waitingPerson;
    struct clist {
        int cid; /* customer id */
        int  destination;
        struct clist *next;
    };

    clist *persons, *lastPerson; // queue for customer, last and first member


    public:
    ElevatorMonitor(): canUse(this), waitingPerson(this){
        currentWeight = 0;
        direction = 0 ;
        npeopleinQ = 0;
        currentFloor = 0;

    };


    int getDirection(){ return direction;}
    void setDirection(int x){direction= x;}

    int getCurrentWeight() {return currentWeight;}
    void setCurrentWeight(int x) {currentWeight = x;}

    int getCurrentFloor() {return currentFloor;}
    void setCurrentFloor(int x) { currentFloor = x;}

    void elevatorUp(){ currentFloor++; direction = 1;}
    void elevatorDown(){ currentFloor--; direction = -1;}

    void newPerson(int x){ //add xth person to the Q
    
    __synchronized__;
        //
    waitingPerson.wait();

    clist *newP = new clist;
    newP->cid = x;
    newP ->destination = people[x].destinationFloor;
    
    if(npeopleinQ == 0){
        persons = lastPerson = newP;
    }
    else{
        lastPerson ->next = newP;
        lastPerson = newP;
    }

    npeopleinQ++;

    waitingPerson.notifyAll();
    }

    

    

 };



ElevatorMonitor elMon;

int peopleServed = 0;

void elevatorController(){
    Person *peopleQ;
    int x = 0;
    int peopleWaiting = 0;


    while(peopleServed < num_people){
        if(elMon.getDirection() == 0){
            while(!peopleWaiting){
                sleep(idleTime);
            }
            Person temp = peopleQ[0];
            int y = temp.initialFloor;
            int z = temp.destinationFloor;
            if(z < y){
                //move up
                sleep(travelTime);
                elMon.elevatorUp();
            }
            else if(z > y){
                sleep(travelTime);
                elMon.elevatorDown();
            }
            else{
                printf("Invalid case.\n");
            }
            int waitingAtCurrentFloor = findPerson(elMon.getCurrentFloor());
            if( waitingAtCurrentFloor != -1 ){
                Person p = people[waitingAtCurrentFloor];
                if(((p.destinationFloor - p.initialFloor) * elMon.getDirection()) > 0  &&  (p.weight + elMon.getCurrentWeight() < weight_capacity)){
                    //asansore girebilir
                    p.initialFloor = -1;

                } 
            }
        }
        
    }


}

void personCallsTheElevator(int x){
    

}



int findPerson(int x){
    
    for(int i = 0; i<num_people; i++){
        if(people[i].initialFloor == x)
            return x;
    }
    return -1;
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
