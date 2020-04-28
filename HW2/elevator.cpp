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
    int pr;
    int priority;
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
    
    std::list<int> terkedilmislerList;
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
        if(direction == -1)
            destinationList.reverse();

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
        
        } //printf("Elevator goes up. Floor: %d \n",currentFloor);}
    void elevatorDown(){
        direction = -1; 
        printElevatorInfo();
        currentFloor--; 
         }  //printf("Elevator goes down. Floor: %d \n",currentFloor);}

    int getPeopleInQ(){ return npeopleinQ;}

    void newPerson(int x){ //add xth person to the Q
    
   // printf("Adding %d th person to the Q\n Currently adding %d\n",x,currentlyAdding);
        //

    while(currentlyAdding != 0){
     //   printf("waits condition\n");
        waitingPerson.wait();
    }
    currentlyAdding++;
   // printf("passes condition\n");

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
    people[x].priority = 1;
    //printf("Added new person to the Q.\nQ now has %d passangers.\n",npeopleinQ);

    updateDestination();
    printPersonInfo(x,1);
    //printf("Notiffuu");
    currentlyAdding--;
    waitingPerson.notify();
    }

    void updateDestination(){
        destinationList.clear();
        for(int i = 0; i<num_people;i++){
            if(people[i].priority == 1){
             //   printf("Person %d is in Q , initial floor %d\n", i,people[i].initialFloor);
                destinationList.push_back(people[i].initialFloor);
            }
            else if(people[i].priority == 2){
            //    printf("Person %d is in elevator , destination floor %d\n", i,people[i].destinationFloor);
                destinationList.push_back(people[i].destinationFloor);
            }
        }
    destinationList.sort();
    destinationList.unique();
    if( * destinationList.begin() == currentFloor)
        destinationList.pop_front();
    }

    void updateNumbers(){
        npeopleinQ = 0;
        nPeopleinElevator = 0;
        peopleServed =0;
        for(int i = 0; i<num_people;i++){
            if(people[i].priority == 1){
                npeopleinQ++;
            }
            else if(people[i].priority == 2){
                nPeopleinElevator++;
            }
            else if(people[i].priority == -1){
                peopleServed++;
            }
        if(terkedilmislerList.size()>0){
            terkedilmislerList.unique();
            terkedilmislerList.sort();
         //   printf("****  Terkedilmiş sayısı:  %d\n ", terkedilmislerList.size());
            for(std::list<int>::iterator i = terkedilmislerList.begin(); i != terkedilmislerList.end(); i++){

                printPersonInfo(*i,1);
             //   printf("priorty  : %d , peopleserved  %d \n", people[*i].priority, peopleServed);
                destinationList.push_back(people[*i].initialFloor);
                people[*i].priority = 1;
                npeopleinQ++;

            }

                destinationList.sort();
                destinationList.unique();
                terkedilmislerList.clear();
        }
        }

    //    printf(" Q : %d , E: %d , S : %d \n",npeopleinQ,nPeopleinElevator,peopleServed);

    }
    


    int findPersonOnFloor(int x){
        clist * temp = persons;
        for(int i = 0 ; i<npeopleinQ ; i++){
            Person p = people[temp->x];
            if(p.initialFloor == x && p.priority == 0){
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
       // printf("X : %d  pri: %d ,, %d %d %d ,, %d %d %d ,, %d %d \n",x,p.priority, p.destinationFloor,p.initialFloor,direction,p.weight,currentWeight,weight_capacity,nPeopleinElevator,person_capacity);
        if(p.priority != 1) return false; 
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
                nPeopleinElevator--;
                npeopleinQ--;

                currentWeight -= p.weight;
                
                printPersonInfo(p.id,3);
            }
        }
    //    printElevatorInfo();
        return true;
    }

    void newToElevator(int x){
      //  printf("adding %dth person to the elevator? \n",x);
        for(int i = 0 ; i< person_capacity; i++){
            printf("%d %d ,,\n",inElevator[i].priority ,nPeopleinElevator); 
            if(inElevator[i].priority != 1 ){
                //silinmiş ya da ilk
                inElevator[i] = people[x];
                nPeopleinElevator++;
                currentWeight += people[x].weight;
                people[x].priority = 1;
                destinationList.push_back(people[x].destinationFloor);
                destinationList.sort();
                destinationList.unique();
                //printf("Person enters the elevator\nElevator now has %d passangers.\n",nPeopleinElevator);
                printPersonInfo(x,2);
          //      printElevatorInfo();
                break;
            }
        }
    }
    int firstInQ(){
       // return persons->x;
        return * destinationList.begin();
    }

    void debugForSimge(){
        printf("Current elevator: %d, current Q: %d, first in Q: %d, destinationListSize: %d \n",nPeopleinElevator,npeopleinQ,firstInQ(),destinationList.size());
    }

    void addDest(int t){
        destinationList.push_back(t);
        destinationList.sort();
        destinationList.unique();
    }

    void yolcuAlmaVakti(){
        int change = 0;
        for(int i = 0 ; i < num_people; i++){
          //  printf("Person [%d] değenlendiriliyor ,",i);

            if(people[i].initialFloor == currentFloor && isEligableToEnter(i)){
                //printf("** onay aldı kat : %d ** \n",currentFloor); 
                printPersonInfo(i,2);
                people[i].priority = 2;
                currentWeight += people[i].weight;
                nPeopleinElevator ++;
                npeopleinQ--;
                change++;
            }
            else if(people[i].initialFloor == currentFloor){
           //     printf("** onay alamadı kat: %d **\n",currentFloor);
                //girmesi lazım ama giremiyo
                terkedilmislerList.push_back(i);
                terkedilmislerList.sort();
                terkedilmislerList.unique();    
            }
        }
        updateDestination();

        if(change>0){
          //  printf("yolcu alma: ");
            printElevatorInfo(); 
            change = 0;
        }
    }

    void yolcuBirakmaVakti(){
        int change= 0;
        for(int i = 0; i<num_people; i++){
            if(people[i].destinationFloor == currentFloor && people[i].priority == 2){
                change++;
                printPersonInfo(i,3);
                people[i].priority = -1;
                currentWeight -= people[i].weight;
                nPeopleinElevator--;
                peopleServed++;
            }
        }

        if(change >0){
            updateDestination();
          //  printf("yolcu bırakma: ");
            printElevatorInfo();
            change = 0;
        }
       

    }

    int getPeopleInE(){ return nPeopleinElevator;}

 };



