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

void Inode::inodes_to_and_from_disk(DISK_OPERATION _op)
{
	Console::puts("FileSystem DIsk Address "); Console::puti((int)(&(fs->disk))); Console::puts("\n");
	
	
	if(_op == DISK_OPERATION::READ)
	fs->disk->read(INODES_BLOCK_NO, (unsigned char *)(fs->inodes));
	else
	fs->disk->write(INODES_BLOCK_NO, (unsigned char *)(fs->inodes));
	
}

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor, allocating inodes & freelist block\n");
    
    inodes = (Inode *)new unsigned char[DISK_BLOCK_SIZE];             //Allocating a block for inodes
    free_blocks = new unsigned char[DISK_BLOCK_SIZE];                 //Allocating a block for freelist
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    
    /* Make sure that the inode list and the free list are saved i.e. written back to the disk */
    
    DiskOperation(DISK_OPERATION::WRITE, INODES_BLOCK_NO, (unsigned char *)(inodes));
    DiskOperation(DISK_OPERATION::WRITE, FREELIST_BLOCK_NO, free_blocks);
    
    delete []inodes;
    delete []free_blocks;
    
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

unsigned long FileSystem::GetFreeBlock()
{
    for(unsigned int i = 0; i < DISK_BLOCK_SIZE; i++)
    {
    	if(free_blocks[i] == 0)
    	{ return i; }                    //return block number viz i
    	
    }
    
    return 0xFFFFFFFF;                   //Indicator of no free blocks
    
	
}

unsigned long FileSystem::GetFreeInode()
{
	for(unsigned int i = 0; i < MAX_INODES; i++)
	{
		if(inodes[i].id == 0xFFFFFFFF)
		return i;                            //return free inode viz i
	}
	
	return 0xFFFFFFFF;                           //Indicator of no free inodes
}

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
    
    //Initializing the disk attribute
    disk = _disk;

    /* Here you read the inode list and the free list into memory */
    DiskOperation(DISK_OPERATION::READ, INODES_BLOCK_NO, (unsigned char *)(inodes));
    DiskOperation(DISK_OPERATION::READ, FREELIST_BLOCK_NO, free_blocks);
    
    /*Return true only if the first two blocks have been marked occupied*/
    if(free_blocks[0] == 1 && free_blocks[1] == 1)
    return true;
    else
    return false;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
       
    
    unsigned char buf[DISK_BLOCK_SIZE];
    for(unsigned int i = 0; i < DISK_BLOCK_SIZE; i++)
    {
    	buf[i] = 0xFF;                                              //0xFFFFFFFF denotes nothing meaningful is stored in inodes
    }
    
    _disk->write(INODES_BLOCK_NO, buf);
    
    for(unsigned int i = 0; i < DISK_BLOCK_SIZE; i++)
    {
    	buf[i] = 0x00;
    }
    buf[0] = 0x01;
    buf[1] = 0x01;
    _disk->write(FREELIST_BLOCK_NO, buf);

    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for(unsigned int i = 0; i < MAX_INODES; i++)
    {
    	if(inodes[i].id == _file_id)
    	return &inodes[i];
    }
    Console::puts("No such file exist, returning NULL ");
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
       
       if(LookupFile(_file_id))
       {
       	Console::puts("File already exists");
       	return false;
       }
       
       unsigned long block_no = GetFreeBlock();
       if(block_no == 0xFFFFFFFF)
       {
       	Console::puts("No more free blocks");
       	return false;	
       }
       
       unsigned long inode_index = GetFreeInode();
       
       if(block_no == 0xFFFFFFFF)
       {
       	Console::puts("No more free inodes");
       	return false;	
       }
       
       
       
       //Assign the file properties
       
       free_blocks[block_no] = 1;                 //Occupied block bitmap
       
       inodes[inode_index].id = _file_id;
       inodes[inode_index].block_no = block_no;
       inodes[inode_index].fs = this;              
    
       return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
       
      Inode *node = new Inode();
       
      if(!(node = LookupFile(_file_id)))
       {
       	Console::puts("File doesn't exist");
       	return false;
       }
       
       //Invalidate the file properties
       
       free_blocks[node->block_no] = 0;                 //Free the block bitmap
       
       node->id = 0xFFFFFFFF;
       node->block_no = 0xFFFFFFFF;
       node->size = 0xFFFFFFFF;
       
       DiskOperation(DISK_OPERATION::WRITE, INODES_BLOCK_NO, (unsigned char *)(inodes));
       DiskOperation(DISK_OPERATION::WRITE, FREELIST_BLOCK_NO, free_blocks);
    	
       delete node;
       return true;
       
}

bool FileSystem::DiskOperation(DISK_OPERATION _op, unsigned long _block_no, unsigned char * _buf)
{
	Console::puts("FileSystem DIsk Address "); Console::puti((int)(&disk)); Console::puts("\n");
	
	
	if(_op == DISK_OPERATION::READ)
	disk->read(_block_no, _buf);
	else
	disk->write(_block_no, _buf);
	
	return true;
}


