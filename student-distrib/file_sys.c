/*
* @Author: Jack, Charlie, Saurav, Rahul
* @Date:   2016-10-20 20:16:52
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 21:46:57
*/

#include "file_sys.h"

/* File scope variables */
static uint32_t file_addr;
static uint32_t num_inodes;

/* File operations tables */
fops_t file_file_operations = {
	.read = file_read,
	.write = file_write,
	.open = file_open,
	.close = file_close
};

fops_t dir_file_operations = {
	.read = read_directory,
	.write = file_write,
	.open = dir_open,
	.close = file_close
};

/* 
 * init_file_sys(module_t * mod)
 *   DESCRIPTION: initialize the file system by loading address
 *   INPUTS: module_t * mod - struct with relevant filesys.img data
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes pages for file_sys
 */
void init_file_sys(module_t * mod)
{
	/* Loop counter through 124 pages of filesys_img */
	int i;

	/* Map the module pointer */
	for(i = 0; i < MAX_PROCESSES; i++){
		map_page(i,(void *)mod,(void *)mod,DEFAULT_PDE);
	}

	/* Set the file_addr to the base of filesys_img */
	file_addr = mod->mod_start;
	num_inodes = *(((uint32_t *)file_addr) + 1);
}

/* 
 * read_dentry_by_name(const uint8 t* fname, dentry_t* dentry)
 *   DESCRIPTION: Crawls through file system and finds file with name "fname"
 *   INPUTS: uint8_t * fname - name of file to find
 *			 dentry_t * dentry - pointer to struct which we copy data to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	uint32_t curr;

	/* search through directory entries by name and populate dentry when found */
	for(curr = file_addr + NUM_DIR_ENTRIES; curr < file_addr + BLOCK_SIZE; curr += NUM_DIR_ENTRIES){
		if(!strncmp((int8_t *) curr, (int8_t *) fname, MAX_NAME_SIZE)){
			*dentry = *((dentry_t *) curr);
			return 0;
		}
	}
	return -1;
}


/* 
 * read_dentry_by_index (uint32_t index, dentry_t* dentry)
 *   DESCRIPTION: uint32_t index - crawls through file system and find file 
 *								   with input index
 *   INPUTS: uint8_t * index - index of file to find
 *			 dentry_t * dentry - pointer to struct which we copy data to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	uint32_t i, curr = file_addr + NUM_DIR_ENTRIES;
	if(index >= NUM_DIR_ENTRIES) return -1;

	/* search through directory entries */
	for(i = 0; i < index; i++){
		curr += NUM_DIR_ENTRIES;
	}

	/* populate dentry */
	*dentry = *((dentry_t *) curr);
	return 0;
}

/* 
 * read_size (uint32_t inode)
 *   DESCRIPTION: gets the size of a file associated with an inode
 *   INPUTS:  uint32_t inode - inode of file to get size of
 *   OUTPUTS: none
 *   RETURN VALUE: size on success, -1 for failure
 *   SIDE EFFECTS: 
 */
int32_t read_size (uint32_t inode)
{
	return *((uint32_t *) (file_addr + (inode + 1) * BLOCK_SIZE));
}

/* 
 * read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 *   DESCRIPTION: checks if the given inode is within valid range, and if so,
 *                copies the file into buf starting at offset and either goes
 *				  to the end of the file or stops when length bytes are copied
 *   INPUTS: uint32_t inode - inode to check 
 *			 uint32_t offset - starting position in the file
 *			 uint8_t * buf - pointer of buffer to be copied to
 *			 uint32_t length - length of bytes
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes successfully copied or -1 on failure
 *   SIDE EFFECTS: copies data from the file specified by inode into buf
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
   /*
	* curr_inode -- pointer to the beginning of the correct inode block
	* i -- utility for looping
	* count -- the number of bytes copied to buf so far
	* size -- the number of bytes in the file
	* block_ptr -- pointer to the part of the inode block that contains the data block index
	* curr_block -- the index of the data block currently being copied from
	* curr_byte -- the byte that needs to be copied from
	*/
	uint32_t * curr_inode = (uint32_t *) (file_addr + (inode + 1) * BLOCK_SIZE);
	uint32_t i, count = 0, size = *curr_inode;
	uint32_t * block_ptr = (uint32_t *) (curr_inode + 1 + offset / BLOCK_SIZE);
	uint32_t curr_block = *block_ptr;
	uint8_t * curr_byte = (uint8_t *)(file_addr + (curr_block + num_inodes + 1) * BLOCK_SIZE + offset % BLOCK_SIZE);

	/* Check for failure on invalid inode index or offset greater than file size */
	if(inode >= num_inodes || offset > size) return -1;
	if(offset == size) return 0;

	/* Start at the location indicated by the offset and copy until the end of the 
	   data block, the end of the file, or the proper amount has been copied */
	for(i = offset % BLOCK_SIZE; i < BLOCK_SIZE; i++){
		buf[count] = *curr_byte;
		count++;
		if(count == length || count + offset >= size){
			return count;
		}
		curr_byte++;
	}

	/* Keep going through data blocks until the end of the file or the proper amount
	   has been copied */
	while(count < length){
		block_ptr++;
		curr_block = *block_ptr;
		curr_byte = (uint8_t *)(file_addr + (curr_block + num_inodes + 1) * BLOCK_SIZE);
		for(i = 0; i < BLOCK_SIZE; i++){
			buf[count] = *curr_byte;
			count++;
			if(count == length || count + offset >= size){
				return count;
			}
			curr_byte++;
		}
	}

	/* Should never reach here*/
	return count;
}

