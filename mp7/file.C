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
    
    //fs->disk->read(inode->block_no, block_cache);     //Reading the file into primamry memory
    fs->DiskOperation(DISK_OPERATION::READ, inode->block_no, block_cache);
    //assert(false);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    
    //fs->disk->write(inode->block_no, block_cache);        
    //fs->disk->write(0, (unsigned char*)(fs->inodes));
    
    fs->DiskOperation(DISK_OPERATION::WRITE, inode->block_no, block_cache);   //Backing up latest file satte to the disk since we are closing the file.   TODO 
    inode->inodes_to_and_from_disk(DISK_OPERATION::WRITE);                                            
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    
    unsigned int read_count = 0;
    bool EoF_flag;
    
    while(!(EoF_flag =EoF()) && (read_count<= _n))
    {
    	_buf[read_count] = block_cache[current_position];
    	read_count++;
    	current_position++;
    }
    
    if(EoF_flag)
    Reset();
    
    return read_count-1;                //Check if the value is correct or not   TODO
    //assert(false);
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    
    unsigned int write_count = 0;
    while(!(write_count > SimpleDisk::BLOCK_SIZE) && (write_count<= _n))
    {
    	if(EoF())
    	Reset();
    	block_cache[current_position] = _buf[write_count];
    	write_count++;
    	current_position++;
    }
    
    inode->size = inode->size + write_count;
    return write_count;                //Check if the value is correct or not    TODO
    //assert(false);
}

void File::Reset() {
    Console::puts("resetting file\n");
    current_position = 0;
    //assert(false);
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    if(current_position == SimpleDisk::BLOCK_SIZE)
    return true;
    else
    return false;
    //assert(false);
}
