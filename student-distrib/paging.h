/*
* paging.h - header file for paging.c
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-16 13:54:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-16 13:54:13
*/

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"
#include "asm_handler.h"
#include "syscall.h"

#define P_SIZE 1024				/* The number of page directory entries and page table entries */
#define P_SHIFT 12				/* Bit shift to only look at page offset */
#define D_SHIFT 22				/* Bit shift to only look at dir entry */
#define FOUR_KB 4096			/* Number of bytes in a page */
#define FOUR_MB 4194304			/* Number of bytes in a super big page */
#define KERNEL_PDE 0x00400183	/* Present, read/write, 4MB size, 4MB offset, global */
#define KERNEL_PDE_FLAGS 0x183   /* Present, read/write, 4MB size, kernel privilege, global */
#define USER_PDE_FLAGS 0x0087   /* Present, read/write, 4MB size, User privilege */
#define TABLE_PDE 0x00000002	/* Not present, read/write */
#define VMEM_PDE 0x00000003		/* Present, read/write */
#define DEFAULT_PDE 0x2		    /* Not present, read/write */
#define VMEM_OFFSET 0xB8000		/* Offset of video memory as given in lib.c */
#define LSB_10	0x3FF			
#define LSB_12	0xFFF	
#define CR0_PBIT 0x80000000
#define CR4_PBIT 0x00000010
#define EXT_BIT 0x80
#define USER_BIT 0x5
#define NUM_COLS 80
#define NUM_ROWS 25
#define USER_OFFSET 6

/* Function declarations */
void init_paging(void);
void map_page(int pd, void * p_addr, void * v_addr, uint32_t flags);
void ext_map_page(int pd, void * p_addr, void * v_addr, uint32_t flags);
void ext_unmap_page(int pd, void * v_addr);
void unmap_page(int pd); 
void flush_tlb(void);
void set_page_directory(uint32_t pd);
void switch_vidmem(uint32_t old, uint32_t new);

extern uint32_t page_directory[MAX_PROCESSES][P_SIZE] __attribute__((aligned(FOUR_KB)));
extern uint32_t page_table[2 * MAX_PROCESSES - 1][P_SIZE] __attribute__((aligned(FOUR_KB)));

#endif /* _PAGING_H */
