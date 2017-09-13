#include "list.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFSIZE 50
#define MAXSIZE 1000

LIST* highreadyq;
LIST* normalreadyq;
LIST* lowreadyq;
LIST* blockq;
LIST* running;

int pidcounter = 1;
Process process[MAXSIZE];
Semaphore semaphore[5];


int searchlist(const void *firstarg, const void *secondarg){
    if (firstarg == secondarg){
	return 1;
    }
    return 0;
}

int idsearchlist(const Process *firstarg, const Process *secondarg){
    if (firstarg->ID == secondarg->ID){
	return 1;
    }
    return 0;
}

void createcommand(char *input){
    if(pidcounter>=MAXSIZE){
	printf("Can not create any more processes");
	return;
    }
    process[pidcounter].ID = pidcounter;
    process[pidcounter].sendmes = NULL;
    process[pidcounter].commsid = -1;
    process[pidcounter].recvmes = NULL;
    process[pidcounter].semaph = -1;

    if(input[2] == '0'){
	process[pidcounter].queue = high;
	ListAppend(highreadyq,&process[pidcounter]);
	printf("Created a high priority process with id = %d\n",process[pidcounter].ID);

	Process* temp = ListRemove(running);
	if(temp->ID == 0){
	    ListAdd(running,&process[pidcounter]);
	    printf("process id: %d is now running\n",process[pidcounter].ID);
	}
	else{
	    ListAdd(running,temp);
	}
	pidcounter++;
    }

    else if(input[2] == '1'){
	process[pidcounter].queue = normal;
	ListAppend(normalreadyq,&process[pidcounter]);
	printf("Created a normal priority process with id = %d\n",process[pidcounter].ID);

	Process* temp = ListRemove(running);
	if(temp->ID == 0){
	    ListAdd(running,&process[pidcounter]);
	    printf("process id: %d is now running\n",process[pidcounter].ID);
	}
	else{
	    ListAdd(running,temp);
	}
	pidcounter++;
    }

    else if(input[2] == '2'){
	process[pidcounter].queue = low;
	ListAppend(lowreadyq,&process[pidcounter]);
	printf("Created a low priority process with id = %d\n",process[pidcounter].ID);

	Process* temp = ListRemove(running);
	if(temp->ID == 0){
	    ListAdd(running,&process[pidcounter]);
	    printf("process id: %d is now running\n",process[pidcounter].ID);
	}
	else{
	    ListAdd(running,temp);
	}
	pidcounter++;
    }

    else{
	printf("Failure to create process\n");
    } 
    return;
}

//Fork command, it forks the running process
void forkcommand(){
    Process* temp = ListFirst(running);

    //checks if the running process is the init
    if(temp->ID == 0){
 	printf("Can not fork the init process\n");
	return;
    }

    //checks if it is a high priority process
    if(temp->queue == high){
	ListAppend(highreadyq,&process[temp->ID]);
	printf("Forking Process id: %d to high priority q\n", temp->ID);
    } 

    //checks if it is a normal priority process   
    else if(temp->queue == normal){
	ListAppend(normalreadyq,&process[temp->ID]);
	printf("Forking Process id: %d to normal priority q\n", temp->ID);
    }  

    //checks if it is a low priority process
    else if(temp->queue == low){
	ListAppend(lowreadyq,&process[temp->ID]);
	printf("Forking Process id: %d to low priority q\n", temp->ID);
    }  

    //Can not find the right queue
    else{
	printf("Error forking the process\n");
    }
}

int chartoint(char *input, int *length){
    int identity;
    if(isdigit(input[3])){
	if(isdigit(input[4])){
	    char toint[3] = {input[2],input[3],input[4]};
	    identity = atoi(toint);
	    *length = 3;
	}
	else{
	    char toint[2] = {input[2],input[3]};
	    identity = atoi(toint);
	    *length = 2;
	}
    }
    else{
        char toint[1] = {input[2]};
        identity = atoi(toint);
	*length = 1;
    }
    return identity;
}

