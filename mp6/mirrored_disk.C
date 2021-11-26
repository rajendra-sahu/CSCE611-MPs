/*
     File        : mirrored_disk.c

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
#include "mirrored_disk.H"
#include "scheduler.H"
#include "mem_pool.H"
#include "thread.H" 

extern MemPool * MEMORY_POOL;
extern Scheduler * SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* MIRRORED DISK */
/*--------------------------------------------------------------------------*/

MirroredDisk::MirroredDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
  
  //Creastes 2 instances of the simple_disk
  master = new SimpleDisk(DISK_ID::MASTER, _size);
  dependent = new SimpleDisk(DISK_ID::DEPENDENT, _size);
  
  //allocates memeory of the io request
  request =   (mirr_request*)(MEMORY_POOL->allocate(sizeof(mirr_request))); 
  Console::puts("Constructed Derived MirroredDisk.\n");
}



/*--------------------------------------------------------------------------*/
/* DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
void MirroredDisk::issue_mirrored_operation(DISK_ID _disk_id, DISK_OPERATION _op, unsigned long _block_no)
{
  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
                         /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
                         /* send next 8 bits of block number */
  unsigned int disk_no = _disk_id == DISK_ID::MASTER ? 0 : 1;
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_no << 4));
                         /* send drive indicator, some bits, 
                            highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);
}


void MirroredDisk::read(unsigned long _block_no, unsigned char * _buf) {
  
  /*Saving the io request*/
  request->op = DISK_OPERATION::READ;
  request->rw_block_no = _block_no;
  request->buf = _buf;
  
  Console::puts("Issuing mirrored operation\n "); 
  
  issue_mirrored_operation(DISK_ID::MASTER, DISK_OPERATION::READ, _block_no);      // IO command to the master
  issue_mirrored_operation(DISK_ID::DEPENDENT,DISK_OPERATION::READ, _block_no);    // IO command to the dependent

  
  //non block wait
  wait_and_process();

}


void MirroredDisk::write(unsigned long _block_no, unsigned char * _buf) {
  
  /*Saving the io request*/
  request->op = DISK_OPERATION::WRITE;
  request->rw_block_no = _block_no;
  request->buf = _buf;
  
  Console::puts("Issuing mirrored operation\n "); 
  issue_mirrored_operation(DISK_ID::MASTER, DISK_OPERATION::WRITE, _block_no);      // IO command to the master
  issue_mirrored_operation(DISK_ID::DEPENDENT,DISK_OPERATION::WRITE, _block_no);    // IO command to the dependent
  
  //non block wait
  wait_and_process();
  
}

void MirroredDisk::wait_and_process()
{
  while(!master->is_ready() || !dependent->is_ready())                  //Waits for both of the devices to be ready since it's a mirrored drive
  {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());              //Add the thread to teh ready queue again
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
