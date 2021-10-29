#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //Inintializing global paramteters for the paging subsystem
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

   page_directory = (unsigned long *)(process_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START);            //Need only one frame for the page directory
   
   unsigned long *page_table = (unsigned long *)(process_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START); //Need to create a page table to hold the 4 MB mapping
   
   unsigned long address=0;                                                                                 // holds the physical address of the frame
   unsigned int i;

   // map the first 4MB of memory
   for(i=0; i<1023; i++)
   {
   	page_table[i] = address | S_W_P;                                                                     // attribute set to: supervisor level, read/write, present(011 in binary)
	address = address + 4096;                                                                            // 4096 = 4kb ; size of a frame 
   }
   
   //Making the last entry point to the PD itself
   page_directory[1023] = (unsigned long)page_directory;                                                    
   page_directory[1023] = page_directory[1023] | S_W_P;                                                      // attribute set to: supervisor level, read/write, present(011 in binary)
   

   // fill the first entry of the page directory
   page_directory[0] = (unsigned long)page_table;                                                      
   page_directory[0] = page_directory[0] | S_W_P;                                                            // attribute set to: supervisor level, read/write, present(011 in binary)
   
   //init the page table entries
   for(i=1; i<1023; i++)
   {
	page_directory[i] = 0 | S_W_NP;                                                                      // attribute set to: supervisor level, read/write, not present(010 in binary)
   }
   
   vmpool_list = NULL;
   vmpool_list_count = 0;

   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory); // put that page directory address into CR3
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   if((unsigned long)(current_page_table->page_directory) != read_cr3())                                   //Check if page table loaded before enabling paging
   {
   	Console::puts("Page table not loaded\n");
   	assert(false);
   }
   write_cr0(read_cr0() | CR0_PAGING_BIT);                                                                 // set the paging bit in CR0 to 1
   Console::puts("Enabled paging\n");
}

/*Recursive lookup computation functions*/

unsigned long * PageTable::PDE_address(unsigned long addr)
{
	unsigned long pde_index = (addr & PDE_INDEX_MASK) >> PDE_FIELD_START;
	return (unsigned long *)(PD_LOOKUP + (pde_index << 2));                                           // Last shift of 2 bits is to make the address  pde multiple
	
}

unsigned long * PageTable::PTE_address(unsigned long addr)
{
	unsigned long pde_index = (addr & PDE_INDEX_MASK) >> PDE_FIELD_START;
	unsigned long pte_index = (addr & PTE_INDEX_MASK) >> PTE_FIELD_START;
	return (unsigned long *)(PT_LOOKUP | (pde_index << PTE_FIELD_START) | (pte_index << 2));         //Last shift of 2 bits is to make the address pte multiple
	
}

/*Recursive lookup computation functions*/



void PageTable::handle_fault(REGS * _r)
{
   switch(_r->err_code & ERR_CODE_MASK)
   {
   	case U_W_P:
   	Console::puts("Error Code : User,       Write,    Protection Violation\n");
   	break;
   	case U_W_NP:
   	Console::puts("Error Code : User,       Write,    Not Present\n");
   	break;
	case U_R_P:
   	Console::puts("Error Code : User,       Read,    Protection Violation\n");
   	break;
   	case U_R_NP:
   	Console::puts("Error Code : User,       Read,    Not present\n");
   	break;
   	case S_W_P:
   	Console::puts("Error Code : Supervisor, Write,    Protection Violation\n");
   	break;
   	case S_W_NP:
   	Console::puts("Error Code : Supervisor, Write,    Not present\n");
   	break;
   	case S_R_P:
   	Console::puts("Error Code : Supervisor, Read,    Protection Violation\n");
   	break;
   	case S_R_NP:
   	Console::puts("Error Code : Supervisor, Read,    Not present\n");
   	break;
   }
  
  if((_r->err_code & 0x1) == 0x0)  //Non-present page error code only 
  {
	  //unsigned long is of 8 bytes in GCC 9.3.0
	  
	  unsigned long fault_address = read_cr2();                                                       // Read the addresss that caused page fault
	  
	  
	  /*Checking the legitimacy of address by calling is_legitimate() on every registered pool*/
	  bool legitimacy_flag = false;  
	  
	  for(unsigned long i = 0; i < PageTable::current_page_table->vmpool_list_count; i++)
	  {
	  	legitimacy_flag = PageTable::current_page_table->vmpool_list[i]->is_legitimate(fault_address);       //find out legitimacy on each registered pool
	  	if(legitimacy_flag == true)
	  	{
	  		break;
	  	}
	  }
	  if(!legitimacy_flag)
	  {
  	  	Console::puts("Address not legitimate\n");                            //Abort the handler
  		assert(false);
	  }
	  
	  /*Proceeding with the exception handler*/
	  
	  unsigned long frame_address = (process_mem_pool->get_frames(1)) << PHYSICAL_ADDRESS_START;             //Allocate a frame for the missing page
                                                       
	  unsigned long pde_index = (fault_address & PDE_INDEX_MASK) >> PDE_FIELD_START;                               //Extract the PDE index
	  unsigned long pte_index = (fault_address & PTE_INDEX_MASK) >> PTE_FIELD_START;                               //Extract the PTE index 
	  bool flag = false;                                                                                     // Flag to denote new page table was create
	  unsigned long *pde = PageTable::current_page_table->PDE_address(fault_address);                //Computing the logical pde                                          

	  if((*pde & 0x1) != 0x1)                                                                        //PDE is not valid
	  {
		*pde = (process_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START);                      //Allocating frame for a new page table
		*pde = *pde | S_W_P;                                                                     // attribute set to: supervisor level, read/write, present(011 in binary) 
		flag = true;
	  }
	  
	  unsigned long * page_table = (unsigned long *)(PT_LOOKUP | (pde_index << PTE_FIELD_START));            //computing the logical address of page table; it is pte address multiple
	  
	  if(flag)                                                                                               //If a new page table was created init the entries
	  {
		  for(unsigned int i=0; i<1024; i++)                                                             //Intialize every entry of page table
		  {
			*(page_table + i) = 0 | S_W_NP;                                                          // attribute set to: supervisor level, read/write, not present(010 in binary)
		  } 
	  }             
	  *(page_table + pte_index) = frame_address | S_W_P;                                                     //Set the PTE entry to point to the actual physical frame
	                                                                                                         // attribute set to: supervisor level, read/write, present(011 in binary)
  } 
  else
  {
  	Console::puts("Fault was cause by a page level protection violation, type of access need to be modified\n");
  	assert(false);
  }
                                                             
  Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    
    if((vmpool_list[0] == NULL) && (vmpool_list_count !=0))
    {
  	Console::puts("VM pool list invalid\n");
  	assert(false);
    }
    
    vmpool_list[vmpool_list_count] = _vm_pool;                    //Add an entry
    vmpool_list_count++;
    
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) 
{
    //The parameter _page_no is the logical address itself; use the value directly
    unsigned long *pt_entry = PTE_address(_page_no);
    
    if(*pt_entry & 0x1)                                                         //Check if the page is valid
    {
    	ContFramePool::release_frames(*pt_entry >> PHYSICAL_ADDRESS_START);     //Releasing the frame by passing the frame number
    	*pt_entry = *pt_entry & 0xFFFFFFFE;                                     //Marking the page invaid or non-present
    	write_cr3((unsigned long)page_directory);                               //Flush TLB by reloading cr3 with the current value
    }
    Console::puts("freed page\n");
}
