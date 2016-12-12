/*
* file_sys.h - header file for file_sys.c
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 17:39:00
* @Last Modified by:   Charlie
* @Last Modified time: 2016-10-15 17:39:00
*/

#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "multiboot.h"
#include "types.h"
#include "lib.h"
#include "paging.h"

#define BLOCK_SIZE 4096
#define NIB_SIZE 4 
#define NUM_DIR_ENTRIES 64
#define MAX_NAME_SIZE 32
#define NUM_FSYS_PAGES 124
#define MAX_FD 7
#define MIN_FD 2

extern fops_t file_file_operations;
extern fops_t dir_file_operations;

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

void init_file_sys(module_t * mod);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t dir_open(const uint8_t* filename);
int32_t file_close(int32_t fd);
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);
int32_t read_size (uint32_t inode);

#endif /* _FILE_SYS_H */
