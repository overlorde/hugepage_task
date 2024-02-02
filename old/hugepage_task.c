#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>



#define MEMORY_OFFSET 0x10000000
#define MEMORY_SIZE   (2*1024*1024)

#define PHYSICAL_LIMIT (2*1024*1024)/32

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


void print_memory_info() {
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp != NULL) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            int rss;
            if (sscanf(buffer, "VmRSS: %d", &rss) == 1) {
                printf("RSS Memory Usage: %d KB\n", rss);
                break;
            }
        }
        fclose(fp);
    } else {
        perror("Error opening /proc/self/status");
        exit(EXIT_FAILURE);
    }
}

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


void object_allocation_phy(void *mem_addr,int mem_fd,hugepage_info *hp_1, hugepage_info *hp_2)
{

    for(int i=0; i<TOTAL_ALLOCATION; i++){

		if(i%2==1)
		{
		    continue;
		}

		    if(memmove(mem_addr+i*ALLOCATION_SIZE, hp_1->hugepage_addr + ALLOCATION_SIZE*i, 32) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
			//printf("memset success A\n");
		    }
    }
    for(int i=0; i<TOTAL_ALLOCATION; i++){

		if(i%2==0)
		{
		    continue;
		}

		    if(memmove(mem_addr+i*ALLOCATION_SIZE, hp_2->hugepage_addr+ALLOCATION_SIZE*i, 32) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
			//printf("memset success B\n");
		    }
    }
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


    printf("Program PID: %d\n", pid);
    // After mounting the hugetable file system, Open hugetlbfs file for memory mapping
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

    // physical ram

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }


//0
    void *mem_addr_0 =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET);

    if (mem_addr_0 == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(mem_addr_0, mem_fd, &hparray[0],&hparray[1]);

    FILE *outputFile_0 = fopen("/home/dante/work/hugetable_task/mmap_devmem/output_0", "wb");
    fwrite(mem_addr_0, 1, MEMORY_SIZE, outputFile_0);

    fclose(outputFile_0);

//1



    void *mem_addr_1 =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET+MEMORY_SIZE);

    if (mem_addr_1 == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(mem_addr_1, mem_fd, &hparray[2],&hparray[3]);

    FILE *outputFile_1 = fopen("/home/dante/work/hugetable_task/mmap_devmem/output_1", "wb");
    fwrite(mem_addr_1, 1, MEMORY_SIZE, outputFile_1);

    fclose(outputFile_1);


//  2


    void *mem_addr_2 =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET+MEMORY_SIZE*2);

    if (mem_addr_2 == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(mem_addr_2, mem_fd, &hparray[4],&hparray[5]);

    FILE *outputFile_2 = fopen("/home/dante/work/hugetable_task/mmap_devmem/output_2", "wb");
    fwrite(mem_addr_2, 1, MEMORY_SIZE, outputFile_2);

    fclose(outputFile_2);

// 3


    void *mem_addr_3 =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET+MEMORY_SIZE*3);

    if (mem_addr_3 == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(mem_addr_3, mem_fd, &hparray[6],&hparray[7]);

    FILE *outputFile_3 = fopen("/home/dante/work/hugetable_task/mmap_devmem/output_3", "wb");
    fwrite(mem_addr_3, 1, MEMORY_SIZE, outputFile_3);

    fclose(outputFile_3);
//4
    void *mem_addr_4 =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET+MEMORY_SIZE*4);

    if (mem_addr_4 == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(mem_addr_4, mem_fd, &hparray[8],&hparray[9]);

    FILE *outputFile_4 = fopen("/home/dante/work/hugetable_task/mmap_devmem/output_4", "wb");
    fwrite(mem_addr_4, 1, MEMORY_SIZE, outputFile_4);

    fclose(outputFile_4);

    getRSS();
    //commeent ends

    //dealloc hugepages

    hugepage_memory_free(hparray);
    close(fd_1);

    getRSS();
    // dealloc physical page
	if (munmap(mem_addr_0, MEMORY_SIZE) == -1) {
            perror("munmap");
        }
	if (munmap(mem_addr_1, MEMORY_SIZE) == -1) {
            perror("munmap");
        }

	if (munmap(mem_addr_2, MEMORY_SIZE) == -1) {
            perror("munmap");
        }
	if (munmap(mem_addr_3, MEMORY_SIZE) == -1) {
            perror("munmap");
        }
	if (munmap(mem_addr_4, MEMORY_SIZE) == -1) {
            perror("munmap");
        }

    close(mem_fd);
}

