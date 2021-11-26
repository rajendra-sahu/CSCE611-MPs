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
  
  
  request =   (rw_request*)(MEMORY_POOL->allocate(sizeof(rw_request)));
  
  Console::puts("Constructed Derived BlockingDisk.\n");
}

/*--------------------------------------------------------------------------*/
/* REQUEST QUEUE FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::push_request(DISK_OPERATION _op, unsigned long _block_no, unsigned char * _buf)
{   
  
  //Saving the io request
  request->op = _op;
  request->rw_block_no = _block_no;
  request->buf = _buf;
  
}



/*--------------------------------------------------------------------------*/
/* BLOCKING_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

  /*
  1. save the request
  2. issue actual command to the device
  3. call the non blocking wait
  */
  
  push_request(DISK_OPERATION::READ, _block_no, _buf);
  issue_operation(DISK_OPERATION::READ, _block_no);
  nonblock_wait_and_process();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

  /*
  1. save the request
  2. issue actual command to the device
  3. call the non blocking wait
  */
  
  push_request(DISK_OPERATION::WRITE, _block_no, _buf);
  issue_operation(DISK_OPERATION::WRITE, _block_no);
  nonblock_wait_and_process();
  
}

void BlockingDisk::nonblock_wait_and_process()
{
  /*This is actually not a blocking wait.
    Thread will yield if device not ready, else the next resume of the thread should have the same consistency*/
  while(!is_ready())                                         
  {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());                  //Add the thread to teh ready queue again
        Console::puts("Device is not ready, voluntarily yielding thread\n ");  
        SYSTEM_SCHEDULER->yield();
  }
  
  Console::puts("Device is ready, performing the actual operation\n "); 
  unsigned long i;
  unsigned short tmpw;
  
  /*Performing the actual data transfer*/
  if(request->op == DISK_OPERATION::READ)
  {
	for (i = 0; i < 256; i++) 
	{
	tmpw = Machine::inportw(0x1F0);
	request->buf[i*2]   = (unsigned char)tmpw;
	request->buf[i*2+1] = (unsigned char)(tmpw >> 8);
	}
  }
  else
  {
  	  for (i = 0; i < 256; i++) 
  	  {
    	  tmpw = request->buf[2*i] | (request->buf[2*i+1] << 8);
          Machine::outportw(0x1F0, tmpw);
  	  }
  }
  
}

