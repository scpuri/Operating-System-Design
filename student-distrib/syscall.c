/*
* @Author: Jack, Charlie, Saurav, Rahul
* @Date:   2016-10-26 17:08:37
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 23:25:28
*/

#include "syscall.h"

/* File scope variables */
uint8_t tasks_bitmap = NO_PROCESSES;

/* 
 * sys_halt(uint8_t status)
 *   DESCRIPTION: Tears down everything associated with this process, jumps 
 *   			  back to the execute syscall in the parent process
 *   INPUTS: status - unused
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: Alters the pcb, the tss, and the cr3 register
 */
int32_t sys_halt(uint8_t status)
{
	pcb_t * pcb = get_pcb();
	pcb_t * parent;
	uint32_t i, parent_esp, parent_ebp;

	/* Mark all files as not in use */
	for(i = 0; i < NUM_FILES; i++){
		if(pcb->file_array[i].flags == 1) sys_close(i);
	}
	pcb->ebp = 0;
	pcb->esp = 0;
	pcb->term = 0;
	pcb->child = NULL;
	pcb->arg_len = 0;
	for(i = 0; i < ARG_BYTES; i++){
		pcb->arg[i] = '\0';
	}
	
	/* These are things that only need to be done if called this is a child of a shell */
	if(pcb->parent != NULL){
		parent = pcb->parent;
		tss.esp0 = EIGHT_MB - (pcb->parent->task_id-1)*EIGHT_KB - 1;
		set_page_directory(parent->task_id);
		ext_unmap_page(pcb->task_id,(uint8_t*)V_PAGE);
        parent->child = NULL;
		parent_esp = parent->esp;
		parent_ebp = parent->ebp;
	}
	/* Otherwise, just return to kernel control */
	else{
		set_page_directory(0);
	}
	/* Set the current process as no longer running */
	tasks_bitmap |= 0x1 << pcb->task_id;

	/* If you are an OG shell just restart */
	if(pcb->parent == NULL){
		screen_x = 0;
		screen_y = 0;
		saved_x[get_active_term()] = 0;
		saved_y[get_active_term()] = 0;
		sys_fork();
		set_first_flag(1);
		sti();
		while(1);
	}

	pcb->task_id = 0;
	pcb->parent = NULL;

	/* Get the old esp and ebp back and jump to the execute syscall */
	__asm__ volatile("\n\t"
				 "sys_halt_assembly:\n\t"
				 "movl %%eax, %%esp\n\t"
				 "movl %%ecx, %%ebp\n\t"
				 "jmp halt_return\n\t"
				:
				: "a"(parent_esp), "c"(parent_ebp)
				: "memory", "cc"
			);

	/* Should never get here, but return failure if it does */
	return -1;
}

/* 
 * sys_execute(const uint8_t* command)
 *   DESCRIPTION: Sets up the pcb for the new task that is to be executed, copies
 *				  the code to be executed to the location where it starts,
 *				  sets up virtual memory for the new task, and 
 *   INPUTS: command - the string that tells which program to execute
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: Alters the TSS and several processor registers, alters memory
 */
