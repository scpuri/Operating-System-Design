/*
* paging.c - initializes and supports paging on the x86
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-16 13:54:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 22:19:24
*/

#include "paging.h"

/* File scope variables */
uint32_t page_directory[MAX_PROCESSES][P_SIZE] __attribute__((aligned(FOUR_KB)));
uint32_t page_table[2 * MAX_PROCESSES - 1][P_SIZE] __attribute__((aligned(FOUR_KB)));

/* 
 * init_paging
 *   DESCRIPTION: Initialize paging by declaring the page directory and table
 * 				  with all entries not present, map the kernel code and video
 *				  memory, then enable paging with the registers CR0,CR3,CR4
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in PDs/PTs
 */
void init_paging(void)
{
	/* Loop through the page directory and table setting all entries to
	 * not present by marking the LSB 0 (in this case all bits 0) */
	int i, j, buf_ptr;
	for(i = 0; i < P_SIZE; i++){
		for(j = 0; j < MAX_PROCESSES; j++){
			page_directory[j][i] = DEFAULT_PDE;
		}
		for(j = 0; j < MAX_PROCESSES * 2 - 1; j++){
			page_table[j][i] = (i * FOUR_KB) | TABLE_PDE;	
		}
	}

	/* Mapping the page table to first page directory entry and set present */
	for(j = 0; j < MAX_PROCESSES; j++){
		page_directory[j][0] = ((uint32_t) page_table[j]) | VMEM_PDE;
		page_table[j][VMEM_OFFSET >> P_SHIFT] |= 1;
	}

	/* Mapping kernel memory to the second page directory entry */
	page_directory[0][1] = KERNEL_PDE;

	/* Set the video memory page frame to present (LSB), the 1,2,3 offsets
	   are for the three copies of video memory mappings (one per terminal) */
	page_table[0][(VMEM_OFFSET >> P_SHIFT) + 1] |= 1;
	page_table[0][(VMEM_OFFSET >> P_SHIFT) + 2] |= 1;
	page_table[0][(VMEM_OFFSET >> P_SHIFT) + 3] |= 1;
	for(i = 0; i < 3; i++){
		buf_ptr = VMEM_OFFSET + (i+1) * FOUR_KB;
		memcpy((void *)buf_ptr, (void *) VMEM_OFFSET, 2*NUM_COLS*NUM_ROWS);
	} 

	/* Initialize the control registers for paging. CR3 gets the address
	 * of the page directory table. CR4 bit 4 gets set for 4MB pages.
	 * CR0 bit 31 gets set to enable paging. */
	__asm__ volatile("\n\t"
				 "init_paging_asm:\n\t"
				 "movl %%eax, %%cr3\n\t"
				 "movl %%cr4, %%eax\n\t"
				 "orl  $0x00000010, %%eax\n\t"
				 "movl %%eax, %%cr4\n\t"
				 "movl %%cr0, %%eax\n\t"
				 "orl  $0x80000000, %%eax\n\t"
				 "movl %%eax, %%cr0\n\t"
			:
			: "a"(page_directory[0]) 
			: "memory", "cc"
			);
	return;
}

/* 
 * void map_page(void * p_addr, void * v_addr, uint32_t flags)
 *   DESCRIPTION: Maps a virtual address to a physical address in the correct
 *				  page table and marks it as present
 *   INPUTS: void * p_addr -- a physical address
 *			 void * v_addr -- the virtal address to map it
 *           uint32_t flags -- the information to set in the page table
 *							   about this address
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in a page table, flushes the TLBs
 */
void map_page(int pd, void * p_addr, void * v_addr, uint32_t flags)
{
	int user = 0;
	if((uint32_t)v_addr > FOUR_MB) user = MAX_PROCESSES - 1;
	/* Get the index in the page directory and table from the virtual address */
	uint32_t pd_index = (uint32_t) v_addr >> D_SHIFT;
	uint32_t pt_index = ((uint32_t) v_addr) >> P_SHIFT & LSB_10;

	/* If the page is extended just return */
	if(page_directory[pd][pd_index] & EXT_BIT) return;

	/* Mark it present and user privilege and map to page table */
	page_directory[pd][pd_index] |= (USER_BIT | ((uint32_t)page_table[pd + user] & (~LSB_12)));

	/* Mark the page present and add its PTE */
    page_table[pd + user][pt_index] = ((uint32_t)p_addr) | (flags & LSB_12) | 0x1; 

    /* Flush the TLB */
    flush_tlb();
}

/* 
 * void ext_map_page(void * p_addr, void * v_addr, uint32_t flags)
 *   DESCRIPTION: Marks present a 4 MB page in the page directory using a virtual
 *				  address to generate the index in the page directory
 *   INPUTS: void * p_addr -- a physical address
 *			 void * v_addr -- the virtal address to map it
 *           uint32_t flags -- the information to set in the page table
 *							   about this address
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in a page directory, flushes the TLBs
 */
