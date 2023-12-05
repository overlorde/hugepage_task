#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>



#define MEMORY_OFFSET 0x10000000
#define MEMORY_SIZE   (2*1024*1024)

#define HUGEPAGE_CHUNK_LENGTH (2*1024*1024)/32

#define HUGEPAGE_SIZE (2 * 1024 * 1024)
#define NUM_HUGEPAGES 10
#define ALLOCATION_SIZE 32
#define TOTAL_ALLOCATION (2*1024*1024/32)
#define START_ADDRESS 0x100000000
#define HUGEPAGE_TYPE_A 1
#define HUGEPAGE_TYPE_B 2

#define MAX_BUFFER_SIZE 1024

typedef struct{
    void *hugepage_addr;
}hugepage_info;



void getRSS() {
    FILE *fp;
    char buffer[MAX_BUFFER_SIZE];

    pid_t pid = getpid();

    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "pmap -x %d", pid);

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    if (pclose(fp) == -1) {
        perror("Error closing pipe");
        exit(EXIT_FAILURE);
    }
}

void hugepage_memory_free(hugepage_info *hugepage_info_ptr) {
	for (int i = 0; i < NUM_HUGEPAGES; i++) {
		if (hugepage_info_ptr[i].hugepage_addr != NULL) {
			if(munmap(hugepage_info_ptr[i].hugepage_addr, HUGEPAGE_SIZE) == -1) {
				perror("hugepage_memory_free: munmap failed");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// Object allocation inside hugepage - helper functions


void object_allocation_A(hugepage_info *a, int n)
{
    for(int i=0; i<TOTAL_ALLOCATION; i++)
    {

		if(i%2==1)
		{
		    continue;
		}

		    if(memset(a->hugepage_addr + ((i)* ALLOCATION_SIZE), 'A', ALLOCATION_SIZE) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
			//printf("memset successA\n");
		    }
    }
}

void object_allocation_B(hugepage_info *b, int n)
{
    printf("object_allocation_B\n");
    for(int i=0; i<TOTAL_ALLOCATION; i++){

		if(i%2==0)
		{
		    continue;
		}

		    if(memset(b->hugepage_addr + ((i)* ALLOCATION_SIZE), 'B', ALLOCATION_SIZE) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
			//printf("memset successB\n");
		    }
    }
}
// name, 32, hparray, 0, 1

void object_allocation_file(char *file_path, int num_bytes_to_write, hugepage_info *hparray, int pos_1, int pos_2)
{
    FILE *file = fopen(file_path, "wb");
    for(int i = 0; i < HUGEPAGE_CHUNK_LENGTH; i++)
	{
	    if(i%2==1){
		continue;
	    }
	    fseek(file, i*ALLOCATION_SIZE, SEEK_SET);
	    size_t bytes_written = fwrite(hparray[0].hugepage_addr + i*ALLOCATION_SIZE, 1, num_bytes_to_write, file);
	}

    for(int i = 0; i < HUGEPAGE_CHUNK_LENGTH; i++)
	{
	    if(i%2==0){
		continue;
	    }
	    fseek(file, i*ALLOCATION_SIZE, SEEK_SET);
	    size_t bytes_written = fwrite(hparray[1].hugepage_addr + i*ALLOCATION_SIZE, 1, num_bytes_to_write, file);
	}

    fclose(file);

    printf("Data successfully written to %s\n", file_path);
}


void create_hugepage(hugepage_info *a, int fd_1, int i)
{

    a->hugepage_addr = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_POPULATE, fd_1, i*HUGEPAGE_SIZE);
    if (a->hugepage_addr == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd_1);
        exit(EXIT_FAILURE);
    }
    else{
	printf("hugepage allocation success\n");
	}
}



int main()
{


    pid_t pid = getpid();


    // Print PID for reference
    printf("Program PID: %d\n", pid);
    // Open hugetlbfs file for memory mapping
    int fd_1 = open("/mnt/hugetlbfs/hugepage", O_CREAT | O_RDWR, 0755);
    if (fd_1 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Seek to the end of the file to make it large enough
    if (lseek(fd_1, (NUM_HUGEPAGES * HUGEPAGE_SIZE) - 1, SEEK_SET) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

  if (ftruncate(fd_1, NUM_HUGEPAGES * HUGEPAGE_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }


    hugepage_info hparray[NUM_HUGEPAGES];
    for(int i=0; i<NUM_HUGEPAGES; i++)
    {
	    hparray[i].hugepage_addr = NULL;
	    create_hugepage(&hparray[i], fd_1, i);
	    if(i%2==0)
	    {
	    object_allocation_A(&hparray[i], i);
	    }
	    else{
		object_allocation_B(&hparray[i], i);
	    }
    }
    getRSS();

    // experiment physical ram

    object_allocation_file("output_0", ALLOCATION_SIZE, hparray, 0, 1);
    object_allocation_file("output_1", ALLOCATION_SIZE, hparray, 2, 3);
    object_allocation_file("output_2", ALLOCATION_SIZE, hparray, 4, 5);
    object_allocation_file("output_3", ALLOCATION_SIZE, hparray, 6,7);
    object_allocation_file("output_4", ALLOCATION_SIZE, hparray, 8, 9);


    //dealloc hugepages
    hugepage_memory_free(hparray);
    close(fd_1);

}

