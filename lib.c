/*
 * Author: Lahiru Samaranayaka
 * E/12/302
 * Operating System
 * LAB 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "threadlib.h"

//#define DEBUG
///* uncomment when you are done! */
//
//#ifdef DEBUG
// #define PRINT   printf
//#else
// #define PRINT(...)
//#endif

/* information about threads
 * To Use Round robin Scheduling we need a proper data structure
 * Which is a Queue I use in this case
 * Because of it is FIFO(First in First Out) one
*/
typedef struct tcb{
  void* sp;
  struct tcb* link;
  /* Address of stack pointer.
	* Keep this as first element would ease switch.S
	* You can do something else as well.
	*/
  /* you will need others stuff */
}tcb_t;

typedef struct queue{
    tcb_t* front;
    tcb_t* rear;
}queue;

typedef struct tcb* TCB;

/**
 * assembly code for switching
 * @sp -- new stack to switch
 * return stack pointer of the old thread
 *
 * Switching
*/
void machine_switch(tcb_t* newthread, tcb_t* oldthread);
void switch_threads(tcb_t* newthread, tcb_t* oldthread);
queue* create_queue(void);
bool isempty(queue* a);
void enqueue(queue* a, tcb_t* temp);
tcb_t* getFront(queue* a);
void dequeue(queue* a);
void * malloc_stack(void);
void schedule(tcb_t* newthread, tcb_t* oldthread);
void addThreadtoQueue(queue* a, long int* p);
int sizeofQueue(queue* a);

/******************************************************************/
/*** Data structures and functions to support thread control box***/
/******************************************************************/
queue* tcb_set;    //Queue to save tcbs
tcb_t* current_thread; // Current running thread

queue* create_queue(void){
    queue* new = (queue*)malloc(sizeof(queue));
    new -> front = NULL;
    new -> rear = NULL;
    return new;
}

bool isempty(queue* a){

    if((a -> front == NULL) && (a -> rear == NULL)){
        return true;
    }
    return false;
}

void enqueue(queue* a, tcb_t* temp){

    if(isempty(a)){
        a -> front = temp;
        a -> rear = temp;
        return;
    }else{
        a -> rear -> link = temp;
        a -> rear = temp;
    }
}

tcb_t* getFront(queue* a){
    if(isempty(a)){
        return NULL;
    }else{
         
        tcb_t* temp = tcb_set -> front;
       
        dequeue(tcb_set);
         
        return temp;
    }
}

void dequeue(queue* a){
    if(isempty(a)){
        return;
    }else{
        if(a -> front == a -> rear){
            a -> front = NULL;
            a -> rear = NULL;
        }else{
            a -> front = a -> front -> link;
        }
    }
}
/******************************************************************/
/********************* end of data structures***********************/
/******************************************************************/


/******************************************************************/
/*********************Thread Library Functions***********************/
/******************************************************************/

void switch_threads(tcb_t* newthread, tcb_t* oldthread) { //old thread is current running thread

  /* This is basically a front end to the low-level assembly code to switch. */
    if(oldthread==NULL){
        //Machenism to steal main threads stack replace it with another
        tcb_t* temp = malloc(sizeof(tcb_t));
        temp -> sp = malloc(sizeof(void *)*1024); // just to copy registers of main thread no need afterwards
        machine_switch(newthread, temp);
    }
    else{
        schedule(newthread,oldthread);
    }
}

void schedule(tcb_t* newthread, tcb_t* oldthread){
    
  if(oldthread == NULL){     //no thread is currently running 
      
        current_thread = newthread;
        machine_switch(newthread, oldthread);
  }else{
        enqueue(tcb_set,oldthread);  // add the past ran thread to run afterward according to round robin
        current_thread = newthread;
        machine_switch(newthread, oldthread);
  }
}
void yield(){
    if(isempty(tcb_set)) {
        printf("This is error can not happen yield will not stop tcb_set is null in yield\n");
        return;
    }
  /* thread wants to give up the CPUjust call the scheduler to pick the next thread. */
    enqueue(tcb_set,current_thread);
    tcb_t* newthread = getFront(tcb_set); // Which Thread should run next and to be run thread from queue
    switch_threads(newthread,current_thread); //current thread is the past ran thread this must be saved
}