int32_t sys_execute(const uint8_t* command)
{
	/* Local variables */
	uint8_t file_name[FNAME_SIZE];
	int i, pd = 0, user_stack, p_addr, v_addr;
	char local_args[ARG_BYTES];
	int local_arglength = 1;

	/* initialize local buffer to NULL */
	for(i=0; i<ARG_BYTES; i++)
		local_args[i] = '\0';

	/* Get the file name from the first word of the command */
	for(i = 0; i < FNAME_SIZE; i++){
		if(command[i] == ' ' || command[i] == '\0') break;
		file_name[i] = command[i];
	}
	file_name[i] = '\0';

	/* Determine the first available process id (and associated PD) */
	while((tasks_bitmap & (0x1 << pd)) == 0) pd++;

	/* Get the arguments into the PCB */
	while(command[i] == ' ') i++;
	while(i < ARG_BYTES){
		if(command[i] == '\0' || command[i] == ' ') break;
		local_args[local_arglength-1] = command[i];
		local_arglength++;
		i++;
	}
	local_arglength++;
	local_args[i] = '\0';

	/* Load the program */
	p_addr = FOUR_MB + FOUR_MB*pd;
	if(-1 == load_program(pd, (void *)p_addr, (void *)(&v_addr), file_name)) return -1;

	/* Toggle the bitmask bit */
	tasks_bitmap ^= 0x1 << pd;

	/* Map the appropriate vmem page in */
	map_page(pd, (void*)VMEM_OFFSET, (void*)VMEM_OFFSET, (uint32_t)VMEM_PDE);

	/* Set the the TSS ss0 and esp0 */
	tss.esp0 = EIGHT_MB - (pd-1)*EIGHT_KB - 1;
	
	/* Create a PCB and initialize it for the child */
	pcb_t pcb;
	strncpy((int8_t*)pcb.arg,(int8_t*)local_args,local_arglength);

	/* Set up stdin and stdout */
	pcb.file_array[0].f_ops = &term_file_operations;
	pcb.file_array[0].inode_num = 0;
	pcb.file_array[0].file_pos = 0;
	pcb.file_array[0].flags = 1;
	pcb.file_array[1].f_ops = &term_file_operations;
	pcb.file_array[1].inode_num = 0;
	pcb.file_array[1].file_pos = 0;
	pcb.file_array[1].flags = 1;

	/* The rest of the file_array should be initialized to empty */
	for(i = STDOUT; i < NUM_FILES; i++){
		pcb.file_array[i].f_ops = NULL;
		pcb.file_array[i].inode_num = 0;
		pcb.file_array[i].file_pos = 0;
		pcb.file_array[i].flags = 0;
	}

	/* Initialize the rest of the of the pcb */
	pcb.task_id = pd;
	pcb.ebp = EIGHT_MB - (pd-1)*EIGHT_KB - 1;
	pcb.esp = pcb.ebp;

	/* Figure out if it is the first process in its terminal or not */
	pcb.parent = get_pcb();
	pcb.term = pcb.parent->term;
	pcb.parent->child = (pcb_t*)(EIGHT_MB - pd*EIGHT_KB);
	pcb.child = NULL;
	pcb.arg_len = local_arglength;

	/* Set the location of the user addr space */
	user_stack = V_PAGE + FOUR_MB - 1;

	/* save current esp, ebp, and eflags for iret */
	__asm__ volatile("\n\t"
					 "movl %%esp, %%eax\n\t"
					 "movl %%ebp, %%ecx\n\t"
					: "=a"(pcb.parent->esp), "=c"(pcb.parent->ebp)
					: 
					: "memory", "cc"
					);

	/* copy filled out pcb to memory */
	memcpy((void*)(EIGHT_MB - pd*EIGHT_KB),(void*)&pcb, sizeof(pcb_t));

	/* inline assembly for iret routine */
	__asm__ volatile("\n\t"
				 "execute:\n\t"
				 "cli\n\t"
				 "movw $0x2B, %%dx\n\t"
				 "movw %%dx, %%ds\n\t"
				 "movw %%dx, %%es\n\t"
				 "movw %%dx, %%fs\n\t"
				 "movw %%dx, %%gs\n\t"				 
				 "pushl $0x0000002B\n\t"
				 "pushl %%ecx\n\t"
				 "pushfl\n\t"
				 "popl %%edx\n\t"
				 "orl $0x200, %%edx\n\t"
				 "pushl %%edx\n\t"
				 "pushl $0x00000023\n\t"
				 "pushl %%eax\n\t"
				 "iret\n\t"
				 "halt_return:\n\t"
			:
			: "a"(v_addr), "c"(user_stack)
			: "memory", "cc", "%edx"
			);

	return 0;
}