//Kill command, takes in input of number which kills pid of up to 3 digit id
void killcommand(char *input){
    if(isdigit(input[2])){
	if(input[2] == '0'){
	    if((ListCount(highreadyq) != 0)||(ListCount(normalreadyq) != 0)||(ListCount(lowreadyq) != 0)||(ListCount(blockq) != 0)){
		printf("Can't kill init process, other processes are still on queues\n");
		return;
	    }
	    else{
		ListRemove(running);
		return;
	    }
    	}
	int fill;
	int identity = chartoint(input,&fill);
	bool flag = false;

	Process* temp = ListFirst(running);
	if (temp->ID == identity){
	    ListRemove(running);
	    printf("Removed process: %d from running\n",temp->ID);
	    flag = true;
	}

	//Set all lists to first item
	ListFirst(highreadyq);
	ListFirst(normalreadyq);
	ListFirst(lowreadyq);
	ListFirst(blockq);

	//if process is in high priority queue remove it
    	if(ListSearch(highreadyq, idsearchlist, &process[identity]) != NULL){
	    
	    temp = ListRemove(highreadyq);
	    printf("Removing process: %d from high priority q\n",temp->ID);
        }

        //if process is in normal priority queue remove it
        else if(ListSearch(normalreadyq, idsearchlist, &process[identity]) != NULL){
	    temp = ListRemove(normalreadyq);
	    printf("Removing process: %d from normal priority q\n",temp->ID);
        }

        //if process is in low priority queue remove it
        else if(ListSearch(lowreadyq, idsearchlist, &process[identity]) != NULL){
	    temp = ListRemove(lowreadyq);
	    printf("Removing process: %d from low priority q\n",temp->ID);
        }
	//if process is in the block queue remove it
        else if(ListSearch(blockq, idsearchlist, &process[identity]) != NULL){
	    temp = ListRemove(blockq);
	    printf("Removing process: %d from block q\n",temp->ID);
        }
	else{
	    printf("Process ID: %d does not exist\n",identity);
	}
	if (flag == true){
	    //if there is a process in highreadyq
	    if(ListCount(highreadyq) != 0){
		ListFirst(highreadyq);
		temp = ListRemove(highreadyq);
		ListAdd(running,temp);
		ListAppend(highreadyq,temp);
		printf("Process ID: %d is now running\n", temp->ID);
	    }
	    //if there is a process in normalreadyq
	    else if(ListCount(normalreadyq) != 0){
		ListFirst(normalreadyq);
		temp = ListRemove(normalreadyq);
		ListAdd(running,temp);
		ListAppend(normalreadyq,temp);
		printf("Process ID: %d is now running\n", temp->ID);
	    }
	    //if there is a process in lowreadyq
	    else if(ListCount(lowreadyq) != 0){
		ListFirst(lowreadyq);
		temp = ListRemove(lowreadyq);
		ListAdd(running,temp);
		ListAppend(lowreadyq,temp);
		printf("Process ID: %d is now running\n", temp->ID);
	    }
	    //If init is only process that can be run
	    else{
		ListAdd(running,&process[0]);
	    }
	}

    }
    else{
	printf("Invalid input \n");
    }
}

