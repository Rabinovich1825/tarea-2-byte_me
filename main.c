/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int npages;
static int nframes;
static char* algor;
static int* frame_table; //Array of size nframes, where each element stores the page number that is associated with the frame index.
static struct disk *disk;
static int last_frame_alterate;
static int* writen_to_blocks;

void page_fault_handler( struct page_table *pt, int page )
{
	if (!(strcmp("fifo", algor)))
	{
		char *vm = page_table_get_virtmem(pt);											//Save pointer to virtual memory in variable vm
		const char *aux = &(vm[(frame_table[last_frame_alterate])*PAGE_SIZE]);			//Save the pointer to the start of the page that's going to be writen to disk
		disk_write(disk, frame_table[last_frame_alterate], aux);						//We write to disk the data on the last frame alterated in physical memory
		writen_to_blocks[frame_table[last_frame_alterate]] = 1;							//We indicate that this page is now stored in disk
		if (writen_to_blocks[page])														//If the page responsable for the page_fault is stored in disk, we take it to memory
		{
			char *pm = page_table_get_physmem(pt);										//Save pointer to physical memory in variable pm
			char *aux1 = &(pm[last_frame_alterate*PAGE_SIZE]);							//Save the pointer to the start of the page that's going to be writen to physical memory
			disk_read(disk, page, aux1);												//We read from disk and save it in memory in the last frame alterated (fifo)
			writen_to_blocks[page] = 0;													//We indicate that this page is no longer in disk
		}
		page_table_set_entry(pt,page, last_frame_alterate, PROT_READ|PROT_WRITE);		//Update page_table
		page_table_set_entry(pt,frame_table[last_frame_alterate],last_frame_alterate,0);
		frame_table[last_frame_alterate] = page;										//Update the frame_table
		last_frame_alterate++;															//Increse our counter so that we know what the last frame alterated was (fifo)
		if(last_frame_alterate == nframes) last_frame_alterate = 0;						//We restart out counter in case it reaches the number of frames
	}

	else if (!(strcmp("rand", algor)))
	{
		int random_index = lrand48()%nframes;
		char *vm = page_table_get_virtmem(pt);
		const char *aux = &(vm[frame_table[random_index]*PAGE_SIZE]);
		disk_write(disk, frame_table[random_index], aux);
		writen_to_blocks[frame_table[random_index]] = 1;
		if (writen_to_blocks[page])
		{
			char *pm = page_table_get_physmem(pt);
			char *aux1 = &(pm[random_index*PAGE_SIZE]);
			disk_read(disk, page, aux1);
			writen_to_blocks[page] = 0;
		}
		page_table_set_entry(pt,page, random_index, PROT_READ|PROT_WRITE);
		page_table_set_entry(pt,frame_table[random_index],random_index,0);
		frame_table[random_index] = page;
	}

	else if (!(strcmp("custom", algor)))
	{

	}

	else
	{
		printf("Algorithm '%s' not found.\n", algor);
		printf("page fault on page #%d\n",page);
		exit(1);
	}
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	algor = argv[3];
	const char *program = argv[4];

	frame_table = malloc(nframes*sizeof(int));

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	char *virtmem = page_table_get_virtmem(pt);

	char *physmem = page_table_get_physmem(pt);

	last_frame_alterate = 0; //we initialized this varialble.
	writen_to_blocks = calloc(npages, sizeof(int));

	for (int i=0; i<nframes; i++)
	{
		page_table_set_entry(pt,i,i,PROT_READ|PROT_WRITE);
		frame_table[i] = i;
		last_frame_alterate++;
		if(last_frame_alterate == nframes) last_frame_alterate = 0;
		if (i == npages-1) break;
	}
	
	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);

	}

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