/* 
 * read_directory()
 *   DESCRIPTION: copies successive names from dir into buf
 *   INPUTS: fd - the file descriptor
 *			 const void* buf - pointer to data to write
 *			 int32_t nbytes - the amount to write
 *   OUTPUTS: none
 *   RETURN VALUE: nbytes - the number of bytes copied
 *   SIDE EFFECTS: increment dir_index of current PCB file
 */
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes)
{
	/* Get the PCB and a pointer to the file_pos */
	pcb_t * pcb = get_pcb();
	uint32_t * dir_index = &(pcb->file_array[fd].file_pos);
	if(nbytes > MAX_NAME_SIZE) nbytes = MAX_NAME_SIZE;

	/* Safety check */
	if(buf == NULL) return -1;

	/* Check for end of dir entries */
	if(*dir_index > NUM_DIR_ENTRIES-1) return 0;

	dentry_t d;
	read_dentry_by_index(*dir_index,&d);
	if(strlen(d.file_name) < MAX_NAME_SIZE){
		nbytes = strlen(d.file_name);
	}

	/* Find the next non-empty directory entry */
	while(d.file_name[0] == '\0' && (*dir_index < NUM_DIR_ENTRIES)){
		(*dir_index)++;
		read_dentry_by_index(*dir_index,&d);
	}

	/* Copy nbytes of file name into buffer */
	memcpy((char*)buf, d.file_name, nbytes);

	/* Move to next directory entry and return success */
	(*dir_index)++;
	if(*dir_index > NUM_DIR_ENTRIES-1) return 0;
	return nbytes;
}


/* 
 * file_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: reads some data from the file into a buffer
 *   INPUTS: fd - the file descriptor
 *			 const void* buf - pointer to data to write
 *			 int32_t nbytes - the amount to write
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if error and bytes copied on success
 *   SIDE EFFECTS: increment file_pos in the file given by the fd
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
	/* Get the current PCB */
	pcb_t * pcb = get_pcb();
	int32_t count;

	/* Check for a valid fd */
	if(fd < MIN_FD || fd > MAX_FD) return -1;

	/* Read the data into the buffer */
	count = read_data(pcb->file_array[fd].inode_num, pcb->file_array[fd].file_pos, (uint8_t*)buf, nbytes);
	if(count == -1) return -1;

	/* Increment the file position and return bytes copied */
	pcb->file_array[fd].file_pos += count;
	return count;
}

/* 
 * file_write(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: NOTHING
 *   INPUTS: fd - the file descriptor
 *			 const void* buf - pointer to data to write
 *			 int32_t nbytes - the amount to write
 *   OUTPUTS: none
 *   RETURN VALUE: -1 always
 *   SIDE EFFECTS: nothing
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/* 
 * file_open(const uint8_t filename)
 *   DESCRIPTION: NA
 *   INPUTS: uint8_t* filename -- the name of the file to open
 *   OUTPUTS: none
 *   RETURN VALUE: always 0
 *   SIDE EFFECTS: 
 */
int32_t file_open(const uint8_t* filename)
{
	return 0;
}

/* 
 * dir_open(const uint8_t filename)
 *   DESCRIPTION: NA
 *   INPUTS: uint8_t* filename -- the name of the file to open
 *   OUTPUTS: none
 *   RETURN VALUE: always 0
 *   SIDE EFFECTS: 
 */
int32_t dir_open(const uint8_t* filename)
{
	return 0;
}

/* 
 * file_close(int32_t fd)
 *   DESCRIPTION: Sets the file given by the fd back to its default state
 *   INPUTS: int32_t fd -- a file descriptor corresponding to the file to close
 *   OUTPUTS: none
 *   RETURN VALUE: always 0
 *   SIDE EFFECTS: 
 */
int32_t file_close(int32_t fd)
{
	return 0;
}

