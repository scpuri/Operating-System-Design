/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0
#define ARG_BYTES 1024
#define NAME_SIZE 32
#define FILE_ARRAY_SIZE 8
#define MAX_PROCESSES 7
#define ATTRIB 0x02
#define REG_SIZE 14
#define SIZEOF_LONG 4

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

/* 
 * Information about a file given by the boot block
 * file_name -- the name of the file
 * file_type -- 0 for RTC, 1 for directory, 2 for normal file
 * inode_num -- the inode number of the file
 */
typedef struct dentry {
	char file_name[NAME_SIZE];
	int32_t file_type;
	int32_t inode_num;
} dentry_t;

/* The RWOC functions for a specific file */
typedef struct fops {
	int32_t (*open)(const uint8_t * filename);
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*close)(int32_t fd);
} fops_t;

/*
 * Information about one file
 * f_ops -- RWOC functions specific to that file
 * inode_num -- the inode number of that file
 * file_pos -- the number of bytes of the file that have already been read
 * flags -- 1 if the file is in use, 0 if not
 */
typedef struct file {
	fops_t * f_ops;
	uint32_t inode_num;
	uint32_t file_pos;
	uint32_t flags;
} file_t;

typedef struct pcb pcb_t;

/*
 * Information about the current task being run
 * file_array -- information about up to eight files, 0 and 1 are stdin and stdout
 * task_id -- 0 is reserved for the kernel, 1 is for the term 1 first shell, then the rest are given as they execute
 * ebp -- the ebp for the kernel for this process
 * esp -- the esp for the kernel for this process
 * parent -- a pointer to the pcb of the process that called this one
 * child -- pointer to the pcbs of processes called by this one
 * arg -- arguments to the user program
 * arg_len -- length of arguments to the user program
 * term -- the terminal in which this process in running
 */
struct pcb {
	file_t file_array[FILE_ARRAY_SIZE];
	uint8_t task_id;
	uint32_t ebp;
	uint32_t esp;
	pcb_t * parent;
	pcb_t * child;
	uint8_t arg[ARG_BYTES];
	int arg_len;
	int term;
};

#endif /* ASM */

#endif /* _TYPES_H */
