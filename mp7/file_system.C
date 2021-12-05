/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
#define INODES_BLOCK_NO 0
#define FREELIST_BLOCK_NO 1
#define DISK_BLOCK_SIZE 512
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor, allocating inodes & freelist block\n");
    
    inodes =  (Inode *)(new unsigned char[DISK_BLOCK_SIZE]);           //Allocating a block for inodes
    free_blocks = new unsigned char[DISK_BLOCK_SIZE];                 //Allocating a block for freelist
    //assert(false);
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    
    /* Make sure that the inode list and the free list are saved i.e. written back to the disk */
    
    disk->write(INODES_BLOCK_NO, (unsigned char *)(inodes));
    disk->write(FREELIST_BLOCK_NO, free_blocks);
    
    delete []inodes;
    delete []free_blocks;
    
    //assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

unsigned long FileSystem::GetFreeBlock()
{
    //bool free_found = false;
    for(unsigned int i = 0; i < free_blocks_count; i++)
    {
    	if(free_blocks[i] == 0)
    	return i;
    }
    
    free_blocks_count++;
    return free_blocks_count-1;
	
}

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    disk->read(INODES_BLOCK_NO, (unsigned char *)(inodes));
    disk->read(FREELIST_BLOCK_NO, free_blocks);
    
    /*Return true only if the first two blocks have been marked occupied*/
    if(free_blocks[0] == 1 && free_blocks[1] == 1)
    return true;
    else
    return false;
    //assert(false);
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    unsigned int i;
    for(i = 0; i < DISK_BLOCK_SIZE; i++)
    {
    	inodes[i] = 0;
    	free_blocks[i] = 0;
    }
    
    free_blocks[0] = 1;
    free_blocks[1] = 1;
    
    disk->write(INODES_BLOCK_NO, (unsigned char *)(inodes));
    disk->write(FREELIST_BLOCK_NO, free_blocks);
    
    //assert(false);
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for(unsigned int i = 0; i < inodes_count; i++)
    {
    	if(inodes[i].id == _file_id)
    	return &inodes[i];
    }
    Console::puts("No such file exist, returning NULL ");
    return NULL;
    //assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
       
       if(LookupFile(_file_id))
       {
       	Console::puts("File already exists");
       	assert(false);
       }
       
       
       

}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
}
