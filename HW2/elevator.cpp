#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include "monitor.h"
#include <list>

struct Person
{ 
    int weight;
    int initialFloor;
    int destinationFloor;
    int pr;
    int priority;
//    struct Person *nextPerson;
};

int num_floors,num_people, weight_capacity,person_capacity, travelTime, idleTime,inoutTime;
Person *people;

class ElevatorMonitor:public Monitor{
    
    int currentWeight;
    int direction; // 1 up -1 down 0 idle
    int npeopleinQ;
    int currentFloor;
    int nPeopleinElevator;

    std::list<int> destinationList;
    
    
    Condition canUse;
    Condition waitingPerson;

    int currentlyAdding;
    struct clist {
       // Person person;
        int x;
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
        inElevator = new Person[person_capacity];
        nPeopleinElevator = 0;
        currentlyAdding = 0;
        
    };



    void printElevatorInfo(){

        printf("Elevator (");
        if(direction == 1)
            printf("Moving-up, "); 
        else if(direction == -1)
            printf("Moving-down, ");
        else
            printf("Idle, ");
        printf("%d, %d, %d -> ",currentWeight, nPeopleinElevator, currentFloor);

        int t = destinationList.size();

        for(std::list<int>::iterator i = destinationList.begin(); i != destinationList.end(); i++){
            --t;
            if(t == 0)
                printf("%d", *i);
            else
                printf("%d,", *i);
            
        }
        printf(")\n");
    }

    void printPersonInfo(int x, int type){ // type 1 req, 2 enter, 3 left
        printf("Person (%d, ",x);
        if(people[x].pr == 2)
            printf("hp");
        else
            printf("lf");
        printf(", %d -> %d, %d) ",people[x].initialFloor, people[x].destinationFloor, people[x].weight);

        if(type == 1)
            printf("made a request\n");
        else if(type == 2)
            printf("entered the elevator\n");
        else if(type == 3)
            printf("has left the elevator\n");
        
        //printElevatorInfo();
    
    }

    int getDirection(){ return direction;}
    void setDirection(int x){direction= x;}

    int getCurrentWeight() {return currentWeight;}
    void setCurrentWeight(int x) {currentWeight = x;}

    int getCurrentFloor() {return currentFloor;}
    void setCurrentFloor(int x) { currentFloor = x;}

    void elevatorUp(){
        direction = 1;
        printElevatorInfo();
        currentFloor++;  
        if(*destinationList.begin() == currentFloor){
            destinationList.pop_front();
        }
        
        printElevatorInfo(); 
        
        } //printf("Elevator goes up. Floor: %d \n",currentFloor);}
    void elevatorDown(){
        direction = -1; 
        printElevatorInfo();
        currentFloor--; 
        printElevatorInfo();
        if(*destinationList.begin() == currentFloor){
            destinationList.pop_front();
        }
         }  //printf("Elevator goes down. Floor: %d \n",currentFloor);}

    int getPeopleInQ(){ return npeopleinQ;}

    void newPerson(int x){ //add xth person to the Q
    
    //printf("Adding %d th person to the Q\n Currently adding %d\n",x,currentlyAdding);
        //

    while(currentlyAdding != 0){
        //printf("waits condition\n");
        waitingPerson.wait();
    }
    currentlyAdding++;
    //printf("passes condition\n");

    clist *newP = new clist;
    newP ->x = x;

    if(npeopleinQ == 0){
        persons = lastPerson = newP;
    }
    else{
        lastPerson ->next = newP;
        lastPerson = newP;
    }
    npeopleinQ++;
    //printf("Added new person to the Q.\nQ now has %d passangers.\n",npeopleinQ);

    destinationList.push_back(people[x].initialFloor);
    destinationList.sort();
    destinationList.unique();
    printPersonInfo(x,1);
    waitingPerson.notifyAll();
    }

    

