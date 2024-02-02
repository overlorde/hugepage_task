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

void hugepage_memory_free(hugepage_info *hugepage_info_ptr) {
	for (int i = 0; i < 2/*NUM_HUGEPAGES*/; i++) {
		if (hugepage_info_ptr[i].hugepage_addr != NULL) {
			if(munmap(hugepage_info_ptr[i].hugepage_addr, HUGEPAGE_SIZE) == -1) {
				perror("hugepage_memory_free: munmap failed");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void create_hugepage_A(hugepage_info *a, int fd_1, int i)
{

    a->hugepage_addr = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_POPULATE, fd_1, 0);
    if (a->hugepage_addr == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd_1);
        exit(EXIT_FAILURE);
    }
    else{
	printf("hugepage allocation success\n");
	}
}
void create_hugepage_B(hugepage_info *a, int fd_2, int i)
{

    a->hugepage_addr = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_POPULATE, fd_2, 0);
    if (a->hugepage_addr == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd_2);
        exit(EXIT_FAILURE);
    }
    else{
	printf("hugepage allocation success\n");
	}
}

//5 bytes each
void object_allocation_A(hugepage_info *a)
{
    for(int i=0; i<10; i++)
    {
	if(i%2 == 0)
	{
	    memset(a->hugepage_addr + i*5, 'A', 5);
	}
    }
}

void object_allocation_B(hugepage_info *a)
{
    for(int i=0; i<10; i++)
    {
	if(i%2 == 1)
	{
	    memset(a->hugepage_addr + i*5, 'B', 5);
	}
    }
}

int main()
{

    int fd_1 = open("/mnt/hugetlbfs/hugepage_1", O_CREAT | O_RDWR, 0755);
    int fd_2 = open("/mnt/hugetlbfs/hugepage_2", O_CREAT | O_RDWR, 0755);
    if (fd_1 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fd_2 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Seek to the end of the file to make it large enough
    if (lseek(fd_1, (1 * HUGEPAGE_SIZE) - 1, SEEK_SET) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd_2, (1 * HUGEPAGE_SIZE) - 1, SEEK_SET) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd_1, 1 * HUGEPAGE_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd_2, 1 * HUGEPAGE_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }



    hugepage_info hparray[2];
    for(int i=0; i<2; i++)
    {
	    hparray[i].hugepage_addr = NULL;
	    if(i%2==0)
	    {
	    create_hugepage_A(&hparray[i], fd_1, i);
	    object_allocation_A(&hparray[i]);
	    }
	    else{
	    create_hugepage_B(&hparray[i], fd_2, i);
	    object_allocation_B(&hparray[i]);
	    }
    }

    printf("Copying operation started \n");
    memcpy(hparray[1].hugepage_addr, hparray[0].hugepage_addr, 5);

    printf("Copying operation ended \n");

    void *addr_1 = hparray[0].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }

  void *addr_2 = mmap(addr_1, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fd_2, 0);
    if (addr_2 == MAP_FAILED) {
        perror("Remap failed");
        return 1;
    }

    printf(" huge page mapped at address_1: %p\n", addr_1);
    printf(" huge page mapped at address_2: %p\n", addr_2);
    printf(" huge page mapped at address for 2nd hugepage: %p\n", hparray[1].hugepage_addr);



    /*
    for (int i = 0; i < 10; ++i) {
	printf("%d Object A - %c\n",i, object_1[i]);
	printf("%d Object B - %c\n",i, object_2[i]);
    }
    */

    hugepage_memory_free(hparray);
    close(fd_1);
    close(fd_2);
}