void delete_thread(){
  /* When a user-level thread calls this you should not
   * let it run any more but let others run
   * make sure to exit when all user-level threads are dead */
  if(isempty(tcb_set) || tcb_set -> front == NULL) exit(0);
  /*
   * Reclaim all the resources only when the last thread in
   * the application calls delete_thread
   */
  tcb_t* newthread = getFront(tcb_set);  // Which Thread should run next
  tcb_t* oldthread = NULL;
  
  free(current_thread);
  
  switch_threads(newthread, oldthread);
  if(isempty(tcb_set)) exit(0);
  assert(!printf("Implement %s",__func__));

  return;

}

void stop_main(){
  /* Main function was not created by our thread management system.
   * So we have no record of it. So hijack it.
   * Do not put it into our ready queue, switch to something else.*/
  if(tcb_set == NULL || isempty(tcb_set)) {
 
      printf("This is error can not happen tcb_set is null\n");
      return;  //If no threads to run return this
  }
  //tcb_t* newthread = (tcb_t*)getFront(tcb_set);
  tcb_t* newthread = getFront(tcb_set);
  
  current_thread = newthread;
  switch_threads(newthread,NULL);
  assert(!printf("Implement %s",__func__));
}

void addThreadtoQueue(queue* a, long int* p){

    tcb_t* temp = (tcb_t*)malloc(sizeof(tcb_t));
    temp -> sp = p + 1018; //need to save registers
    temp -> link = NULL;
    enqueue(a,temp);
    
}

/******************************************************************/
/*********************End of Thread Library Functions***************/
/******************************************************************/

/*******************************************************************/
/*********************Thread creation etc***************************/
/*******************************************************************/

/* Notes: make sure to have sufficient space for the stack
 * also it needs to be aligned
 */

#define STACK_SIZE (sizeof(void *) * 1024)
#define FRAME_REGS 48 // is this correct for x86_64?
//yes we do not use address of 64 bits because 48 bit can address 256 GB that is more than enough

#include <stdlib.h>
#include <assert.h>

/*
 * allocate some space for thread stack.
 * malloc does not give size aligned memory
 * this is some hack to fix that.
 * You can use the code as is.
 */

void* malloc_stack(void){
  /* allocate something aligned at 16
   */
   void* ptr = malloc(STACK_SIZE + 16);
   if (!ptr) return NULL;
   ptr = (void*)(((long int)ptr & (-1 << 4)) + 0x10);
   return ptr;
}

int create_thread(void (*ip)(void)) {

	long int* stack;
	stack = malloc_stack();
	if(!stack) return -1;
        if(tcb_set == NULL){

            tcb_set = create_queue();
            
        }
        /*
         * no memory? This is very unlikely happens
         * because this is a virtual stack that we are creating
         * which is dynamically allocated at heap
         */
        long int * temp = stack + 1024; // to start from first of the stack cause stack groes downwards 
        
        *temp = (long int) ip;   
        *stack = (long int)ip;          //return address is saved to the last slot of the stack
        addThreadtoQueue(tcb_set,stack); // Save the new thread at thread stack

        /**
   * Stack layout: last slot should contain the return address and I should have some space
   * for callee saved registers. Also, note that stack grows downwards. So need to start from the top.
   * Should be able to use this code without modification Basic idea: C calling convention tells us the top
   * most element in the stack should be return ip. So we create a stack with the address of the function
   * we want to run at this slot.
   */

	return 0;
}


/******************************************************************/
/********************End of Thread creation************************/
/******************************************************************/


/******************************************************************/
/************Debuging functions for data structueres***************/
/******************************************************************/

void printQueue(void){
    tcb_t* temp = tcb_set -> front;
    int i=1;
    if(isempty(tcb_set)) return;
    while(1){
        if(temp == tcb_set -> rear){
            printf("printing thread address of tcb %d : %p\n",i,temp);
            break;
        }
        printf("printing thread address of tcb %d : %p\n",i,temp);
        temp = temp -> link;
        i++;
    }
    return;
}

int sizeofQueue(queue* a){
    int i=1;
    tcb_t* temp = tcb_set -> front;
    if(isempty(tcb_set)) return 0;
    while(1){
        if(temp == tcb_set -> rear){
            printf("The size of Queue is %d\n",i);
            break;
        }
        temp = temp -> link;
        i++;
    }
    return i;
}

/******************************************************************/
/************End*of*Debuging*functions*for*data*structueres********/
/******************************************************************/