/* 
 * sys_fork(const uint8_t* command)
 *   DESCRIPTION: Sets up the pcb for the new task that is to be executed, copies
 *				  the code to be executed to the location where it starts,
 *				  sets up virtual memory for the new task, but does not iret
 *   INPUTS: command - the string that tells which program to execute
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: Alters the TSS and several processor registers, alters memory
 */
int32_t sys_fork(void)
{
	int i, pd = 0, old_pd = 0, p_addr, v_addr, k_stack;

	/* Determine the first available process id (and associated PD) */
	while((tasks_bitmap & (0x1 << pd)) == 0) pd++;
	if(pd != 1) old_pd = get_pcb()->task_id;

	/* Load the program */
	p_addr = FOUR_MB + FOUR_MB*pd;
	if(-1 == load_program(pd, (void *)p_addr, (void *)(&v_addr), (uint8_t *)"shell")) return -1;

	/* Map the appropriate vmem page in */
	map_page(pd, (void*)VMEM_OFFSET, (void*)VMEM_OFFSET, (uint32_t)VMEM_PDE);

	/* Toggle the bitmask bit */
	tasks_bitmap ^= 0x1 << pd;

	/* Create a PCB and initialize it for the child */
	pcb_t pcb;
	for(i = 0; i < ARG_BYTES; i++) 
		pcb.arg[i] = '\0';

	pcb.arg_len = 0;

	/* Set up stdin and stdout */
	pcb.file_array[0].f_ops = &term_file_operations;
	pcb.file_array[0].inode_num = 0;
	pcb.file_array[0].file_pos = 0;
	pcb.file_array[0].flags = 1;
	pcb.file_array[1].f_ops = &term_file_operations;
	pcb.file_array[1].inode_num = 0;
	pcb.file_array[1].file_pos = 0;
	pcb.file_array[1].flags = 1;

	/* The rest of the file_array should be initialized to empty */
	for(i = STDOUT; i < NUM_FILES; i++){
		pcb.file_array[i].f_ops = NULL;
		pcb.file_array[i].inode_num = 0;
		pcb.file_array[i].file_pos = 0;
		pcb.file_array[i].flags = 0;
	}

	/* Initialize the rest of the of the pcb */
	pcb.task_id = pd;
	pcb.ebp = EIGHT_MB - (pd-1)*EIGHT_KB - 1;
	pcb.esp = pcb.ebp;
	pcb.parent = NULL;
	pcb.term = get_active_term();
	pcb.child = NULL;

	/* Set up the context for the IRET into the process by the scheduler */
	k_stack = EIGHT_MB - (pd-1)*EIGHT_KB - 1 - REG_SIZE * sizeof(uint32_t);

	/* Init EBX, ECX, EDX, ESI, EDI (indices 0-4) */
	for(i=0; i<5; i++)
	{
		*((uint32_t *)(k_stack + i * sizeof(uint32_t))) = 0;
	}

	/* Set up EBP (index 5) as bottom of its kernel stack */
	*((uint32_t *)(k_stack + 5 * sizeof(uint32_t))) = V_PAGE + FOUR_MB - 1;

	/* Init EAX, DS, ES (indices 6-8) */
	for(i=6; i<9; i++)
	{
		*((uint32_t *)(k_stack + i * sizeof(uint32_t))) = 0;
	}

	/* Set up EIP and CS with beginning of Shell code and USER_CS */
	*((uint32_t *)(k_stack + 9 * sizeof(uint32_t))) = v_addr;
	*((uint32_t *)(k_stack + 10 * sizeof(uint32_t))) = USER_CS;

	/* Save current eflags register into the context */
	__asm__ volatile("\n\t"
					 "pushfl\n\t"
					 "popl %%edx\n\t"
					: "=d"(*((uint32_t *)(k_stack + 11 * sizeof(uint32_t))))
					: 
					: "memory"
					);

	/* Set up ESP and SS for user stack */
	*((uint32_t *)(k_stack + 12 * sizeof(uint32_t))) = V_PAGE + FOUR_MB - 1;
	*((uint32_t *)(k_stack + 13 * sizeof(uint32_t))) = USER_DS;

	/* Fill in the EBP and ESP of the pcb */
	pcb.ebp = EIGHT_MB - (pd-1)*EIGHT_KB - 1;
	pcb.esp = k_stack;

	/* Copy filled out pcb to memory */
	memcpy((void*)(EIGHT_MB - pd*EIGHT_KB),(void*)&pcb, sizeof(pcb_t));

	/* Initialize terminal screen */
	clear();

	/* Return to previous address space */
	set_page_directory(old_pd);
	return 0;
}

