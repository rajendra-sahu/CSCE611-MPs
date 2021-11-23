/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "mem_pool.H"
#include "thread.H" 

extern MemPool * MEMORY_POOL;
extern Scheduler * SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
  
  /*Initilaize head & tail*/
  head = NULL;
  tail = NULL;
  
  Console::puts("Constructed Derived BlockingDisk.\n");
}

/*--------------------------------------------------------------------------*/
/* REQUEST QUEUE FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::push_request(DISK_OPERATION _op, unsigned long _block_no, unsigned char * _buf)
{
  /*Create an empty node*/
  rw_request* node = (rw_request*)(MEMORY_POOL->allocate(sizeof(rw_request)));
  node->op = _op;
  node->rw_block_no = _block_no;
  node->buf = _buf;
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
    
  Console::puts("Added request to the queue\n "); 
}


void BlockingDisk::pop_request()
{
  if(head)
  {
  	rw_request* node = head;
  	head - head->next;
  	SimpleDisk::issue_operation(node->op, node->rw_block_no);
  	MEMORY_POOL->release((unsigned long)node);
  }
  else
  {
  	Console::puts("No more requests in the queue, wait for requests to be enqueued\n "); 
  }
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  push_request(DISK_OPERATION::READ, _block_no, _buf);
  issue_operation(DISK_OPERATION::READ, _block_no);
  //SYSTEM_SCHEDULER->yield();
  //SimpleDisk::read(_block_no, _buf);
  
  nonblock_wait_and_process();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  push_request(DISK_OPERATION::WRITE, _block_no, _buf);
  issue_operation(DISK_OPERATION::WRITE, _block_no);
  //SYSTEM_SCHEDULER->yield();
  //SimpleDisk::write(_block_no, _buf);
  nonblock_wait_and_process();
}

void BlockingDisk::nonblock_wait_and_process()
{
  while(!is_ready())
  {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread()); 
        SYSTEM_SCHEDULER->yield();
  }
  
  Console::puts("Device is ready, performing the actual operation\n "); 
  int i;
  unsigned short tmpw;
  
  if(head->op == DISK_OPERATION::READ)
  {
	for (i = 0; i < 256; i++) 
	{
	tmpw = Machine::inportw(0x1F0);
	head->buf[i*2]   = (unsigned char)tmpw;
	head->buf[i*2+1] = (unsigned char)(tmpw >> 8);
	}
  }
  else
  {
  	  for (i = 0; i < 256; i++) 
  	  {
    	  tmpw = head->buf[2*i] | (head->buf[2*i+1] << 8);
          Machine::outportw(0x1F0, tmpw);
  	  }
  }

  
}
