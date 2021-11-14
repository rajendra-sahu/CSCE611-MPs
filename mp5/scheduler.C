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
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "mem_pool.H"


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
extern EOQTimer *timer;
extern MemPool * MEMORY_POOL;
/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() 
{
  
  /*Initilaize head & tail*/
  head = NULL;
  tail = NULL;
    
  Console::puts("Constructed Base Scheduler.\n");
}

void Scheduler::yield() {
  Console::puts("In base Scheduler's yield();  no actual implementation.\n");
  assert(false);
}

void Scheduler::resume(Thread * _thread) {
  Console::puts("In base Scheduler's resume(); no actual implementation.\n");
  assert(false);
}

void Scheduler::add(Thread * _thread) 
{
  /*Create an empty node*/
  tcb_node* node = (tcb_node*)(MEMORY_POOL->allocate(sizeof(tcb_node)));
  node->thread = _thread;
  node->next = NULL;
  
  /*Keeping enqueuing critical; hence disable & enable interrupts*/
  //Console::puts("Critical Section Adding: Disable & Enable Interrupts\n");
  //Machine::disable_interrupts();
  
  /*Add it appropriately*/
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
  
  //Machine::enable_interrupts();
    
  Console::puts("In non virtual add(); could from add() or resume(); Adding Thread: "); Console::puti(node->thread->ThreadId() + 1); Console::puts("\n");
}

void Scheduler::terminate(Thread * _thread) {
  Console::puts("In base Scheduler's terminate(); no actual implementation.\n");
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
  //Current running thread has been added to the ready queue by the resume() call; now time to pop the head thread in queue & dispatch that node.
  
  /*Keeping cpu yield/dequeueing critical; hence disable & enable interrupts*/
  Console::puts("Critical Section Yielding: Disable & Enable Interrupts\n");
  Machine::disable_interrupts();
  
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
  
  Machine::enable_interrupts();
  
  Console::puts("Dispatching Thread: "); Console::puti(node->thread->ThreadId() + 1); Console::puts("\n");
  Thread::dispatch_to(node->thread);
  MEMORY_POOL->release((unsigned long)node);
  Console::puts("In derived FIFOscheduler  yield()'s actual implementation.\n");
}

void FIFOScheduler::resume(Thread * _thread) 
{
  //Resuming means adding thread to ready queue
  Console::puts("In derived FIFOscheduler resume()'s actual implementation.\n");
  add(_thread); 
}


void FIFOScheduler::terminate(Thread * _thread)
{
       /*2 possibles scenarios 
	1- The current running thread has to be terminated
	2 - A specific thread in the noe has to be terminated*/
	
       Console::puts("In derived FIFOscheduler's terminate(); Terminating Thread: "); Console::puti(_thread->ThreadId() + 1); Console::puts("\n");
       //Machine::disable_interrupts();
	if (Thread::CurrentThread() == _thread)    //Curent running thread
	{
		yield();
	}
	else if(head->thread == _thread)          //Specific thread in the list
	{
		tcb_node* curr = head;
		head = head->next;
		MEMORY_POOL->release((unsigned long)curr);
	}
	else                                     //Specific thread in the list
	{
		tcb_node* prev = head;
		while(prev->next->thread != _thread)
			prev = prev->next;
		tcb_node* curr = prev->next;
		prev ->next = curr->next;
		MEMORY_POOL->release((unsigned long)curr);
	}
	//Machine::enable_interrupts();
}


/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   R R S c h e d u l e r  */
/*--------------------------------------------------------------------------*/
RRScheduler::RRScheduler()
{
  rr_yield_flag = false;
  Console::puts("Scheduler already constructed in Base Class.\n");
}

void RRScheduler::yield() 
{

  /*Try to distinguish between voluntary yield & timer pre-emption yield*/
  if(rr_yield_flag == false)
  {
  	if(timer->get_ticks() == 4)
  	{
  		timer->reset_ticks();
  		Console::puts("Since thread was yielded voluntarily when the quantum was almost over, reseting the timer.\n");
  	}
  }
  else
  {
  	rr_yield_flag == false;
  }
  
  //Current running thread has been added to the ready queue by the resume() call; now time to pop the head thread in queue & dispatch that node.
    
  /*Keeping cpu yield/dequeueing critical; hence disable & enable interrupts*/
  //Console::puts("Critical Section Yielding: Disable & Enable Interrupts\n");
  //Machine::disable_interrupts();
  
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
  
  //Machine::enable_interrupts();
  
  Console::puts("Dispatching Thread: "); Console::puti(node->thread->ThreadId() + 1); Console::puts("\n");
  Thread::dispatch_to(node->thread);
  MEMORY_POOL->release((unsigned long)node);
  Console::puts("In derived RRscheduler  yield()'s actual implementation.\n");
  
}

void RRScheduler::resume(Thread * _thread) 
{
  //Resuming means adding thread to ready queue
  add(_thread);
  Console::puts("In derived RRscheduler resume()'s actual implementation.\n");
}


void RRScheduler::terminate(Thread * _thread)
{
        /*2 possibles scenarios 
	1- The current running thread has to be terminated
	2 - A specific thread in the noe has to be terminated*/
       Console::puts("In derived RRscheduler's terminate(); Terminating Thread: "); Console::puti(_thread->ThreadId() + 1); Console::puts("\n");
	if (Thread::CurrentThread() == _thread)
	{
		yield();
	}
	else if(head->thread == _thread) 
	{
		tcb_node* curr = head;
		head = head->next;
		MEMORY_POOL->release((unsigned long)curr);
	}
	else
	{
		tcb_node* prev = head;
		while(prev->next->thread != _thread)
			prev = prev->next;
		tcb_node* curr = prev->next;
		prev ->next = curr->next;
		MEMORY_POOL->release((unsigned long)curr);
	}
}

void RRScheduler::handle_rr_quantum()
{
	/*Same business of resume & yield but set the flag also*/
	Console::puts("50 mS time quantum has passed; now yielding \n");
	rr_yield_flag = true;
	resume(Thread::CurrentThread());
	yield();
}
