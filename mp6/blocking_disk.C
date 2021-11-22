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
#include "mem_pool.H"

extern MemPool * MEMORY_POOL;
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

void BlockingDisk::push_request(unsigned long _block_no, unsigned char * _buf)
{
  /*Create an empty node*/
  rw_request* node = (rw_request*)(MEMORY_POOL->allocate(sizeof(rw_request)));
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


void BlockingDisk::pop_request(unsigned long _block_no, unsigned char * _buf)
{
	
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::write(_block_no, _buf);
}
