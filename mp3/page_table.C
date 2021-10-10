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
//TODO Create a function to convert frame no to physical address.


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

   page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) << 12);            //Need only one frame for the page directory
   
   unsigned long *page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) << 12); //Since only first entry of PDE is valid.
   
   unsigned long address=0; // holds the physical address of where a page is
   unsigned int i;

   // map the first 4MB of memory
   for(i=0; i<1024; i++)
   {
   	page_table[i] = address | 3;       // attribute set to: supervisor level, read/write, present(011 in binary)
	address = address + 4096;          // 4096 = 4kb
   }

   // fill the first entry of the page directory
   page_directory[0] = (unsigned long)page_table;        // attribute set to: supervisor level, read/write, present(011 in binary)
   page_directory[0] = page_directory[0] | 3;
   
   for(i=1; i<1024; i++)
   {
	page_directory[i] = 0 | 2;        // attribute set to: supervisor level, read/write, not present(010 in binary)
   }


   //assert(false);
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory); // put that page directory address into CR3
   
   //assert(false);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
   
   //assert(false);
   Console::puts("Enabled paging\n");
}


void PageTable::handle_fault(REGS * _r)
{
  //assert(false);
  unsigned long *temp_page_directory = (unsigned long *)read_cr3();
  unsigned long frame_address = (process_mem_pool->get_frames(1)) << 12;  //Allocate a frame for the missing page
  unsigned long fault_address = read_cr2();
  unsigned long pde = (fault_address & 0xFFC00000) >> 22;                                //Extract the PDE index

  if((temp_page_directory[pde] & 0x1) != 0x1)                                //PDE is not valid
  {
	temp_page_directory[pde] = (kernel_mem_pool->get_frames(1) << 12);     //Allocating frame for a new page table
	
  }

  unsigned long *page_table = (unsigned long *)(temp_page_directory[pde]);   //Storing it in an intermediate variable
  
  if((temp_page_directory[pde] & 0x1) != 0x1)
  {
	  for(unsigned int i=0; i<1024; i++)
	  {
		page_table[i] = 0 | 2;                                           // attribute set to: supervisor level, read/write, not present(010 in binary)
	  }
	  temp_page_directory[pde] = temp_page_directory[pde] | 3;               // attribute set to: supervisor level, read/write, present(011 in binary) 
  }   


  
  unsigned long pte = (fault_address & 0x3FF000) >> 12;                          //Extract the PTE index                 
  page_table[pte] = frame_address | 3;
  Console::puts("handled page fault\n");
}


