/*
 File: cont_frame_pool.H
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/02/04 
 
 Description: Management of the CONTIGUOUS Free-Frame Pool.
 
 As opposed to a non-contiguous free-frame pool, here we can allocate
 a sequence of CONTIGUOUS frames.
 
 */

#ifndef _CONT_FRAME_POOL_H_                   // include file only once
#define _CONT_FRAME_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

typedef struct bitmap_char_
{
	unsigned char bmp0 : 2;
	unsigned char bmp1 : 2;
	unsigned char bmp2 : 2;
	unsigned char bmp3 : 2;
} bitmap_char_s;

/*--------------------------------------------------------------------------*/
/* C o n t F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

class ContFramePool {
    
private:
    /* -- DEFINE YOUR CONT FRAME POOL DATA STRUCTURE(s) HERE. */
    bitmap_char_s* bitmap;
    unsigned long base_frame_no;
    unsigned long n_frames;
    unsigned long nFreeFrames;
    unsigned long info_frame_no;
    unsigned long n_info_frames;
    
    //helper functions
    
    /*
	Only specific to getter setter pair :
	_frame_index_4_multiple -> index to refer the char that contains frame state out of the char array
	_bitmap_index           -> bitmap index to refer the actual state from the 4 states present in 1 char 
	Since the state is 2 bits & char is 8 bits the factor of 4 comes into picture
	
    */
    unsigned char getState(unsigned long _frame_index_4_multiple, unsigned short _bitmap_index );
    void setState(unsigned long _frame_index_4_multiple, unsigned short _bitmap_index, unsigned char val);
    
    
    bool isFree(unsigned long _frame_index);                  //To check if the frame is free or not.
    void allocate(unsigned long _frame_index, bool _head);    //To allocate a single frame
    void release(unsigned long _frame_index);                 //To release a single frame
    
    /*The frame pool list has been implemneted through 2 ContFramePool pointers.
      next - private non-static next pointer to point to the next frame pool
      head - public static pointer to the head of the tail that has the same value for every node.
      The list will be traversed with the help of the next pointer*/
      
    ContFramePool* next;
                                          
public: 
    
    static ContFramePool* head;                                          
        
    // The frame size is the same as the page size, duh...    
    static const unsigned int FRAME_SIZE = Machine::PAGE_SIZE; 

    ContFramePool(unsigned long _base_frame_no,
                  unsigned long _n_frames,
                  unsigned long _info_frame_no,
                  unsigned long _n_info_frames);
    /*
     Initializes the data structures needed for the management of this
     frame pool.
     _base_frame_no: Number of first frame managed by this frame pool.
     _n_frames: Size, in frames, of this frame pool.
     EXAMPLE: If _base_frame_no is 16 and _n_frames is 4, this frame pool manages
     physical frames numbered 16, 17, 18 and 19.
     _info_frame_no: Number of the first frame that should be used to store the
     management information for the frame pool.
     NOTE: If _info_frame_no is 0, the frame pool is free to
     choose any frames from the pool to store management information.
     _n_info_frames: If _info_frame_no is NOT 0, this argument specifies the
     number of consecutive frames needed to store the management information
     for the frame pool.
     EXAMPLE: If _info_frame_no is 699 and _n_info_frames is 3,
     then Frames 699, 700, and 701 are used to store the management information
     for the frame pool.
     NOTE: This function must be called before the paging system
     is initialized.
     */
    

    
    unsigned long get_frames(unsigned int _n_frames);
    /*
     Allocates a number of contiguous frames from the frame pool.
     _n_frames: Size of contiguous physical memory to allocate,
     in number of frames.
     If successful, returns the frame number of the first frame.
     If fails, returns 0.
     */
    
    void mark_inaccessible(unsigned long _base_frame_no,
                           unsigned long _n_frames);
    /*
     Marks a contiguous area of physical memory, i.e., a contiguous
     sequence of frames, as inaccessible.
     _base_frame_no: Number of first frame to mark as inaccessible.
     _n_frames: Number of contiguous frames to mark as inaccessible.
     */
    
    void release_frames_from_pool(unsigned long _first_frame_no);
    /*the actual function which releases frames from a specific frame pool*/
    
    static void release_frames(unsigned long _first_frame_no);
    /*
     Releases a previously allocated contiguous sequence of frames
     back to its frame pool.
     The frame sequence is identified by the number of the first frame.
     NOTE: This function is static because there may be more than one frame pool
     defined in the system, and it is unclear which one this frame belongs to.
     This function must first identify the correct frame pool and then call the frame
     pool's release_frame function.
     */
     
    
    static unsigned long needed_info_frames(unsigned long _n_frames);
    /*
     Returns the number of frames needed to manage a frame pool of size _n_frames.
     The number returned here depends on the implementation of the frame pool and 
     on the frame size.
     EXAMPLE: For FRAME_SIZE = 4096 and a bitmap with a single bit per frame 
     (not appropriate for contiguous allocation) one would need one frame to manage a 
     frame pool with up to 8 * 4096 = 32k frames = 128MB of memory!
     This function would therefore return the following value:
       _n_frames / 32k + (_n_frames % 32k > 0 ? 1 : 0) (always round up!)
     Other implementations need a different number of info frames.
     The exact number is computed in this function..
     */
};
#endif
