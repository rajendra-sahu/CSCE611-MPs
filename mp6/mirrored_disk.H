/*
     File        : mirrored_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _MIRRORED_DISK_H_
#define _MIRRORED_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
/*Structure to save all in the info relevant to the io request*/
struct mirr_request_s
{
	DISK_OPERATION op;
	unsigned long  rw_block_no;
	unsigned char* buf;
};

typedef struct mirr_request_s mirr_request;
/*--------------------------------------------------------------------------*/
/* M i r r o r e d D i s k  */
/*--------------------------------------------------------------------------*/

class MirroredDisk : public SimpleDisk {

//master & dependent drives of the same controller
SimpleDisk* master;
SimpleDisk* dependent;

//placeholder to save the request
mirr_request* request;
  
public:
   MirroredDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a MirroredDisk device with the given size connected to the 
    */
      

   void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
   
   void wait_and_process();
   /*waits non-blockingly (yields if the device is not ready) and then processes the io request when the device is ready*/
   
   void issue_mirrored_operation(DISK_ID _disk_id, DISK_OPERATION _op, unsigned long _block_no);
   /*Issues 2 simulatneous commands to both of the drives*/

};
#endif