void exitcommand(){
    //Removal from ready queue
    Process* temp = ListRemove(running);

    //If init was the running process
    if(temp->ID == 0){
	//Still values being blocked
	if(ListCount(blockq)!=0){
	    printf("Still values on the blocked qs\n");
	    ListAdd(running,temp);
	}

	//No more processes
	else{
	    printf("Init has been killed, Ending the program\n");
	}
	return;
    }

    printf("Process ID: %d has been removed as the running process\n", temp->ID);

    //Set all items to first item
    ListFirst(highreadyq);
    ListFirst(normalreadyq);
    ListFirst(lowreadyq);
   
    //if process is in high priority queue remove it
    if(ListSearch(highreadyq, searchlist, temp) != NULL){
	Process* temp = ListRemove(highreadyq);
	printf("Removing process: %d from high priority q\n",temp->ID);
    }

    //if process is in normal priority queue remove it
    else if(ListSearch(normalreadyq, searchlist, temp) != NULL){
	Process* temp = ListRemove(normalreadyq);
	printf("Removing process: %d from normal priority q\n",temp->ID);
    }

    //if process is in low priority queue remove it
    else if(ListSearch(lowreadyq, searchlist, temp) != NULL){
	Process* temp = ListRemove(lowreadyq);
	printf("Removing process: %d from low priority q\n",temp->ID);
    }

    //Can not find the process
    else{
	printf("Error removing process\n");
    }

    //If high priority queue is not empty, run the next process
    if(ListCount(highreadyq) != 0){
	ListFirst(highreadyq);
	temp = ListRemove(highreadyq);
	ListAdd(running, temp);
	ListAppend(highreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If normal priority queue is not empty, run the next process
    else if (ListCount(normalreadyq) != 0){
	ListFirst(normalreadyq);
	temp = ListRemove(normalreadyq);
	ListAdd(running, temp);
	ListAppend(normalreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If low priority queue is not empty, run the next process
    else if (ListCount(lowreadyq) != 0){
	ListFirst(lowreadyq);
	temp = ListRemove(lowreadyq);
	ListAdd(running, temp);
	ListAppend(lowreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //Run the init process
    else{
	ListAdd(running,&process[0]);
	printf("Process: 0(init) is now running\n");
    }
    return;
}

void quantumcommand(){
    Process* temp = ListRemove(running);
    printf("Process ID: %d has had its time quantum expire\n", temp->ID);
	
    if(temp->ID == 0){
	ListAdd(running,&process[0]);
	printf("Process: 0(init) is now running\n");
	return;
    }

    if(ListCount(highreadyq) != 0){
	ListFirst(highreadyq);
	temp = ListRemove(highreadyq);
	ListAdd(running, temp);
	ListAppend(highreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    else if (ListCount(normalreadyq) != 0){
	ListFirst(normalreadyq);
	temp = ListRemove(normalreadyq);
	ListAdd(running, temp);
	ListAppend(normalreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    else if (ListCount(lowreadyq) != 0){
	ListFirst(lowreadyq);
	temp = ListRemove(lowreadyq);
	ListAdd(running, temp);
	ListAppend(lowreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    else{
	printf("There has been an error\n");
    }
    return;
}

void sendcommand(char *input){
    Process* temp = ListFirst(running);
    if(temp->ID == 0){
	printf("Can not message with the init process!\n");
	return;
    }

    temp = ListRemove(running);
    int offset;
    int identity = chartoint(input,&offset);
    char msg[40];
    offset = offset+3;
    int i=0;
    while((i < 40) && (input[offset] != '\n')){
	msg[i]=input[offset+i];
	i++;
    }
    temp->sendmes = msg;
    temp->commsid = identity;
    temp->semaph = -1;
    temp->recvmes = NULL;
    Process* search;
    if(temp->queue == high){
	search = ListFirst(highreadyq);
  	while (temp != search){
	    search = ListNext(highreadyq);
	}
	printf("Removing Process: %d from high priority q\n",temp->ID);
	ListRemove(highreadyq);
	temp->queue = highblock;
	
    }
    else if(temp->queue == normal){
	search = ListFirst(normalreadyq);
  	while (temp != search){
	    search = ListNext(normalreadyq);
	}
	printf("Removing Process: %d from normal priority q\n",temp->ID);
	ListRemove(normalreadyq);
	temp->queue = normalblock;
    }
    else if(temp->queue == low){
	search = ListFirst(lowreadyq);
  	while (temp != search){
	    search = ListNext(lowreadyq);
	}
	printf("Removing Process: %d from low priority q\n",temp->ID);
	ListRemove(lowreadyq);
	temp->queue = lowblock;
    }
    else{
	printf("Error finding queue\n");
	return;
    }
    printf("Sending process id: %d the message: \"%s\", now waiting for a reply\n",temp->commsid,temp->sendmes);
    ListAppend(blockq,temp);

    search = ListFirst(blockq);
    while((search != NULL)&&(search->ID != temp->commsid)){
	search = ListNext(blockq);
    }

    if (search != NULL){
	if(search->commsid==-1){
	    ListRemove(blockq);
	    search->commsid = temp->ID;
	    search->recvmes = temp->sendmes;
	    search->sendmes = NULL;
	    search->semaph = -1;
	    printf("Found Process: %d on blocked q, Which has received message \"%s\" ",search->ID,search->recvmes);
	    if(search->queue = highblock){
		search->queue = high;
		ListAppend(highreadyq,search);
		printf("Placed process: %d back in high ready q",search->ID);
	    }
	    else if(search->queue = normalblock){
		search->queue = normal;
		ListAppend(normalreadyq,search);
		printf("Placed process: %d back in normal ready q",search->ID);
	    }
	    else if(search->queue = lowblock){
		search->queue = low;
		ListAppend(lowreadyq,search);
		printf("Placed process: %d back in low ready q", search->ID);
	    }
	    printf("\n");
	}
    }

    if(ListCount(highreadyq) != 0){
	ListFirst(highreadyq);
	temp = ListRemove(highreadyq);
	ListAdd(running, temp);
	ListAppend(highreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If normal priority queue is not empty, run the next process
    else if (ListCount(normalreadyq) != 0){
	ListFirst(normalreadyq);
	temp = ListRemove(normalreadyq);
	ListAdd(running, temp);
	ListAppend(normalreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If low priority queue is not empty, run the next process
    else if (ListCount(lowreadyq) != 0){
	ListFirst(lowreadyq);
	temp = ListRemove(lowreadyq);
	ListAdd(running, temp);
	ListAppend(lowreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //Run the init process
    else{
	ListAdd(running,&process[0]);
	printf("Process: 0(init) is now running\n");
    }

    return;
}

void receivecommand(){
    Process* temp = ListFirst(running);
    if(temp->ID == 0){
	printf("Can not message with the init process!\n");
	return;
    }

    char *receiver = "~";

    temp->sendmes = NULL;
    temp->recvmes = receiver;
    temp->semaph = -1;
    temp->commsid = -1;

    temp = ListRemove(running);
    printf("Removing process: %d from running q\n", temp->ID);

    Process* search;
    if (temp->queue == high){
	temp->queue = highblock;
	search = ListFirst(highreadyq);
	while (search->ID != temp->ID){
	    search = ListNext(highreadyq);
	}
	printf("Removing process: %d from high ready q\n",temp->ID);
	ListRemove(highreadyq);
    }
    else if (temp->queue == normal){
	temp->queue = normalblock;
	search = ListFirst(normalreadyq);
	while (search->ID != temp->ID){
	    search = ListNext(normalreadyq);
	}
	printf("Removing process: %d from normal ready q\n",temp->ID);
	ListRemove(normalreadyq);
    }
    else if (temp->queue == low){
	temp->queue = lowblock;
	search = ListFirst(lowreadyq);
	while (search->ID != temp->ID){
	    search = ListNext(lowreadyq);
	}
	printf("Removing process: %d from low ready q\n",temp->ID);
	ListRemove(lowreadyq);
    }
    else{
	printf("could not find proper queue");
	return;
    }

    ListAppend(blockq,temp);
    printf("Adding process: %d to the blocked q, and waiting to receive\n",temp->ID);

    search = ListFirst(blockq);
    while((search != NULL) && (search->commsid != temp->ID)){
	search = ListNext(blockq);
    }
    
    //IF there is a message waiting for the current process to receive on blocked q
    if (search != NULL){
	ListLast(blockq);
	ListRemove(blockq);
	temp->commsid = search->ID;
	temp->recvmes = search->sendmes;
	temp->semaph = -1;
	temp->sendmes = NULL;

	printf("queue: %d\n",temp->queue);

	printf("process: %d, has received a message from process id: %d, sent message \"%s\" Loading process back onto ",temp->ID,temp->commsid,temp->recvmes);
	if(temp->queue == highblock){
	    temp->queue = high;
	    ListAppend(highreadyq,temp);
	    printf("high ready q\n");
	}
	else if(temp->queue == normalblock){
	    temp->queue = normal;
	    ListAppend(normalreadyq,temp);
	    printf("normal ready q\n");
	}
	else if(temp->queue == lowblock){
	    temp->queue = low;
	    ListAppend(lowreadyq,temp);
	    printf("low ready q\n");
	}
	else{
	    printf("ERROR finding queue\n");
	    return;
	}
    }

    if(ListCount(highreadyq) != 0){
	ListFirst(highreadyq);
	temp = ListRemove(highreadyq);
	ListAdd(running, temp);
	ListAppend(highreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If normal priority queue is not empty, run the next process
    else if (ListCount(normalreadyq) != 0){
	ListFirst(normalreadyq);
	temp = ListRemove(normalreadyq);
	ListAdd(running, temp);
	ListAppend(normalreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //If low priority queue is not empty, run the next process
    else if (ListCount(lowreadyq) != 0){
	ListFirst(lowreadyq);
	temp = ListRemove(lowreadyq);
	ListAdd(running, temp);
	ListAppend(lowreadyq, temp);
	printf("Process ID: %d is now running\n", temp->ID);
    }

    //Run the init process
    else{
	ListAdd(running,&process[0]);
	printf("Process: 0(init) is now running\n");
    }

}

void replycommand(char *input){
    Process* temp = ListFirst(running);
    if(temp->ID == 0){
	printf("You can not message with process 0 (init)\n");
	return;
    }

    int offset;
    int identity = chartoint(input,&offset);
    char msg[40];
    offset = offset+3;
    int i=0;
    while((i < 40) && (input[offset] != '\n')){
	msg[i]=input[offset+i];
	i++;
    }
    temp->sendmes = msg;
    temp->commsid = identity;
    temp->semaph = -1;
    temp->recvmes = NULL;
    Process* search = ListFirst(blockq);
    while((search != NULL) && (temp->commsid != search->ID)){
	search = ListNext(blockq);
    }
    if (search == NULL){
	temp->sendmes = NULL;
	printf("Could not find the process: %d in the blocked q\n", temp->commsid);
	temp->commsid = -1;
    }
    else{
	printf("Process: %d, Sent reply message to process: %d consisting of message: \"%s\" and removed it from the blocked q\n",temp->ID,search->ID,search->recvmes);
	ListRemove(blockq);
	search->recvmes = temp->sendmes;
	search->semaph = -1;
	search->commsid = temp->ID;
	
	if(search->queue == highblock){
	    search->queue = high;
	    ListAppend(highreadyq,search);

	}
	else if (search->queue == normalblock){
	    search->queue = normal;
	    ListAppend(normalreadyq,search);
	}
	else if (search->queue == lowblock){
	    search->queue = low;
	    ListAppend(lowreadyq,search);
	}
    }
    return;
}

void newsemcommand(char *input){
    char num = input[2];
    int id = atoi(&num);
    if ((id < 0) || (id > 4)){
	printf("Error with id \n");
	return;
    }
    if (semaphore[id].val != -1){
	printf("Semaphore: %d has already been created\n",id);
	return;
    }

    int* fill;
    int value = chartoint(&input[2],fill);
    printf("Created semaphore: %d, with value: %d\n",id,value);
    semaphore[id].val = value;
    return;
}

void sempcommand(char *input){
    char num = input[2];
    int id = atoi(&num);
    if ((id < 0) || (id>4)){
	printf("Error with id \n");
	return;
    }
    Process* temp = ListFirst(running);
    if(temp->ID == 0){
	printf("You can not P a semaphore while in the init process\n");
	return;
    }
    if (semaphore[id].val == -1){
	printf("The semaphore: %d has not yet been created\n",id);
	return;
    }
    else if (semaphore[id].val == 0){
	temp = ListRemove(running);
	temp->commsid = -1;
	temp->semaph = id;
	temp->sendmes = NULL;
	temp->recvmes = NULL;
	printf("Process: %d has been removed from the running q and placed in blocked q\n",temp->ID);

	if(temp->queue == high){
	    temp->queue = semhighblock;
	    Process* search = ListFirst(highreadyq);
	    while((search!=NULL)&&(search->ID != temp->ID)){
		search = ListNext(highreadyq);
	    }
	    if(search==NULL){
		printf("Error Finding the value in high ready q\n");
		return;
	    }
	    else{
		printf("Removed process: %d from high ready q\n",search->ID);
		ListRemove(highreadyq);
	    }
	}
	else if(temp->queue == normal){
	    temp->queue = semnormalblock;
	    Process* search = ListFirst(normalreadyq);
	    while((search!=NULL)&&(search->ID != temp->ID)){
		search = ListNext(normalreadyq);
	    }
	    if(search==NULL){
		printf("Error Finding the value in high ready q\n");
		return;
	    }
	    else{
		printf("Removed process: %d from normal ready q\n",search->ID);
		ListRemove(normalreadyq);
	    }
	}
	else if(temp->queue == low){
	    temp->queue = semlowblock;
	    Process* search = ListFirst(lowreadyq);
	    while((search!=NULL)&&(search->ID != temp->ID)){
		search = ListNext(lowreadyq);
	    }
	    if(search==NULL){
		printf("Error Finding the value in high ready q\n");
		return;
	    }
	    else{
		printf("Removed process: %d from low ready q\n",search->ID);
		ListRemove(lowreadyq);
	    }
	}
	else{
	    printf("Error Finding the proper ready queue\n");
	    return;
	}

	ListAppend(blockq,temp);

	//If high priority queue is not empty, run the next process
	if(ListCount(highreadyq) != 0){
	    ListFirst(highreadyq);
	    temp = ListRemove(highreadyq);
	    ListAdd(running, temp);
	    ListAppend(highreadyq, temp);
	    printf("Process ID: %d is now running\n", temp->ID);
	}

        //If normal priority queue is not empty, run the next process
	else if (ListCount(normalreadyq) != 0){
	    ListFirst(normalreadyq);
	    temp = ListRemove(normalreadyq);
	    ListAdd(running, temp);
	    ListAppend(normalreadyq, temp);
	    printf("Process ID: %d is now running\n", temp->ID);
	}

	//If low priority queue is not empty, run the next process
	else if (ListCount(lowreadyq) != 0){
	    ListFirst(lowreadyq);
	    temp = ListRemove(lowreadyq);
	    ListAdd(running, temp);
	    ListAppend(lowreadyq, temp);
	    printf("Process ID: %d is now running\n", temp->ID);
	}

	//Run the init process
	else{
	    ListAdd(running,&process[0]);
	    printf("Process: 0(init) is now running\n");
	}
    }
    else{
	semaphore[id].val--;
	printf("Semaphore id: %d has been decremented to: %d\n",id,semaphore[id].val);
    }
}

void semvcommand(char *input){
    char num = input[2];
    int id = atoi(&num);
    if ((id < 0) || (id>4)){
	printf("Error with id \n");
	return;
    }
    
    if (semaphore[id].val == -1){
	printf("The semaphore: %d has not yet been created\n",id);
    }
    else if(semaphore[id].val != 0){
	semaphore[id].val++;
	printf("The semaphore: %d has been incremented and is now: %d\n",id,semaphore[id].val);
    }
    else{
	Process* temp = ListFirst(blockq);
	while ((temp!=NULL)&&(temp->semaph!=id)){
	    temp = ListNext(blockq);
	}
	if (temp==NULL){
	    semaphore[id].val++;
	    printf("The semaphore: %d has been incremented and is now: %d\n",id,semaphore[id].val);
	}
	else{
	    ListRemove(blockq);
	    temp->semaph = -1;
	    printf("Process: %d has been taken off the blocked q\n",temp->ID);
	    if(temp->queue == semhighblock){
		temp->queue = high;
		ListAppend(highreadyq,temp);
		printf("Process: %d has been placed onto high ready q\n",temp->ID);
	    }
	    else if(temp->queue == semnormalblock){
		temp->queue = normal;
		ListAppend(normalreadyq,temp);
		printf("Process: %d has been placed onto normal ready q\n",temp->ID);
	    }
	    else if (temp->queue == semlowblock){
 		temp->queue = low;
		ListAppend(lowreadyq,temp);
		printf("Process: %d has been placed onto low ready q\n",temp->ID);
	    }
	    else{
		printf("Error finding proper queue\n");
	    }
	    Process* test = ListFirst(running);
	    if (test->ID == 0){
		ListRemove(running);
		printf("Process: %d is now on the running q\n",temp->ID);
		ListAdd(running,temp);
	    }
	}
   }
}


char* queuetypetostring(unsigned int queue){
    switch(queue){
	case high:
	     return "High priority queue";
	case normal:
	     return "Normal priority queue";
	case low:
	     return "Low priority queue";
	case initial:
	     return "Initial queue";
	case highblock:
	case semhighblock:
	     return "Blocked queue with high priority";
	case normalblock:
	case semnormalblock:
	     return "Blocked queue with normal priority";
	case lowblock:
	case semlowblock:
	     return "Blocked queue with low priority";
	default:
	     return "Error finding queue type";
    }
}

//This functions prints all processes in all queues
void printmessage(Process* p){
    if(p->semaph != -1){
	printf(" waiting for semaphore: %d to be free\n",p->semaph);
    }
    else if((p->sendmes == NULL)&&(p->recvmes == NULL)){
	printf(" No message\n");
    }
    else if (p->recvmes =="~"){
	printf(" waiting to receive message\n");
    }
    else if(p->recvmes == NULL){
	printf(" sent message: \"%s\" to process: %d, waiting for a reply\n",p->sendmes,p->commsid);
    }
    else{
	printf("Error finding state of process\n");
    }
}

void procinfocommand(char *input){
    int* fill;
    int identity = chartoint(input,fill);
    Process* temp = ListFirst(running);
    if(temp->ID == identity){
	printf("Process: %d is currently running\n",temp->ID);
    }
    if(identity == 0){
	printf("Process: 0 is the init process\n");
	return;
    }
    bool flag = false;

    ListFirst(highreadyq);
    ListFirst(normalreadyq);
    ListFirst(lowreadyq);
    ListFirst(blockq);

    //if process is in high priority queue remove it
    if(ListSearch(highreadyq, idsearchlist, &process[identity]) != NULL){
	flag = true;
    }

    //if process is in normal priority queue remove it
    else if(ListSearch(normalreadyq, idsearchlist, &process[identity]) != NULL){
	flag = true;
    }

    //if process is in low priority queue remove it
    else if(ListSearch(lowreadyq, idsearchlist, &process[identity]) != NULL){
	flag = true;
    }

    //if process is in the block queue remove it
    else if(ListSearch(blockq, idsearchlist, &process[identity]) != NULL){
	flag = true;
    }

    ListFirst(highreadyq);
    ListFirst(normalreadyq);
    ListFirst(lowreadyq);
    ListFirst(blockq);    

    if(flag == true){
	printf("\tPid: %d",identity);
	printf("\tQueue: %s",queuetypetostring(process[identity].queue));
        printmessage(&process[identity]);
	printf("\n");
    }
    else{
	printf("The process: %d does not exist\n",identity);
    }
    return;
}

void totalinfocommand(){
    bool flag;
    int n = 0;
    Process* temp;
    temp = ListFirst(running);
    printf("In running queue:\n");
    while(ListCount(running) > n){
	printf("\tPid: %d",temp->ID);
	printf("\tQueue: %s",queuetypetostring(temp->queue));
        printmessage(temp);
	temp = ListNext(running);
	n++;
    }
    ListFirst(running);
    printf("\n");

    n=0;
    temp = ListFirst(highreadyq);
    printf("In high priority queue:\n");
    while(ListCount(highreadyq)!= n){
	printf("\tPid: %d",temp->ID);
	printf("\tQueue: %s",queuetypetostring(temp->queue));
        printmessage(temp);
	temp = ListNext(highreadyq);
	n++;
	printf("\n");
    }
    ListFirst(highreadyq);

    n=0;
    temp = ListFirst(normalreadyq);
    printf("In normal priority queue:\n");
    while(ListCount(normalreadyq)!= n){
	printf("\tPid: %d",temp->ID);
	printf("\tQueue: %s",queuetypetostring(temp->queue));
        printmessage(temp);
	temp = ListNext(normalreadyq);
	n++;
	printf("\n");
    }
    ListFirst(normalreadyq);

    n=0;
    temp = ListFirst(lowreadyq);
    printf("In low priority queue:\n");
    while(ListCount(lowreadyq)!= n){
	printf("\tPid: %d",temp->ID);
	printf("\tQueue: %s",queuetypetostring(temp->queue));
        printmessage(temp);
	temp = ListNext(lowreadyq);
	n++;
	printf("\n");
    }
    ListFirst(lowreadyq);

    n=0;
    temp = ListFirst(blockq);
    printf("In blocked queue:\n");
    while(ListCount(blockq)!= n){
	printf("\tPid: %d",temp->ID);
	printf("\tQueue: %s",queuetypetostring(temp->queue));
        printmessage(temp);
	temp = ListNext(blockq);
	n++;
	printf("\n");
    }
    ListFirst(blockq);
    printf("\n");
    return;
}

//Switch statement handling the input of the user
void switchstatement(char *input){
    if (strlen(input) != 0){
	switch(input[0]){
	    case 'C':
	    case 'c':
		createcommand(input);
		break;
	    case 'F':
	    case 'f':
		forkcommand();
		break;
	    case 'K':
	    case 'k':
		killcommand(input);
		break;
	    case 'E':
	    case 'e':
		exitcommand();
		break;
	    case 'Q':
	    case 'q':
		quantumcommand();
		break;
	    case 'S':
	    case 's':
		sendcommand(input);
		break;
	    case 'R':
	    case 'r':
		receivecommand();
		break;
	    case 'Y':
	    case 'y':
		replycommand(input);
		break;
	    case 'N':
	    case 'n':
		newsemcommand(input);
		break;
	    case 'P':
	    case 'p':
		sempcommand(input);
		break;
	    case 'V':
	    case 'v':
		semvcommand(input);
		break;
	    case 'I':
	    case 'i':
		procinfocommand(input);
		break;
	    case 'T':
	    case 't':
		totalinfocommand();
		break;
	    case '!':
		printf("! entered\n");
		printf("No more processes. Terminating\n");
		ListRemove(running);
		break;
	    default:
		printf("Invalid character\n");
	}
    }
    return;
}

//Prints instructions of the simulation
void printinstructions(){
    printf("\nc - Create a process and put it on the appropriate ready Q\n");
    printf("f - Copy the currently running process and put it on he ready Q corresponding to the original process' priority\n");
    printf("k - Kill the named process and remove it from the system\n");
    printf("e - Kill the currently running process\n");
    printf("q - Time quantum of runnin process expires\n");
    printf("s - Send a message to another process - block until reply\n");
    printf("r - Receive a message - block until one arrives\n");
    printf("y - Unblocks sender and delivers reply\n");
    printf("n - Initialize the named semaphore with the value given. ID's can take a value from 0 to 4\n");
    printf("p - Execute the semaphore P operation on behalf of the running process\n");
    printf("v - Execute the semaphore V operation on behalf of the running process\n");
    printf("i - Dump complete state information of procss to screen\n");
    printf("t - Display all process queues and their contents\n"); 
    printf("! - To immediately quit the simulation\n\n");
}

int main(){
    //Initialize the queues
    highreadyq = ListCreate();
    normalreadyq = ListCreate();
    lowreadyq = ListCreate();
    blockq = ListCreate();
    running = ListCreate();

    //Create the init process
    process[0].ID = 0;
    process[0].queue = initial;
    process[0].sendmes = NULL;
    process[0].commsid = -1;
    process[0].recvmes = NULL;
    process[0].semaph = -1;
    ListAdd(running, &process[0]);

    for(int i=0;i<5;i++){
	semaphore[i].val = -1;
    }

    printinstructions(); 
    char input[BUFFSIZE];
    //Execution of the program
    while (ListCount(running) > 0){
	memset(input,'\n',strlen(input));	
        printf("Input: ");
        fgets(input, BUFFSIZE, stdin);
	switchstatement(input);	
    }

    return 0;
}
