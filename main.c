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

static char* algor;
static int* frame_table; 		//Array of size nframes, where each element stores the page number that is associated with the frame index.
static struct disk *disk;
static int last_frame_altered;
static int* written_to_blocks;
static int* page_hit_counter; 	//Array of page fault hits per page.
static int page_fault_counter;	//Total of page fault hits
static int total_disk_write;	//Total of disk writes.
static int total_disk_read;		//Total of disk reads.

int find_index(int* frame_table, int page, int nframes)
{
	for (int i=0; i<nframes; i++)
	{
		if (frame_table[i] == page)
		{
			return i;
		}
	}
	return 0;
}

void page_fault_handler( struct page_table *pt, int page )
{
	page_fault_counter++;

	int nframes = page_table_get_nframes(pt);
	/*if (pt->page_bits[page] != 0)
	{
		//page_table_set_entry(pt,page, find_index(frame_table, page, page_table_get_nframes(pt)), PROT_READ|PROT_WRITE);
	}*/
	if (!(strcmp("fifo", algor)))
	{
		char *pm = page_table_get_physmem(pt);											//Save pointer to physical memory in variable pm
		int block = frame_table[last_frame_altered];									//This is the page_number of the page that's going to be writen to memory
		const char *data = &(pm[last_frame_altered*PAGE_SIZE]);						    //Save the pointer to the start of the page that's going to be writen to disk
		disk_write(disk, block, data);													//We write to disk the data on the last frame alterated in physical memory
		total_disk_write++;
		written_to_blocks[frame_table[last_frame_altered]] = 1;							//We indicate that this page is now stored in disk
		if (written_to_blocks[page])													//If the page responsable for the page_fault is stored in disk, we take it to memory
		{
			char *data1 = &(pm[last_frame_altered*PAGE_SIZE]);							//Save the pointer to the start of the page that's going to be writen to physical memory
			disk_read(disk, page, data1);												//We read from disk and save it in memory in the last frame alterated (fifo)
			total_disk_read++;
			written_to_blocks[page] = 0;												//We indicate that this page is no longer in disk
		}
		page_table_set_entry(pt,page, last_frame_altered, PROT_READ|PROT_WRITE);		//Update page_table
		page_table_set_entry(pt,frame_table[last_frame_altered],last_frame_altered,0);
		frame_table[last_frame_altered] = page;											//Update the frame_table
		last_frame_altered++;															//Increse our counter so that we know what the last frame alterated was (fifo)
		if(last_frame_altered == nframes) last_frame_altered = 0;						//We restart out counter in case it reaches the number of frames
		//page_table_print(pt);
		//printf("\n");
	}

	else if (!(strcmp("rand", algor)))
	{
		int random_index = lrand48()%nframes;											//We take a random number between 0 and nframes - 1
		char *pm = page_table_get_physmem(pt);											//We continue the same way as fifo, except know we use our random_index instead of last_frame_alterated
		int block = frame_table[random_index];
		const char *data = &(pm[random_index*PAGE_SIZE]);
		disk_write(disk, block, data);
		total_disk_write++;
		written_to_blocks[frame_table[random_index]] = 1;
		if (written_to_blocks[page])
		{
			char *data1 = &(pm[random_index*PAGE_SIZE]);
			disk_read(disk, page, data1);
			total_disk_read++;
			written_to_blocks[page] = 0;
		}
		page_table_set_entry(pt,page, random_index, PROT_READ|PROT_WRITE);
		page_table_set_entry(pt,frame_table[random_index],random_index,0);
		frame_table[random_index] = page;
		//page_table_print(pt);
		//printf("\n");
	}


	//This algorithm takes into account how many times a page arises a page fault.
	//A less page fault hits means this page is probably not as used as others,
	//so the algorithm swaps the data related to this page to the disk.
	else if (!(strcmp("custom", algor)))
	{	
		page_hit_counter[page]++;											
		int nframes = page_table_get_nframes(pt);
		int counter = -1;
		int frame_index = -1; 	//The frame to be used
		int frame_page; 		//the page related to that frame

		//Gets the frame wich contains the page with less page fault hits.
		for (int i = 0; i < nframes; i++)
		{
			frame_page = frame_table[i];
			if (counter == -1) 
			{
				counter = page_hit_counter[frame_page];
				frame_index = i;
			}
			else if (page_hit_counter[frame_page] < counter) 
			{
				counter = page_hit_counter[frame_page];
				frame_index = i;
			}
		}
		frame_page = frame_table[frame_index];

		char *pm = page_table_get_physmem(pt);
		int block = frame_page;
		const char *data = &(pm[frame_index*PAGE_SIZE]);
		disk_write(disk, block, data);
		total_disk_write++;
		written_to_blocks[frame_page] = 1;
		if (written_to_blocks[page])
		{
			char *data1 = &(pm[frame_index*PAGE_SIZE]);
			disk_read(disk, page, data1);
			total_disk_read++;
			written_to_blocks[page] = 0;
		}
		page_table_set_entry(pt, page, frame_index, PROT_READ|PROT_WRITE);
		page_table_set_entry(pt, frame_page, frame_index, 0);
		frame_table[frame_index] = page;
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

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
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

	//char *physmem = page_table_get_physmem(pt);

	last_frame_altered = 0; //we initialized this varialble.
	written_to_blocks = calloc(npages, sizeof(int));
	page_hit_counter = calloc(npages, sizeof(int));
	page_fault_counter = 0;
	total_disk_write = 0;
	total_disk_read = 0;

	for (int i=0; i<nframes; i++)
	{
		page_table_set_entry(pt,i,i,PROT_READ|PROT_WRITE);
		frame_table[i] = i;
		last_frame_altered++;
		if(last_frame_altered == nframes) last_frame_altered = 0;
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

	printf("page faults: %d\n", page_fault_counter);
	printf("total disk writes: %d\n", total_disk_write);
	printf("total disk readings: %d\n", total_disk_read);
	return 0;
}