    int findPersonOnFloor(int x){
        clist * temp = persons;
        for(int i = 0 ; i<npeopleinQ ; i++){
            Person p = people[temp->x];
            if(p.initialFloor == x){
                //printf("There is eligable person (%d) in the floor %d \n",i,x);
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
                p.priority == -1;
                npeopleinQ--;
                printPersonInfo(10,3);
            }
        }
        printElevatorInfo();
        return true;
    }

    void newToElevator(int x){
        //printf("adding %dth person to the elevator? \n",x);
        for(int i = 0 ; i< person_capacity; i++){
            if(inElevator[i].priority == -1 || nPeopleinElevator == 0){
                //silinmiş ya da ilk
                inElevator[i] = people[x];
                nPeopleinElevator++;
                currentWeight += people[x].weight;
                people[x].priority = -1;
                destinationList.push_back(people[x].destinationFloor);
                destinationList.sort();
                destinationList.unique();
                //printf("Person enters the elevator\nElevator now has %d passangers.\n",nPeopleinElevator);
                printPersonInfo(x,2);
                printElevatorInfo();
            }
        }
    }
    int firstInQ(){
        return persons->x;
    }


 };



ElevatorMonitor elMon;

int peopleServed = 0;

void *elevatorController(void *){

    //printf("ElevatorController starts working\n");

    while(peopleServed < num_people){
        if(elMon.getDirection() == 0){
            while(elMon.getPeopleInQ() == 0 ){
               // //printf("Elevator controller waits.\n");
                usleep(idleTime);
            }

          //  //printf("** Someone in the Q ** \n");
            //printf("First in Q = %d \n",elMon.firstInQ());
            //printf("babus ");
            //elMon.printElevatorInfo();
            int y = people[elMon.firstInQ()].initialFloor;
            int z = people[elMon.firstInQ()].destinationFloor;
            int x = elMon.getCurrentFloor();
            //printf("y = %d , z = %d \n",y,z);   
            if(x< y){
                //move up
                usleep(travelTime);
                elMon.elevatorUp();
            }
            else if(x > y){
                usleep(travelTime);
                elMon.elevatorDown();
            }
            else{
                //printf("Else case.\n");
                if(z > x){
                    elMon.elevatorUp();
                }
                else if(z < x){
                    elMon.elevatorDown();
                }
                else{
                    //printf("Invalid case2\n");
                }

            }
            
            if(elMon.isEligableToLeave()){
                peopleServed++;
                usleep(inoutTime);
                //printf("people left the elevator\n");
            }

            int waitingAtCurrentFloor = elMon.findPersonOnFloor(elMon.getCurrentFloor());

            if(waitingAtCurrentFloor != -1 ){
                if(elMon.isEligableToEnter(waitingAtCurrentFloor)){
                    //asansore girebilir
                    people[waitingAtCurrentFloor].priority = -1;
                    elMon.newToElevator(waitingAtCurrentFloor);
                    usleep(inoutTime);

                } 
            }
            elMon.setDirection(0);
        }
        
    }

}

void* passengerCreator(void * t){

    long x = (long) t;
    
    elMon.newPerson((int) x);

    //sleep(2);   
    //printf("New person %d created\n",x);
}

int main(){

    pthread_t *passengers, controller;
 

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

    //printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

    people= new Person[num_people];


    for(int i = 0 ; i<num_people; i++){
        Person temp;
        scanf("%d %d %d %d ", &temp.weight,&temp.initialFloor, &temp.destinationFloor, &temp.pr);
        temp.priority = 0;
        people[i] = temp;

    }

    passengers = new pthread_t[num_people];
    //printf("Input succesfully taken.\n");   
    for(int i = 0 ; i<num_people; i++){
        Person temp = people[i];
        //printf("%d %d %d %d\n", temp.weight,temp.initialFloor, temp.destinationFloor, temp.priority);
        pthread_create(&(passengers[i]), NULL, passengerCreator, (void *) i);

    }

    pthread_create(&controller, NULL, elevatorController, NULL);

    //printf("Treads are created.\n");

    for (int i = 0; i < num_people; i++) {
        pthread_join(passengers[i], NULL);
    }

    pthread_join(controller, NULL);

    //printf("?\n");

    return 0;
}