void ext_map_page(int pd, void * p_addr, void * v_addr, uint32_t flags)
{
	/* Get the index in the page directory and table from the virtual address */
	uint32_t pd_index = (uint32_t) v_addr >> D_SHIFT;

	/* Mark the page present and add its PTE */
	page_directory[pd][pd_index] = ((uint32_t)p_addr) | (flags & LSB_12) | 0x1;

    /* Flush the TLB */
    flush_tlb();
}

/* 
 * void ext_unmap_page(void * v_addr)
 *   DESCRIPTION: Marks a page corresponding to a particular virtual
 *				  address not present
 *   INPUTS: void * v_addr -- a virtual address that should not exist anymore
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in a page table
 */
void ext_unmap_page(int pd, void * v_addr)
{
	/* Get the index in the page directory and table from the virtual address */
	uint32_t pd_index = (uint32_t) v_addr >> D_SHIFT;

	/* If the PDE is not present return */
	if(!(page_directory[pd][pd_index] & 0x1)) return;

	page_directory[pd][pd_index] &= (~1);

    /* Flush the TLB */
    flush_tlb();
}

/* 
 * void unmap_page(void * v_addr)
 *   DESCRIPTION: Marks a page corresponding to a particular virtual
 *				  address not present
 *   INPUTS: void * v_addr -- a virtual address that should not exist anymore
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in a page table
 */
void unmap_page(int pd) 
{
	/* If the PDE is not present return */
	if(!(page_directory[pd][0] & 0x1)) return;

	/* Mark the page not present in the PTE */
    page_directory[pd][0] &= (~1);

    /* Flush the TLB */
    flush_tlb();
}

/* 
 * void set_page_directory(uint32_t pd)
 *   DESCRIPTION: Changes the value in the PDBR
 *   INPUTS: uint32_t pd - index in page directories
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes the TLB
 */
void set_page_directory(uint32_t pd)
{
	/* Flush the TLB by refreshing CR3 register */
	__asm__ volatile("\n\t"
				 "set_pd:\n\t"
				 "movl %%eax, %%cr3\n\t"
		    :
			: "a"(page_directory[pd]) 
			: "memory", "cc"
			);
}

/* 
 * void flush_tlb(void)
 *   DESCRIPTION: Refreshes all the values in the TLBs, called upon a change in paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes all the TLBs
 */
void flush_tlb(void)
{
	/* Flush the TLB by refreshing CR3 register */
	__asm__ volatile("\n\t"
				 "flush:\n\t"
				 "movl %%cr3, %%eax\n\t"
				 "movl %%eax, %%cr3\n\t"
			:
			:
			: "eax"
			);
}

/* 
 * void switch_vidmem(vuint32_t old, uint32_t new)
 *   DESCRIPTION: Switches between two video memory mappings for terminals
 *   INPUTS: old - the terminal being switched from
 *			 new - the terminal being switched to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes mappings in the paging structures
 */
void switch_vidmem(uint32_t old, uint32_t new){
	int pd;
	pcb_t * pcb;

	/* Do nothing if switching to curr terminal */
	if(old == new) return;

	/* Loop through the active processes  */
	for(pd = 1; pd < MAX_PROCESSES; pd++){
		if((tasks_bitmap & (0x1 << pd))) continue;
		pcb = (pcb_t *)(EIGHT_MB - (pd * EIGHT_KB));

		/* Set the video mapping to point to the buffer for its terminal */
		if(pcb->term == old){
			page_table[pd][VMEM_OFFSET >> P_SHIFT] \
				= (page_table[pd][VMEM_OFFSET >> P_SHIFT] & LSB_12) \
				| (VMEM_OFFSET + FOUR_KB * (old + 1));
			page_table[pd + USER_OFFSET][0] = (page_table[pd + USER_OFFSET][0] & LSB_12) | (VMEM_OFFSET + FOUR_KB * (old + 1));
		}
		/* Set the video mapping to point to the actual video memory */
		else if(pcb->term == new){
			page_table[pd][VMEM_OFFSET >> P_SHIFT] = (page_table[pd][VMEM_OFFSET >> P_SHIFT] & LSB_12) | VMEM_OFFSET;
			page_table[pd + USER_OFFSET][0] = (page_table[pd + USER_OFFSET][0] & LSB_12) | VMEM_OFFSET;
		}
	}

	/* Switch to kernel addr space, swap the vmem buffers, and return to curr process addr space */
	set_page_directory(0);
	memcpy((void *)(VMEM_OFFSET + (old + 1) * FOUR_KB), (void *) VMEM_OFFSET, 2*NUM_ROWS*NUM_COLS);
	memcpy((void *)VMEM_OFFSET, (void *) (VMEM_OFFSET + (new + 1) * FOUR_KB), 2*NUM_ROWS*NUM_COLS);
	set_page_directory(get_pcb()->task_id);
}

