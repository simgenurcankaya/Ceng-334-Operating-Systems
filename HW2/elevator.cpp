#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
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
    int nPeopleinElevator;
    
    Condition canUse;
    Condition waitingPerson;

    int currentlyAdding;
    struct clist {
        Person person;
        struct clist *next;
    };

    clist *persons, *lastPerson; // queue for customer, last and first member
    Person * inElevator;

    public:
    ElevatorMonitor(): canUse(this), waitingPerson(this){
        currentWeight = 0;
        direction = 0 ;
        npeopleinQ = 0;
        currentFloor = 0;
        destinations = new int[person_capacity];
        for(int i = 0 ; i<person_capacity; i++){
            destinations[i] = -1;
        }
        inElevator = new Person[person_capacity];
        nPeopleinElevator = 0;
        currentlyAdding = 0;
        
    };


    int getDirection(){ return direction;}
    void setDirection(int x){direction= x;}

    int getCurrentWeight() {return currentWeight;}
    void setCurrentWeight(int x) {currentWeight = x;}

    int getCurrentFloor() {return currentFloor;}
    void setCurrentFloor(int x) { currentFloor = x;}

    void elevatorUp(){ currentFloor++; direction = 1; printf("Elevator goes up. Floor: %d",currentFloor);}
    void elevatorDown(){ currentFloor--; direction = -1; printf("Elevator goes down. Floor: %d",currentFloor);}

    int getPeopleInQ(){ return npeopleinQ;}

    void newPerson(int x){ //add xth person to the Q
    
    printf("Adding %d th person to the Q\n Currently adding %d\n",x,currentlyAdding);
        //

    while(currentlyAdding != 0){
        printf("waits condition\n");
        waitingPerson.wait();
    }
    currentlyAdding++;
    printf("passes condition\n");

    clist *newP = new clist;
    printf("HUMMMMMMMMMMMMMMMMMMMMMMMMM\n");
    Person p;
    printf("AMK");
    p.destinationFloor = people[x].destinationFloor;
    p.initialFloor = people[x].initialFloor;
    p.priority = 1;
    p.weight = people[x].weight;
    printf("Derdimden kaçakkk\n");
    newP->person = p;
    
    printf("sknt");
    
    if(npeopleinQ == 0){
        persons = lastPerson = newP;
    }
    else{
        lastPerson ->next = newP;
        lastPerson = newP;
    }
    printf("ıdkdıdk");
    npeopleinQ++;
    printf("Added new person to the Q.\nQ now has %d passangers.\n",npeopleinQ);

    waitingPerson.notify();
    }

    int findPersonOnFloor(int x){
        clist * temp = persons;
        for(int i = 0 ; i<npeopleinQ ; i++){
            if(temp->person.initialFloor == x){
                printf("There is eligable person in the floor %d \n",x);
                return i;
            }
            temp = temp->next;
        }
        return -1;
    }

    bool isEligableToEnter(int x){
        //xth person eliable to the elevator
        Person p = people[x];
        return (((p.destinationFloor - p.initialFloor) * direction ) > 0  &&  (p.weight + currentWeight < weight_capacity) && (nPeopleinElevator < person_capacity) );
    }

    bool isEligableToLeave(){
        int x = currentFloor;
        //any person eligable to leave at floor x
        if(nPeopleinElevator == 0){
            return false;
        }

        for(int i = 0; i<person_capacity; i++){
            Person p = inElevator[i];
            if(p.destinationFloor == currentFloor){
                p.destinationFloor == -1;
                npeopleinQ--;
            }
        }
        return true;
    }

    void newToElevator(int x){
        
        for(int i = 0 ; i< person_capacity; i++){
            if(inElevator[i].destinationFloor == -1 || nPeopleinElevator == 0){
                //silinmiş ya da ilk
                inElevator[i] = people[x];
                nPeopleinElevator++;
                printf("Person enters the elevator\nElevator now has %d passangers.\n",nPeopleinElevator);
            }
        }
    }
    Person firstInQ(){
        return persons->person;
    }

 };



ElevatorMonitor elMon;

int peopleServed = 0;

void *elevatorController(void *){

    printf("ElevatorController starts working\n");

    while(peopleServed < num_people){
        if(elMon.getDirection() == 0){
            while(elMon.getPeopleInQ() == 0 ){
               // printf("Elevator controller waits.\n");
                usleep(idleTime);
            }

            printf("** Someone in the Q ** \n");
            Person temp = elMon.firstInQ();
            int y = temp.initialFloor;
            int z = temp.destinationFloor;
            if(z < y){
                //move up
                usleep(travelTime);
                elMon.elevatorUp();
            }
            else if(z > y){
                usleep(travelTime);
                elMon.elevatorDown();
            }
            else{
                printf("Invalid case.\n");
            }
            
            if(elMon.isEligableToLeave()){
                peopleServed++;
                usleep(inoutTime);
                printf("people left the elevator\n");
            }

            int waitingAtCurrentFloor = elMon.findPersonOnFloor(elMon.getCurrentFloor());

            if(waitingAtCurrentFloor != -1 ){
                if(elMon.isEligableToEnter(waitingAtCurrentFloor)){
                    //asansore girebilir
                    people[waitingAtCurrentFloor].initialFloor = -1;
                    elMon.newToElevator(waitingAtCurrentFloor);
                    usleep(inoutTime);

                } 
            }
        }
        
    }

}

void* passengerCreator(void * t){

    long x = (long) t;
    
    elMon.newPerson((int) x);

    //sleep(2);   
    printf("New person %d created\n",x);
}

int main(){

    pthread_t *passengers, controller;
 

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

    printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

    Person people[num_people];


    for(int i = 0 ; i<num_people; i++){
        Person temp;
        scanf("%d %d %d %d ", &temp.weight,&temp.initialFloor, &temp.destinationFloor, &temp.priority);
        people[i] = temp;
    }

    passengers = new pthread_t[num_people];
    printf("Input succesfully taken.\n");   
    for(int i = 0 ; i<num_people; i++){
        Person temp = people[i];
        printf("%d %d %d %d\n", temp.weight,temp.initialFloor, temp.destinationFloor, temp.priority);
        pthread_create(&(passengers[i]), NULL, passengerCreator, (void *) i);

    }

    pthread_create(&controller, NULL, elevatorController, NULL);

    printf("Treads are created.\n");

    for (int i = 0; i < num_people; i++) {
        pthread_join(passengers[i], NULL);
    }

    pthread_join(controller, NULL);

    printf("?\n");

    return 0;
}
