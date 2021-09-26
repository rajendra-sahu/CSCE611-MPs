/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/*  STATE LOGIC
	State consists of only 2 bits. 
	Zeroth bit -> Free or not
	First bit  -> Head of an allocated frame of sequence or not
	Each 8 bit char bitmap stores the state of 4 frames.
	Appropriate calculation is in place to refer the correct char & state index

	Take note both bits can't be one which would indicate head of a free sequence of streams; Doesn't make logical sense
	Hence a head of sequence frame will have it's free bit cleared.
	To initialize set the first frame as head and rest all free
*/
        
#define ALLOCATE_BUT_NOT_HEAD 0x0
#define ALLOCATE_AND_HEAD     0x2
#define FREE                  0x1
// Only these 3 states are valid

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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

ContFramePool* ContFramePool::head;                      

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

/*
	_frame_index_4_multiple -> index to refer the char that contains frame state out of the char array
	_bitmap_index           -> bitmap index to refer the actual state from the 4 states present in 1 char 
	Since the state is 2 bits & char is 8 bits the factor of 4 comes into picture
	
*/

unsigned char ContFramePool::getState(unsigned long _frame_index_4_multiple, unsigned short _bitmap_index)
{
	unsigned char val;
	
	if(_bitmap_index == 0x0)
	{val = bitmap[_frame_index_4_multiple].bmp0;}
	if(_bitmap_index == 0x1)
	{val = bitmap[_frame_index_4_multiple].bmp1;}
	if(_bitmap_index == 0x2)
	{val = bitmap[_frame_index_4_multiple].bmp2;}
	if(_bitmap_index == 0x3)
	{val = bitmap[_frame_index_4_multiple].bmp3;}
	
	return val;
}

void ContFramePool::setState(unsigned long _frame_index_4_multiple, unsigned short _bitmap_index, unsigned char val)
{
	if(_bitmap_index == 0x0)
	{bitmap[_frame_index_4_multiple].bmp0 = val;}
	if(_bitmap_index == 0x1)
	{bitmap[_frame_index_4_multiple].bmp1 = val;}
	if(_bitmap_index == 0x2)
	{bitmap[_frame_index_4_multiple].bmp2 = val;}
	if(_bitmap_index == 0x3)
	{bitmap[_frame_index_4_multiple].bmp3 = val;}
}

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // INITIALISATION OF DATA MEMBERS
    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;
    
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) 
    {
        bitmap = (bitmap_char_s *) (base_frame_no * FRAME_SIZE);
    }
    else 
    {
        bitmap = (bitmap_char_s *) (info_frame_no * FRAME_SIZE);
    }
    
    
    
    //  Everything ok. Proceed to mark all frames free
        
    for(int i=0; i < _n_frames; i++) 
    {                        
        setState(i/4, i%4, FREE);                      //Only setting the zeroth bit i.e. free
        /*Please take care that we don't access the wrong bitmpas in the last _frame_index_4_multiple 
        because they might be set to zero by default which would mean allocated in bitmap sense*/
    }
    
                                                 
    if(info_frame_no == 0)                       // Mark the first frame as being used if it is being used
    {                       
        setState(0x0, 0x0, ALLOCATE_AND_HEAD);   //Clearing the free bit & setting head of sequence bit in the first frame state
        nFreeFrames--;
    }
    //TODO Implement if info_frame_no is non zero
    
    
    /*Lets update the list of frame pools*/
    if(head == nullptr)                          //head is null means the list is empty
    {
    	head = this;
    }
    else                                         //traverse the non empty list to find the tail
    {
    	ContFramePool* temp;
    	temp = head;
    	while(temp->next != nullptr)
    	{
    		temp = temp->next;             
    	}
    	temp->next = this;                       //Append the frame pool at the tail
    }
    next = nullptr;                              //the next ptr of the tail node should point to null 
    
    Console::puts("ContframePool::Frame pool initialized!\n");
}

bool ContFramePool::isFree(unsigned long _frame_index)
{
	if( (getState(_frame_index/4, _frame_index%4) & FREE) == FREE)
	{return true;}
	else
	{return false;}
}