/* 
 * load_program(int pd, void * p_addr, uint8_t * file_name)
 *   DESCRIPTION: Sets up virtual memory for a new process and copies
 *				  code to that location
 *   INPUTS: pd -- the page directory to swap to
 *			 p_addr -- the physical address to copy the memory to
 *			 file_name -- the name of the file to execute
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: Alters cr3 and loads the program to virtual memory
 */
int32_t load_program(int pd, void * p_addr, void * v_addr, uint8_t * file_name)
{	
	/* Get the size of the file */
	uint8_t m_num[MNUM_SIZE];
	dentry_t d;

	if(-1 == read_dentry_by_name(file_name,&d)) return -1;
	uint32_t size = read_size(d.inode_num);

	/* Do not run more than 6 processes */
	if(pd == MAX_PROCESSES){
		term_write(0,(void*)err_proc,strlen((int8_t*)err_proc));
		return -1;
	}

	/* Map the page in the appropriate page directory for this process */
	ext_map_page(pd, p_addr, (uint8_t*)V_PAGE, USER_PDE_FLAGS);

	/* If the file_name does not exist or is not a program file return failure */
	if(d.file_type != STDOUT) return -1;

	if(-1 == read_data(d.inode_num,0,m_num,SIZEOF_LONG)) return -1;

	/* Check if the file is an executable via its 4 bytes of magic numbers */
	if(MG_1 != m_num[0] || MG_2 != m_num[1] || MG_3 != m_num[2] || MG_4 != m_num[3]) return -1;

	/* Copy the program from the file system into new user frame */
	if(-1 == read_data(d.inode_num,MNUM_OFFSET,(uint8_t*)v_addr,SIZEOF_LONG)) return -1;

	/* Map the kernel page into the process PD */
	ext_map_page(pd, (void *)FOUR_MB, (void *)FOUR_MB, KERNEL_PDE_FLAGS);
	set_page_directory(pd);
	if(-1 == read_data(d.inode_num,0,(uint8_t*)V_ADDR,size)) return -1;

	/* Return success */
	return 0;
}

/* 
 * sys_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: fd - 
 *			 buf -
 *			 nbytes - 
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: clears the file scope int_has_occurred flag
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
{
	/* Call the appropriate read function and return its return value */
	if(fd > NUM_FILES-1 || fd < 0 || fd == 1) return -1;
	if(get_pcb()->file_array[fd].flags == 0) return -1;
	return get_pcb()->file_array[fd].f_ops->read(fd,buf,nbytes);
}
/* 
 * sys_write(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: fd - 
 *			 buf -
 *			 nbytes - 
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, nbytes on success
 *   SIDE EFFECTS: 
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
	/* Call the appropriate write function and return its return value */
	if(fd > NUM_FILES-1 || fd < 0 || fd == 0) return -1;
	if(get_pcb()->file_array[fd].flags == 0) return -1;
	return get_pcb()->file_array[fd].f_ops->write(fd,buf,nbytes);
}

/* 
 * sys_open(const uint8_t filename)
 *   DESCRIPTION: Finds an open entry in the file array and sets it for the given file
 *   INPUTS: filename - the name of the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: Changes the file_array
 */