ElevatorMonitor elMon;

bool whileConst = true;

int simge = 0;
void *elevatorController(void *){

    //printf("ElevatorController starts working\n");

    while(peopleServed< num_people && simge<5){
        if(elMon.getDirection() == 0){
            while(elMon.getPeopleInQ() == 0 && elMon.getPeopleInE() == 0 ){
                simge++;
                if(simge>4){
                    elMon.updateNumbers();
                    simge = 0;
                    break;

                    //peopleServed =100;
                }
                usleep(idleTime);
            }

            //elMon.debugForSimge();
            int y = elMon.firstInQ(); //gidilecek
            int x = elMon.getCurrentFloor();
         //   printf( "Y : %d , x : %d \n",y,x);
            if(y < 0){
                whileConst = false;
                break;
            }
            if(x < y){
                //move up
                usleep(travelTime);
                elMon.elevatorUp();
            }
            else if(x > y){
                usleep(travelTime);
                elMon.elevatorDown();
            }

            
            elMon.yolcuAlmaVakti();

            elMon.yolcuBirakmaVakti();

            if(peopleServed >= num_people){

            elMon.setDirection(0);
                break;
            }
            
            elMon.setDirection(0);
        }
        
    }
   // printf("Fin ? ");
    elMon.printElevatorInfo();
    exit(1);
    

}

void* passengerCreator(void * t){

    long x = (long) t;
    
    elMon.newPerson((int) x);

    
   // printf("New person %d created\n",x);

    //sleep(2);  

}

int main(){

    pthread_t *passengers, controller;
 

    scanf("%d %d %d %d %d %d %d", &num_floors, &num_people,&weight_capacity, &person_capacity, &travelTime, &idleTime, &inoutTime);

    //printf("%d %d %d %d %d %d %d\n", num_floors, num_people,weight_capacity, person_capacity, travelTime, idleTime, inoutTime);

    people= new Person[num_people];


    for(int i = 0 ; i<num_people; i++){
        Person temp;
        temp.id=i;
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
