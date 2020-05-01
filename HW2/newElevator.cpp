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
    std::list<int> destinationList;

    Condition canAdd;
    Condition thereIsChange;

    int currentlyAdding ;

    public:
    ElevatorMonitor(): canAdd(this), thereIsChange(this){
        currentWeight = 0;
        direction = 0;
        npeopleinQ = 0;
        nPeopleinElevator = 0;
        currentFloor = 0;
        currentlyAdding = 0;
        
    }

    bool isEligableToEnter(int x){

       // printf("IsEligible %d, direction = %d , PDirec %d , nPeopleinE = %d, nPeopleinQ = %d, initialF = %d , currentF = %d \n",x,direction,people[x].direction,nPeopleinElevator,npeopleinQ,people[x].initialFloor,currentFloor);
        if(direction == 0 || (npeopleinQ == 0 && nPeopleinElevator == 0)){
            return true;
        }
        
        if(people[x].direction == direction){
            if(direction == 1 && people[x].initialFloor >= currentFloor){
                return true;
            }
            else if(direction == -1 && people[x].initialFloor <= currentFloor){
                return true;
            }
            else{

                return false;
            }
        }
        else{
         //   printf("false\n");
            return false;
        }
        return false;
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
        
        if(direction == -1)
            destinationList.reverse();


        for(std::list<int>::iterator i = destinationList.begin(); i != destinationList.end(); i++){
            --t;
             if(*i == currentFloor) 
                 continue;
            if(t == 0)
                printf("%d", *i);
            else
                printf("%d,", *i);
            
        }

        if(direction == -1)
            destinationList.reverse();

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
        
        if(direction == 0){
            direction = -1;
            if(currentFloor <= * destinationList.begin()){
                direction = 1;
            }
        }

        

        t = destinationList.size();
    }
    else direction = 0;
        
    }

    void newPerson(int x){



    }

    void startAdding(int x){

        while(currentlyAdding || !isEligableToEnter(x)){
            sleep(5/100);
        }
        currentlyAdding++;

        npeopleinQ++;
        people[x].status = 1;
        destinationList.push_back(people[x].initialFloor);
        updateDestination();
        printPersonInfo(x,1);
        printElevatorInfo();

        currentlyAdding--;

    }

    void finishAdding(int x){
       
        //canAdd.notifyAll();
        // printf("----- inside finished adding\n");

        while(people[x].status != -1){

            // printf("----- Controlling status... \n");
            
            if(people[x].status == 0){
                // printf("----- Status is zero %d\n",x);

                startAdding(x);
            }
            else{
                // printf("----- sleeping %d\n",x);
              //  thereIsChange.wait();
              sleep(1);
            }
        }

        // printf("----- Finished adding %d \n",x);
    }

    void moveUp(){

        currentFloor++;
        direction = 1;
     //   printf("Move up:  ");
        printElevatorInfo();
    }

    void MoveDown(){

        currentFloor--;
        direction  = -1;
    //    printf("Move down: ");

        printElevatorInfo();

    }

    bool canGetIn(int x){

        Person p = people[x];
        return ((p.weight + currentWeight <= weight_capacity) && (nPeopleinElevator <= person_capacity) );

    }
    void yolcuAlmaVakti(){

        for(int i = 0 ; i < num_people; i++){
            
            if(people[i].initialFloor == currentFloor && people[i].status == 1){
                if(!canGetIn(i)){
                //    printf("%dth cant get in to elevator\n",i);
                    people[i].status = 0;
                    npeopleinQ--;
                    updateDestination();
                  //  thereIsChange.notifyAll();
                    continue;
                }
                else{
            //    printf("%dth CAN get in to elevator\n",i);
                people[i].status = 2;
                currentWeight += people[i].weight;
                nPeopleinElevator ++;
                npeopleinQ--;
              //  thereIsChange.notifyAll();
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
               // thereIsChange.notifyAll();

            updateDestination();
            printPersonInfo(i,3);
            printElevatorInfo();
            }
        }

        
    
    }

    void yeniYolcuAvcisi(){
        int added = 0;
        for(int i  = 0 ; i< num_people ; i++){
            if(people[i].status == 0 ){
                if(added == 0){
                    newPerson(i);
                    added++;
                }
                else{
                    newPerson(i);
                    added++;
                    }

            }
        }
    }

    void controller(){

        while(peopleServed < num_people){
            while(destinationList.size() == 0){
                 usleep(idleTime);
            //    printf("Notified\n");
                sleep(1);
            }

           // printf("Current floor = %d , Destination = %d\n",currentFloor,* destinationList.begin());
            if(currentFloor < * destinationList.begin()){
                usleep(travelTime);
                moveUp();
            }
            else if(currentFloor > * destinationList.begin()){
                usleep(travelTime);
                MoveDown();
            }

            if(nPeopleinElevator >0){
                yolcuBirakmaVakti();
            }

            if(npeopleinQ > 0){
                yolcuAlmaVakti();
            }
            if(peopleServed == num_people){
                break;
            }

            if(npeopleinQ == 0 && nPeopleinElevator == 0){
                direction = 0;
            }
        }
    }

};

ElevatorMonitor elMon;

void *elevatorController(void *){
    elMon.controller();

}

void* passengerCreator(void * t){

    int x = (int)(long) t;
    
    elMon.startAdding(x);
   // elMon.newPerson(x);
    elMon.finishAdding(x);
}


int main(){

    pthread_t *passengers, controller;
 

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

   // printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

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
  //  printf("Input succesfully taken.\n");   
    for(int i = 0 ; i<num_people; i++){
        Person temp = people[i];
        //printf("%d %d %d %d\n", temp.weight,temp.initialFloor, temp.destinationFloor, temp.priority);
        pthread_create(passengers+i, NULL, passengerCreator, (void *) i);
        if(i == 0)
            pthread_create(&controller, NULL, elevatorController, NULL);

    }



    for (int i = 0; i < num_people; i++) {
        pthread_join(passengers[i], NULL);
    }

    pthread_join(controller, NULL);

    //printf("?\n");

    return 0;
}