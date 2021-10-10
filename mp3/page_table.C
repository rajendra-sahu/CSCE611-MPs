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

//TODO Put meaingful asserts in the functions


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //Inintializing global paramteters for the paging subsystem
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   
   //assert(false);
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

   page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START);            //Need only one frame for the page directory
   
   unsigned long *page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START); //Need to create a page table to hold the 4 MB mapping
   
   unsigned long address=0;                                                                                 // holds the physical address of the frame
   unsigned int i;

   // map the first 4MB of memory
   for(i=0; i<1024; i++)
   {
   	page_table[i] = address | S_W_P;                                                                     // attribute set to: supervisor level, read/write, present(011 in binary)
	address = address + 4096;                                                                            // 4096 = 4kb ; size of a frame 
   }

   // fill the first entry of the page directory
   page_directory[0] = (unsigned long)page_table;                                                      
   page_directory[0] = page_directory[0] | S_W_P;                                                            // attribute set to: supervisor level, read/write, present(011 in binary)
   
   //init the page table entries
   for(i=1; i<1024; i++)
   {
	page_directory[i] = 0 | S_W_NP;                                                                      // attribute set to: supervisor level, read/write, not present(010 in binary)
   }

   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory); // put that page directory address into CR3
   
   //assert(false);
   Console::puts("Loaded page table\n");
}

bool PageTable::check_if_pt_loaded()
{
   if((unsigned long)(current_page_table->page_directory) == read_cr3())
   return true;
   else
   return false;

}

void PageTable::enable_paging()
{
   if(!PageTable::check_if_pt_loaded())
   {
   	Console::puts("Page table not loaded\n");
   	assert(false);
   }
   write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
   Console::puts("Enabled paging\n");
}


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
	  unsigned long *temp_page_directory = (unsigned long *)read_cr3();                                  //read page directory address from CR3 register
	  unsigned long frame_address = (process_mem_pool->get_frames(1)) << PHYSICAL_ADDRESS_START;         //Allocate a frame for the missing page
	  unsigned long fault_address = read_cr2();                                                          // Read the addresss that caused page fault
	  unsigned long pde = (fault_address & PDE_INDEX_MASK) >> PDE_FIELD_START;                           //Extract the PDE index
	  unsigned long pte = (fault_address & PTE_INDEX_MASK) >> PTE_FIELD_START;                           //Extract the PTE index 
	  bool flag = false;                                                                                 // Flag to denote new page table was created

	  if((temp_page_directory[pde] & 0x1) != 0x1)                                                        //PDE is not valid
	  {
		temp_page_directory[pde] = (kernel_mem_pool->get_frames(1) << PHYSICAL_ADDRESS_START);       //Allocating frame for a new page table
		temp_page_directory[pde] = temp_page_directory[pde] | S_W_P;                                     // attribute set to: supervisor level, read/write, present(011 in binary) 
		flag = true;
	  }

	  unsigned long *page_table = (unsigned long *)(temp_page_directory[pde] & PHYSICAL_ADDRESS_MASK);   //Storing it in an intermediate variable
	  
	  if(flag)                                                                                           //If a new page table was created init the entries
	  {
		  for(unsigned int i=0; i<1024; i++)                                                         //Intialize every entry of page table
		  {
			page_table[i] = 0 | S_W_NP;                                                          // attribute set to: supervisor level, read/write, not present(010 in binary)
		  } 
	  }             
	  page_table[pte] = frame_address | S_W_P;                                                           //Set the PTE entry to point to the actual physical frame
	                                                                                                     // attribute set to: supervisor level, read/write, present(011 in binary)
  } 
  else
  {
  	Console::puts("Fault was cause by a page level protection violation, type of access need to be modified\n");
  	assert(false);
  }
                                                             
  Console::puts("handled page fault\n");
}


