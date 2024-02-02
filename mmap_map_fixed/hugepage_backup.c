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
	for (int i = 0; i < 10/*NUM_HUGEPAGES*/; i++) {
		if (hugepage_info_ptr[i].hugepage_addr != NULL) {
			if(munmap(hugepage_info_ptr[i].hugepage_addr, HUGEPAGE_SIZE) == -1) {
				perror("hugepage_memory_free: munmap failed");
				exit(EXIT_FAILURE);
			}
		}
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

void object_copy(void *a, void *b )
{

    for(int i=0; i<10; i++)
    {
	if(i%2 == 0)
	{
	    memcpy(b + i*5, a + i*5, 5);
	}
    }

    memset(a, 0, 50);
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
    int fds[10];

    fds[0] = open("/mnt/hugetlbfs/hugepage_1", O_CREAT | O_RDWR, 0755);
    fds[1] = open("/mnt/hugetlbfs/hugepage_2", O_CREAT | O_RDWR, 0755);
    fds[2] = open("/mnt/hugetlbfs/hugepage_3", O_CREAT | O_RDWR, 0755);
    fds[3] = open("/mnt/hugetlbfs/hugepage_4", O_CREAT | O_RDWR, 0755);
    fds[4] = open("/mnt/hugetlbfs/hugepage_5", O_CREAT | O_RDWR, 0755);
    fds[5] = open("/mnt/hugetlbfs/hugepage_6", O_CREAT | O_RDWR, 0755);
    fds[6] = open("/mnt/hugetlbfs/hugepage_7", O_CREAT | O_RDWR, 0755);
    fds[7] = open("/mnt/hugetlbfs/hugepage_8", O_CREAT | O_RDWR, 0755);
    fds[8] = open("/mnt/hugetlbfs/hugepage_9", O_CREAT | O_RDWR, 0755);
    fds[9] = open("/mnt/hugetlbfs/hugepage_10", O_CREAT | O_RDWR, 0755);

    // Seek to the end of the file to make it large enough

    /*if (ftruncate(fd_1, 5 * HUGEPAGE_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd_2, 5 * HUGEPAGE_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }*/


    hugepage_info hparray[10];
    for(int i=0; i<10; i++)
    {
	    hparray[i].hugepage_addr = NULL;
	    if(i%2==0)
	    {
	    hparray[i].hugepage_addr = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB, fds[i], 0);
		object_allocation_A(&hparray[i]);
	    }
	    else{
	    hparray[i].hugepage_addr = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB, fds[i], 0);
		object_allocation_B(&hparray[i]);
	    }
    }
// object transfer
    printf("transferring operation started \n");


    for(int i = 0; i < 10; i+=2)
    {
	object_copy(hparray[i].hugepage_addr, hparray[i+1].hugepage_addr);
    }

    printf("transferring operation ended \n");

    // unmapping
//1
    void *addr_1 = hparray[0].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }
    close(fds[0]);
    unlink("mnt/hugetlbfs/hugepage_1");

  void *addr_2 = mmap(addr_1, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fds[1], 0);
    if (addr_2 == MAP_FAILED) {
        perror("Remap failed");
        return 1;
    }

 //2
    void *addr_3 = hparray[2].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }

    close(fds[2]);
    unlink("mnt/hugetlbfs/hugepage_3");

    void *addr_4 = mmap(addr_3, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fds[3], 0);
    if (addr_2 == MAP_FAILED) {
        perror("Remap failed");
        return 1;
    }

//3
    void *addr_5 = hparray[4].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }

    close(fds[4]);
    unlink("mnt/hugetlbfs/hugepage_5");


    void *addr_6 = mmap(addr_5, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fds[5], 0);
    if (addr_2 == MAP_FAILED) {
        perror("Remap failed");
        return 1;
    }

//4

    void *addr_7 = hparray[6].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }

    close(fds[6]);
    unlink("mnt/hugetlbfs/hugepage_7");

    void *addr_8 = mmap(addr_7, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fds[7], 0);
    if (addr_2 == MAP_FAILED) {
        perror("Remap failed");
        return 1;
    }

//5

    void *addr_9 = hparray[8].hugepage_addr;
    if (munmap(addr_1, HUGEPAGE_SIZE) == -1) {
        perror("munmap failed");
        return 1;
    }

    close(fds[8]);
    unlink("mnt/hugetlbfs/hugepage_9");

    void *addr_10 = mmap(addr_9, HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_FIXED, fds[9], 0);
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
    for(int i=1; i<=10; i++)
	{
	    if(i%2==1)
	    {
		close(fds[i]);
	    }
	}




    unlink("mnt/hugetlbfs/hugepage_2");
    unlink("mnt/hugetlbfs/hugepage_4");
    unlink("mnt/hugetlbfs/hugepage_6");
    unlink("mnt/hugetlbfs/hugepage_8");
    unlink("mnt/hugetlbfs/hugepage_10");
}

