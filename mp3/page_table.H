/*
    File: page_table.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 16/12/07

    Description: Basic Paging.

*/

#ifndef _page_table_H_                   // include file only once
#define _page_table_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define PHYSICAL_ADDRESS_MASK  0xFFFFF000
#define PHYSICAL_ADDRESS_START 12
#define PDE_INDEX_MASK         0xFFC00000
#define PTE_INDEX_MASK         0x003FF000
#define PDE_FIELD_START        22
#define PTE_FIELD_START        12

#define ERR_CODE_MASK          7
#define U_W_P                  7            //User,       Write,    Protection Violation
#define U_W_NP                 6            //User,       Write,    Not present
#define U_R_P                  5            //User,       Read,     Protection Violation
#define U_R_NP                 4            //User,       Read,     Not present
#define S_W_P                  3            //Supervisor, Write,    Protection Violation
#define S_W_NP                 2            //Supervisor, Write,    Not present
#define S_R_P                  1            //Supervisor, Read,     Protection Violation
#define S_R_NP                 0            //Supervisor, Read,     Not Present
//Protection violation means the page is present

#define CR0_PAGING_BIT         0x80000000
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "exceptions.H"
#include "cont_frame_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* P A G E - T A B L E  */
/*--------------------------------------------------------------------------*/

class PageTable {

private:

  /* THESE MEMBERS ARE COMMON TO ENTIRE PAGING SUBSYSTEM */
  static PageTable     * current_page_table; /* pointer to currently loaded page table object */
  static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
  static ContFramePool * kernel_mem_pool;    /* Frame pool for the kernel memory */
  static ContFramePool * process_mem_pool;   /* Frame pool for the process memory */
  static unsigned long   shared_size;        /* size of shared address space */

  /* DATA FOR CURRENT PAGE TABLE */
  unsigned long        * page_directory;     /* where is page directory located? */

public:
  static const unsigned int PAGE_SIZE        = Machine::PAGE_SIZE; 
  /* in bytes */
  static const unsigned int ENTRIES_PER_PAGE = Machine::PT_ENTRIES_PER_PAGE; 
  /* in entries, duh! */

  static void init_paging(ContFramePool * _kernel_mem_pool,
                          ContFramePool * _process_mem_pool,
                          const unsigned long _shared_size);
  /* Set the global parameters for the paging subsystem. */

  PageTable();
  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! 
     Probably it is best to have it on the stack, as there is no 
     memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* 
     paging has been enabled.
     
     We have to make sure the constructor sets up the paging table in the kernel mem_pool
     & follows one-to-one mapping for the first 4 MB of memory.
     Hence the constrcutor follows the following logic
     1. Allocate a kernel pool frame for the page directory.
     2. Allocate a kernel pool frame for a page table which is enough to store mapping of 4MB
     3. We make all of the entries in page table point to actual memory one-to-one in the 4MB boundary
        and also set the atributes in each of the pte as : supervisor level, read/write, present(011 in binary)
     4. We use the zeroth pde to point to the page table created above 
        & attribute set to: supervisor level, read/write, present(011 in binary)
     5. Now we set the attributes of rest 1023 pde as : supervisor level, read/write, not present(010 in binary) 
        since they are not valid entries and will be populated in the future by page fault handling
     
  */
  
  void load();
  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
 
  static void enable_paging();
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. 
     Have added an assert to check if the concerned page table has been loaded or not before enabling paging*/
     

  static void handle_fault(REGS * _r);
  /* 
     The page fault handler. 
     First the console outputs the 3 bit error code
     Then it runs on the following logic
     If it's a NOT PRESENT error go handle the page fault exception
     	1. Read the page directoty from CR3 because you want to start making the translation 
     	2. Allocate a frame from process_mem_pool for the page accessed beyond 4 MP
     	3. Read the logical address for which the page fualt occurred from CR2
     	4. Extract the PDE & PTE indices from yhis logical address
     	5. Check if the pde in the page directory is valid or not.
     		If it's not valid
     		a. we have to create a page table by allocating another frame in kernel mem pool
     		b. Make the pde point to this page table frame (physical address) & set the PRESENT bit in the pde entry
     		c. Init all of the enries in this page table with attribute set to: supervisor level, read/write, not present(010 in binary)
	6. Since we have made the pde valid, now make the pte point to the frame address that we allocated from process_mem_pool & set attribute to: supervisor level, read/write, present
	
      If it's not a NOT PRESENT error (a page protectin violation)
      Console a differernt message and handle the exception appropriately which is not in the scope fo this MP
   */
  

  
};

#endif

