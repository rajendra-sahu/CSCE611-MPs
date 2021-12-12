/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    
    fs = _fs;
    id = _id;
    inode = fs->LookupFile(_id);
    current_position = 0;
    
    fs->DiskOperation(DISK_OPERATION::READ, inode->block_no, block_cache);          //Reading the file into primamry memory

}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    
    
    fs->DiskOperation(DISK_OPERATION::WRITE, inode->block_no, block_cache);   //Backing up latest file state (viz data block and inode)to the disk since we are closing the file
    inode->inodes_to_and_from_disk(DISK_OPERATION::WRITE);                                            
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    
    unsigned int read_count = 0;
    bool EoF_flag;
    
    while(  ((EoF_flag =EoF()) == false) && (read_count<= _n))
    {
    	_buf[read_count] = block_cache[current_position];
    	read_count++;
    	current_position++;
    }
    
    if(EoF_flag)
    Reset();
    
    if(current_position == 0)
    return read_count;                //return actual count since it hadn't exceeded the limit EoF case
    else
    return read_count-1;              //count has exceeded the limit
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    
    unsigned int write_count = 0;
    
    if(_n > SimpleDisk::BLOCK_SIZE)            //If the intended size to be written is greater than a block trim it to the block
    _n = SimpleDisk::BLOCK_SIZE;
    
    while(write_count<= _n)
    {
    	if(EoF())
    	Reset();
    	block_cache[current_position] = _buf[write_count];
    	write_count++;
    	current_position++;
    }
    
    inode->size = inode->size + write_count;
    return write_count-1;                      //Return n-1 since it had been incremented beyond the limit by 1

}

void File::Reset() {
    Console::puts("resetting file\n");
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    if(current_position == SimpleDisk::BLOCK_SIZE)
    return true;
    else
    return false;
}
