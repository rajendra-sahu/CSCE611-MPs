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
    
    page_table->register_pool(this);                                 //registering the new vm pool
    
    allocated_list = (region_node_s *)allocate(4096);                //allocating a full page ; first half for allocated_list
    free_list = allocated_list + 256;                                //second half for free_list
    
    allocated_list[0].base_address = base_address;
    allocated_list[0].size = 4096;
    no_of_allocated = 1;
    
    free_list[0].base_address = base_address + 4096;
    free_list[0].size = size - 4096;
    no_of_freed = 1;
    
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) 
{
    unsigned long free_index;
    bool allocation_flag = false;
    for(unsigned long i = 0; i< no_of_freed; i++)
    {
    	if(_size <= free_list[i].size)
    	{
    		free_index = i;
    		allocation_flag = true;
    		break;
    	}
    }
    
    if(allocation_flag)
    {
	    //Creating new allocated list entry
	    allocated_list[no_of_allocated].base_address = free_list[free_index].base_address;
	    allocated_list[no_of_allocated].size = _size;
	    no_of_allocated++;
	    
	    //Reducing the free size available
	    free_list[free_index].base_address =  free_list[free_index].base_address + _size;
	    free_list[free_index].size = free_list[free_index].size - _size;
	   
	    Console::puts("Allocated region of memory.\n");
	    return allocated_list[no_of_allocated].base_address;
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
    for(unsigned long i = 0; i<no_of_allocated; i++)
    {
    	if(allocated_list[i].base_address == _start_address)
    	{	
    		allocated_index = i;
    		break;
    	}
    }
    
    if(allocated_list[allocated_index].size == 0)
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
    
    for(unsigned long i = 0; i<no_of_allocated; i++)
    {
    	if((allocated_list[i].base_address <= _address) && (_address < (allocated_list[i].base_address + allocated_list[i].size)))
    	{
    		return true;
    	}
    }

    return false;
    
}

