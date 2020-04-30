#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include "monitor.h"
#include <list>


struct Person
{ 
    int id;
    int weight;
    int initialFloor;
    int destinationFloor;
    int status; // 0 initial, 1  in Q, 2 in Elevator, -1 DONE
    int priority;
    int direction; //1 up -1 down
//    struct Person *nextPerson;
};

int num_floors,num_people, weight_capacity,person_capacity, travelTime, idleTime,inoutTime;
Person *people;
int peopleServed = 0;


class ElevatorMonitor:public Monitor{
    
    int currentWeight;
    int direction; // 1 up -1 down 0 idle
    int npeopleinQ;
    int currentFloor;
    int nPeopleinElevator;
    int peopleServed;
    std::list<int> destinationList;

    Condition canAdd;
    Condition isAvailable;

    int currentlyAdding =0;

    public:
    ElevatorMonitor(): canAdd(this), isAvailable(this){
        currentWeight = 0;
        direction = 0;
        npeopleinQ = 0;
        nPeopleinElevator = 0;
        currentFloor = 0;
        currentlyAdding = 0;
        
    }

    bool isEligableToEnter(int x){
        if(direction == 0){
            return true;
        }
        if(people[x].direction == direction){
            if(direction == 1 && people[x].initialFloor >= currentFloor){
                return true;
            }
            else if(direction == -1 && people[x].initialFloor <= currentFloor){
                return true;
            }
        }
        else return false;
    }

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
        if(people[x].priority == 2)
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

    void updateDestination(){

        int t = destinationList.size();

        printf("\n Update Destination Function Before ** \n");
        for(std::list<int>::iterator i = destinationList.begin(); i != destinationList.end(); i++){
            --t;
            if(t == 0)
                printf("%d", *i);
            else
                printf("%d,", *i);
            
        }

        printf("\n");

        destinationList.clear();
        for(int i = 0; i<num_people;i++){
            if(people[i].status == 1 ){
                destinationList.push_back(people[i].initialFloor);
            }
            else if(people[i].status == 2){
                destinationList.push_back(people[i].destinationFloor);
            }
        }
    if(destinationList.size()>0){
        destinationList.sort();
        destinationList.unique();
        if( direction == 1 &&  * destinationList.begin() == currentFloor)
            destinationList.pop_front();
        if( direction == -1 &&  * destinationList.end() == currentFloor)
            destinationList.pop_back();
        }
        if(direction == -1)
            destinationList.reverse();


        t = destinationList.size();

        printf( "\n After:: \n" );
        for(std::list<int>::iterator i = destinationList.begin(); i != destinationList.end(); i++){
            --t;
            if(t == 0)
                printf("%d", *i);
            else
                printf("%d,", *i);
            
        }
        printf("\n");

        
    }

    void newPerson(int x){

        printf("inside new person funct %d\n",x);

        while(currentlyAdding != 0){
            canAdd.wait();
        }
        currentlyAdding++;

        if(npeopleinQ == 0 && nPeopleinElevator == 0){
            //ilk
            printf("First person to add %d\n",x);

            people[x].status = 1;
            direction = people[x].direction;
            destinationList.push_back(people[x].initialFloor);
            printf("Person %d : ",x);
            printPersonInfo(x,1);
            printElevatorInfo();
            npeopleinQ++;
        }

        else{

            //ilk değilse
            printf("Not first person to add %d\n",x);

            while(direction != people[x].direction && direction != 0){
                //aynı yolda degiller
                printf("Not in the same direction. %d Waiting..........\n",x);
                isAvailable.wait();
            }

            while(!isEligableToEnter(x)){
                printf("Not eligible. %d Waiting..........\n",x);
                isAvailable.wait();
            }
                
            printf("%d adding to the Q. \n",x);
            people[x].status = 1;
            printf("Person %d : ",x);
            updateDestination();
            printPersonInfo(x,1);
            printElevatorInfo();
            npeopleinQ++;
            
            
        }

        currentlyAdding--;
        canAdd.notify();

    }

    void moveUp(){

        currentFloor++;
        direction = 1;
        printf("Move up:  ");
        printElevatorInfo();
    }

    void MoveDown(){

        currentFloor--;
        direction  = -1;
        printf("Move down: ");
        printElevatorInfo();

    }

    bool canGetIn(int x){

        Person p = people[x];
        return ((p.weight + currentWeight < weight_capacity) && (nPeopleinElevator < person_capacity) );

    }
    void yolcuAlmaVakti(){


        for(int i = 0 ; i < num_people; i++){
            
            if(people[i].initialFloor == currentFloor){
                if(!canGetIn(i)){
                    printf("%dth cant get in to elevator\n",i);
                    people[i].status = 0;
                    continue;
                }
                else{
                printf("%dth CAN get in to elevator\n",i);
                people[i].status = 2;
                currentWeight += people[i].weight;
                nPeopleinElevator ++;
                npeopleinQ--;
                updateDestination();
                printPersonInfo(i,2);
                printElevatorInfo(); 
                }
            }
        }
    }

    void yolcuBirakmaVakti(){

        for(int i = 0; i<num_people; i++){
            if(people[i].destinationFloor == currentFloor && people[i].status == 2){
                people[i].status = -1;
                currentWeight -= people[i].weight;
                nPeopleinElevator--;
                peopleServed++;

            updateDestination();
            printPersonInfo(i,3);
            printElevatorInfo();
            }
        }
    
    }

    void controller(){


        while(peopleServed < num_people){
            while(destinationList.size() == 0){
                printf("Empty destination list, waiting & notifiying...\n");
                usleep(idleTime);
                isAvailable.notifyAll();
            }

            printf("Current floor = %d , Destination = %d\n",currentFloor,* destinationList.begin());
            if(currentFloor < * destinationList.begin()){
                usleep(travelTime);
                moveUp();
            }
            else if(currentFloor > * destinationList.begin()){
                usleep(travelTime);
                MoveDown();
            }
            printf("Elevator moveddddddd\n");
            if(nPeopleinElevator >0)
                yolcuBirakmaVakti();

            if(npeopleinQ > 0)
                yolcuAlmaVakti();
        }
    }

};

ElevatorMonitor elMon;

void *elevatorController(void *){
    printf("Calling controller\n");
    elMon.controller();

}

void* passengerCreator(void * t){

    long x = (long) t;
    printf("Creating passenger %d\n",x);
    
    while(people[x].status == 0){
        elMon.newPerson((int) x);

    }

}


int main(){

    pthread_t *passengers, controller;
 

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

    printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

    people= new Person[num_people];


    for(int i = 0 ; i<num_people; i++){
        Person temp;
        temp.id=i;
        scanf("%d %d %d %d ", &temp.weight,&temp.initialFloor, &temp.destinationFloor, &temp.priority);
        temp.status = 0;
        temp.direction = 1;
        if(temp.initialFloor > temp.destinationFloor)
            temp.direction = -1;
        people[i] = temp;

    }

    passengers = new pthread_t[num_people];
    printf("Input succesfully taken.\n");   
    for(int i = 0 ; i<num_people; i++){
        Person temp = people[i];
        //printf("%d %d %d %d\n", temp.weight,temp.initialFloor, temp.destinationFloor, temp.priority);
        pthread_create(&(passengers[i]), NULL, passengerCreator, (void *) i);

    }

    pthread_create(&controller, NULL, elevatorController, NULL);

    printf("Treads are created.\n");

    for (int i = 0; i < num_people; i++) {
        pthread_join(passengers[i], NULL);
    }

    pthread_join(controller, NULL);

    //printf("?\n");

    return 0;
}