void ContFramePool::allocate(unsigned long _frame_index, bool _head)
{
	assert(isFree(_frame_index) == true);                                   //Check if the frame is actually free or not
	setState(_frame_index/4, _frame_index%4, ALLOCATE_BUT_NOT_HEAD);        //Clearing the free bit
	if(_head == true)  
	setState(_frame_index/4, _frame_index%4, ALLOCATE_AND_HEAD);            //Setting the head of sequence bit
} 

void ContFramePool::release(unsigned long _frame_index)
{
	assert(isFree(_frame_index) == false);                  //Check if the frame is actually allocated or not.
	setState(_frame_index/4, _frame_index%4, FREE);         //Setting the free bit & clearing the head of sequence bit irrespective of its state
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    
    // Any frames left to allocate?
    assert(nFreeFrames > 0);
    
    unsigned int frame_no = base_frame_no;
    
    unsigned int i = 0;
    unsigned int j = 0;
    bool free_frames_flag = true;                             //Intermediate flag that indicates that a free sequence of frames was found 
    bool allocated = false;                                   //Final flag that indicates 
    while(i < n_frames)
    {
    	if(isFree(i) == true)                                 //start off the search with atleast one free frame
    	{
    		for(j = i; j <= i+ _n_frames ; j++)           //traverse for the next _n_frames
    		{
    			if(isFree(j) == false)                //Allocated frame found; allocation of _n_frames not possible in this range
    			{
    				free_frames_flag = false;
    				break;
    			}
    		}
    		if(free_frames_flag == true)                  //_n_frames free frames found
    		{
    			frame_no = frame_no + i;     
    			allocated = true;
    			break;
    		}
    		if(free_frames_flag == 0)                     //traverse the remaining list
    		{
    			free_frames_flag = true;
    			i = j + 1;
    		}
    	}
    	else
    	i++;
    }
    
    if(allocated == true)
    {
    	for(j = i; j <= i+ _n_frames ; j++)                   //Set the appropriate head_of_sequence bit & clear the free bits
    	{
    		if(j == i)
    			allocate(j, true);
    		else
    			allocate(j, false);
    	}
    	nFreeFrames = nFreeFrames - _n_frames;              //Reduce the no of free frames
    	Console::puts("ContframePool::getFrames() Frame sequence allocated!\n");
    	return frame_no;
    }
    else
    {
    	Console::puts("ContframePool::getFrames() Frame sequence could not be allocated!\n");
    	return 0;                                           //Allocation request didn't go through ; return 0
    }

    
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{    
    // Let's first do a range check.
    assert ((_base_frame_no >= base_frame_no) && (_base_frame_no < base_frame_no + n_frames));
    assert ((_base_frame_no + _n_frames >= base_frame_no) && (_base_frame_no + _n_frames < base_frame_no + n_frames));
    
    for(int i = _base_frame_no; i < _base_frame_no + _n_frames; i++)    //Traverse through each frame to allocate
    {
    	if(i == _base_frame_no)
    	{allocate(i - base_frame_no, true);}
    	else                              
    	{allocate(i - base_frame_no, false);}
    	
    	nFreeFrames--;                                                  //Reduce the free frames count
    }
    
    Console::puts("ContFramePool::mark_inaccessible - Memory marked inaccessigble\n");
}

void ContFramePool::release_frames_from_pool(unsigned long _first_frame_no)
{
	unsigned long i = _first_frame_no;
	//Release frame one by one
	while(isFree(i - base_frame_no) == false)
	{
		release(i - base_frame_no);     //release one frame      
		nFreeFrames++;                  //Increase free count
		i++;
	}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{   
    //Find which frame pool the sequence of frames belongs to
    
    ContFramePool* required_frame_pool = ContFramePool::head;
    while(required_frame_pool->next != nullptr)
    {
    	if((_first_frame_no >= required_frame_pool->base_frame_no) && (_first_frame_no < required_frame_pool->base_frame_no + required_frame_pool->n_frames))
    	{
    		//Framepool found
    		break;
    	}
    	required_frame_pool = required_frame_pool->next;
    }
    
    //Release frames via this frame pool specific release function
    required_frame_pool->release_frames_from_pool(_first_frame_no);
    
    Console::puts("ContFramePool::release_frames - Frame sequence released\n");
}



unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    //Its 2 bits per frame implementation now 
    // No of management info frames = _n_frames / 16k + (_n_frames % 16k > 0 ? 1 : 0)
    return (_n_frames/ 16384 + (_n_frames % 16384 > 0 ? 1 : 0)) ;     
}
