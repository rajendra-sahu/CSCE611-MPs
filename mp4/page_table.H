/*
 File: page_table.H
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/12/07
 Update: 21/10/13
 
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


#define PD_LOOKUP              0xFFFFF000
#define PT_LOOKUP              0xFFC00000

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "exceptions.H"
#include "cont_frame_pool.H"
#include "vm_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class VMPool */
/* We need this to break a circular include sequence. */
class VMPool;
bool is_legitimate(unsigned long _address);
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

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
    /* in entries */
    
    VMPool**               vmpool_list;           //List of virtual memory pools
    unsigned long          vmpool_list_count;     //The count of registered pools
    
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
     */
    
    void load();
    /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
    
    static void enable_paging();
    /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */
    
    unsigned long* PDE_address(unsigned long addr);
    /*Compute the logical address of the page directory from an entire logical address*/
    
    unsigned long* PTE_address(unsigned long addr);
    /*Compute the logical address of the page table from an entire logical address*/
    
    static void handle_fault(REGS * _r);
    /* The page fault handler. */
    
    // -- NEW IN MP4
    
    void register_pool(VMPool * _vm_pool);
    /* Register a virtual memory pool with the page table. */
    
    void free_page(unsigned long _page_no);
    /* If page is valid, release frame and mark page invalid. */
    
};

#endif

