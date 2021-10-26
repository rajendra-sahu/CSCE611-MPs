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
    
    page_table->register_pool(this);                            //registering the new vm pool
    
    allocated_list = (region_node_s *)allocate(4096);      //allocating a full page ; first half for allocated_list
    free_list = (region_node_s *)((void *)(allocated_list) + 2048);  //second half for free_list
    
    allocated_list[0].base_address = base_address;
    allocated_list[0].size = 4096;
    no_of_allocated = 1;
    
    free_list[0].base_address = base_address + 4096;
    free_list[0].size = size - 4096;
    no_of_freed = 1;
    
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    assert(false);
    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) 
{
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

