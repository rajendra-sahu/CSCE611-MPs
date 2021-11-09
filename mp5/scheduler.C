/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() 
{
  //assert(false);
  
  head = NULL;
  tail = NULL;
    
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  //assert(false);
  Console::puts("In base Scheduler's yield();  no actual implementation.\n");
}

void Scheduler::resume(Thread * _thread) {
  //assert(false);
  Console::puts("In base Scheduler's resume(); no actual implementation.\n");
}

void Scheduler::add(Thread * _thread) 
{
  tcb_node* node = (tcb_node*) new char[sizeof(tcb_node)];
  node->thread = _thread;
  node->next = NULL;
  if(head == NULL && tail == NULL)
  {
  	head = node;
  	tail = node;
  }
  else
  {
  	tail->next = node;
  	tail = node;
  }
  
  Console::puts("In non virtual add().\n");
  Console::puts("Adding Thread: "); Console::puti(node->thread->ThreadId()); Console::puts("\n");
  Console::puts("Added thread to the ready queue.\n");
}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   F I F O S c h e d u l e r  */
/*--------------------------------------------------------------------------*/
FIFOScheduler::FIFOScheduler() 
{
  //assert(false);
  
  Console::puts("Scheduler already constructed in Base Class.\n");
}

void FIFOScheduler::yield() {
  //assert(false);
  //Current running thread has been added to the ready queue by the resume() call; now time to pop the head thread in queue & dispatch that node.
  tcb_node* node = head;
  if(head == NULL)
  {
  	Console::puts("No ready thread to yield\n");
  	assert(false);
  }
  if(head->next == NULL)
  {
  	Console::puts("Ready queue is empty now; Threading must terminate after this last thread\n");
  }
  head = head->next;
  Console::puts("Dispatching Thread: "); Console::puti(node->thread->ThreadId()); Console::puts("\n");
  Thread::dispatch_to(node->thread);
  delete []node;
  Console::puts("In derived FIFOscheduler  yield()'s actual implementation.\n");
}

void FIFOScheduler::resume(Thread * _thread) 
{
  //Resuming means adding thread to ready queue
  add(_thread);
  Console::puts("In derived FIFOscheduler resume()'s actual implementation.\n");
}

/*void Scheduler::add(Thread * _thread) 
{
  //assert(false);
  tcb_node* node;
  node->thread = _thread;
  node->next = NULL;
  if(head == NULL && tail == NULL)
  {
  	head = node;
  	tail = node;
  }
  else
  {
  	tail->next = node;
  	tail = node;
  }
  
  Console::puts("Added thread to the ready queue.\n");
}*/

void FIFOScheduler::terminate(Thread * _thread) {
  assert(false);
}
