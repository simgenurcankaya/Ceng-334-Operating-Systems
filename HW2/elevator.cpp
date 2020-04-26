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
    int nPeopleinElevator;
    
    Condition canUse;
    Condition waitingPerson;
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
        

    };


    int getDirection(){ return direction;}
    void setDirection(int x){direction= x;}

    int getCurrentWeight() {return currentWeight;}
    void setCurrentWeight(int x) {currentWeight = x;}

    int getCurrentFloor() {return currentFloor;}
    void setCurrentFloor(int x) { currentFloor = x;}

    void elevatorUp(){ currentFloor++; direction = 1;}
    void elevatorDown(){ currentFloor--; direction = -1;}

    int getPeopleInQ(){ return npeopleinQ;}

    void newPerson(int x){ //add xth person to the Q
    
    __synchronized__;
        //
    waitingPerson.wait();

    clist *newP = new clist;
    newP->person = people[x];
    
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

    int findPersonOnFloor(int x){
        clist * temp = persons;
        for(int i = 0 ; i<npeopleinQ ; i++){
            if(temp->person.initialFloor == x){
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
            }
        }
    }
    Person firstInQ(){
        return persons->person;
    }

 };



ElevatorMonitor elMon;

int peopleServed = 0;

void elevatorController(){

    while(peopleServed < num_people){
        if(elMon.getDirection() == 0){
            while(elMon.getPeopleInQ() == 0 ){
                sleep(idleTime);
            }
            Person temp = elMon.firstInQ();
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
            
            if(elMon.isEligableToLeave()){
                peopleServed++;
                printf("people left the elevator\n");
            }

            int waitingAtCurrentFloor = elMon.findPersonOnFloor(elMon.getCurrentFloor());

            if(waitingAtCurrentFloor != -1 ){
                if(elMon.isEligableToEnter(waitingAtCurrentFloor)){
                    //asansore girebilir
                    people[waitingAtCurrentFloor].initialFloor = -1;
                    elMon.newToElevator(waitingAtCurrentFloor);

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
