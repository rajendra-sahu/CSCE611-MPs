/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) 
{
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    allocated_list_first_entry_set = false;
    
    if(size < 4096)
    {
    	Console::puts("VM pool size less than a page ; can't store the lists.\n");
    	assert(false);
    }
    
    page_table->register_pool(this);                                 //registering the new vm pool
    
    /*Avoiding the allocate() call since it's a bad practise, alhtough it could have been handled by lists_initialized flag*/           
    allocated_list = (region_node_s *)(base_address);                //Explicitly making it to point to the base_address of the vm pool since allocate() is not ready yet
    free_list = (region_node_s *)(allocated_list + 256);             // Max 256 entries in each list ; 4096 / 8 / 2                  
    
    
    /*The following memory references will trigger page faults since they haven't been allocated actual frames.
      It is handled by returning true setting a flag true and chekcing it in the legitimacy fn and the reseting the flag immediately once the lists have been initialised*/
    allocated_list_first_entry_set = true;                                             //Setting the flag true ; to be used by is_legitimate() for just the first entry                   
    allocated_list[0].base_address = base_address;                                     //Set the first allocated region entry
    allocated_list[0].size = 4096;                                                     //allocating a full page ; first half for allocated_list; second half for free_list
    
    free_list[0].base_address = base_address + 4096;                                   //Set the first free region entry
    free_list[0].size = size - 4096;                                                   // we keep spliting the free regions to allocate region
    
    allocated_list_first_entry_set = false;
    no_of_allocated++;
    no_of_freed++;
    
    /*Since the previous memeory reference will trigger the frame allcoation and create a page table mapping; it wont page fault in the following memeory reference */
    for(int i = 1; i < 256; i++)                                                       //Set the other entries as invalid i.e. address & size = 0
    {
    	allocated_list[i].base_address = 0;
    	allocated_list[i].size = 0;
    	free_list[i].base_address = 0;
    	free_list[i].size = 0;
    }
    
   
    
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) 
{
    unsigned long free_index;
    bool allocation_flag = false;
    for(unsigned long i = 0; i< no_of_freed; i++)                   //Traverse through the free list if _size of memeory can be allocated 
    {
    	if(_size <= free_list[i].size)
    	{
    		free_index = i;                                      //Found a region where the specified memeory can be allocated
    		allocation_flag = true;                                
    		break;
    	}
    }
    
    if(allocation_flag)                                              // Moving an entry from free list to allocated list
    {
	    //Creating new allocated list entry
	    allocated_list[no_of_allocated].base_address = free_list[free_index].base_address;
	    allocated_list[no_of_allocated].size = _size;
	    no_of_allocated++;
	    
	    //Reducing the free size available
	    free_list[free_index].base_address =  free_list[free_index].base_address + _size;
	    free_list[free_index].size = free_list[free_index].size - _size;
	   
	    Console::puts("Allocated region of memory.\n");
	    return allocated_list[no_of_allocated - 1].base_address;
    }
    else
    {
    	    Console::puts("Not enough memory for requested bytes.\n");
    	    return 0;
    }
}

void VMPool::release(unsigned long _start_address) 
{
    unsigned long allocated_index;
    bool region_found = false;
    for(unsigned long i = 0; i<no_of_allocated; i++)                    //Traverse through the allocated list to find out if such a memory region exists or not.
    {
    	if(allocated_list[i].base_address == _start_address)
    	{	
    		allocated_index = i;                                    //Found the memory region
    		region_found = true;
    		break;
    	}
    }
    
    if(region_found == false )                                      //Didn't find the region
    {
    	Console::puts("No such allocated region.\n");
    	assert(false);
    }
    
    if(allocated_list[allocated_index].size == 0)                        //The free region size is zero
    {
    	Console::puts("Allocated memory region not valid.\n");
    	assert(false);
    }
    
    //Adding a new entry to the freed list
    free_list[no_of_freed].base_address =  allocated_list[allocated_index].base_address;
    free_list[no_of_freed].size         =  allocated_list[allocated_index].size;
    no_of_freed++;
    
    //Invalidating that specific allocated list entry by setting the size 0
    allocated_list[allocated_index].base_address = 0;
    allocated_list[allocated_index].size = 0;
    
    //Freeing the memory
    page_table->free_page(_start_address);

    
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address)
 {
    Console::puts("Checking whether address is part of an allocated region.\n");
    
    if((allocated_list_first_entry_set == true) && (no_of_allocated == 0))                       //Only passing true legitimacy check if it's the first entry memory access
    {
    	return true;
    }
    
    for(unsigned long i = 0; i<no_of_allocated; i++)                                            //Check all the allocated memory regions
    {
    	if((allocated_list[i].base_address <= _address) && (_address < (allocated_list[i].base_address + allocated_list[i].size)))
    	{
    		return true;
    	}
    }
    return false;
    
}