int32_t sys_open(const uint8_t* filename)
{
	int fd;
	dentry_t dentry;
	pcb_t * pcb = get_pcb();
	file_t * file;

	/* Check for null pointer or invalid name */
	if(filename == NULL || filename[0] == '\0') return -1;

	for(fd = 0; fd < NUM_FILES; fd++){
		if(pcb->file_array[fd].flags == 0) break;
	}
	if(fd == NUM_FILES) return -1;
	file = &(pcb->file_array[fd]);

	/* Set dentry to hold info about the file */
	if(-1 == read_dentry_by_name(filename, &dentry)) return -1;

	file->inode_num = dentry.inode_num;
	file->file_pos = 0;
	file->flags = 1;

	/* Check for directory */
	if(dentry.file_type == 0){
		file->f_ops = &rtc_file_operations;
	}
	else if(dentry.file_type == 1){
		file->f_ops = &dir_file_operations;
	}
	else if(dentry.file_type == STDOUT){
		file->f_ops = &file_file_operations;
	}

	file->f_ops->open(filename);

	/* Return the fd */
	return fd;
}

/* 
 * sys_close(int32_t fd)
 *   DESCRIPTION: Close a file
 *   INPUTS: fd - the file descriptor to close
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: none
 */
int32_t sys_close(int32_t fd)
{
	/* Make sure the file descriptor is valid */
	if(fd > NUM_FILES-1 || fd < STDOUT) return -1;

	/* Set the flag to closed */
	pcb_t * pcb = get_pcb();
	if(pcb->file_array[fd].flags == 0) return -1;

	/* Call the correct close function */
	pcb->file_array[fd].f_ops->close(fd);

	/* Clear the file_t fields */
	pcb->file_array[fd].flags = 0;
	pcb->file_array[fd].f_ops = NULL;
	pcb->file_array[fd].file_pos = 0;
	pcb->file_array[fd].inode_num = 0;

	/* Return success */
	return 0;
}
/* 
 * sys_getargs(uint8_t* buf, int32_t bytes)
 *   DESCRIPTION: Returns the arguments in PCB from a sys_execute
 *   INPUTS: buf - buffer to copy args into
 *			 nbytes - number of bytes of args to copy
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{
	/* Null ptr check */
	if(buf == NULL) return -1;

	/* Get the PCB of the current process */
	pcb_t * pcb = get_pcb();

	/* If the arguments don't fit in buf return -1 */
	if(pcb->arg_len > nbytes) return -1;

	/* Copy over the arg array to the buf and return success */
	strncpy((int8_t*)buf, (int8_t*)pcb->arg, nbytes);
	return 0;
}

/* 
 * sys_vidmap(uint8_t** screen_start)
 *   DESCRIPTION: Maps a page for video memory and returns wirtual pointer in screen_start
 *   INPUTS: screen_start - pointer to pointer of returned v_addr
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t sys_vidmap(uint8_t** screen_start)
{
	/* Check if the screen_start is valid */
	if((uint32_t)screen_start < V_PAGE || (uint32_t)screen_start >= V_PAGE+FOUR_MB) return -1;

	/* If its the active term map to real vmem */
	if(get_pcb()->term == get_active_term()){
		map_page(get_pcb()->task_id,(void*) VIDEO, (void*) USER_VMEM, VIDEO_FLAGS);
	}
	/* If inactive term map to buffer */
	else{
		map_page(get_pcb()->task_id,(void*) VIDEO + (get_pcb()->term + 1) * FOUR_KB, (void*) USER_VMEM, VIDEO_FLAGS);
	}

	/* Set screen_start to USER_VMEM */
	*screen_start = (uint8_t*)USER_VMEM;

	/* Return success */
	return 0;
}
/* 
 * sys_set_handler(int32_t signum, void* handler_address)
 *   DESCRIPTION: 
 *   INPUTS: signum -
 *			 handler_address - 
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t sys_set_handler(int32_t signum, void* handler_address)
{
	return 0;
}

/* 
 * sys_sigreturn(void)
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, 0 on success
 *   SIDE EFFECTS: 
 */
int32_t sys_sigreturn (void)
{
	return 0;
}


