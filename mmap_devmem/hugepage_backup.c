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
#define NUM_HUGEPAGES 2
#define ALLOCATION_SIZE 32
#define TOTAL_ALLOCATION (2*1024*1024/32)
#define START_ADDRESS 0x100000000
// no need for start_address here
#define HUGEPAGE_TYPE_A 1
#define HUGEPAGE_TYPE_B 2

#define MAX_BUFFER_SIZE 1024


void print_memory_info() {
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp != NULL) {
        char buffer[256];  // Adjust the buffer size as needed
        // Search for the VmRSS field in /proc/self/status
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

    // Get the process ID
    pid_t pid = getpid();

    // Formulate the command with the obtained process ID
    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "pmap -x %d", pid);

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    // Close the pipe
    if (pclose(fp) == -1) {
        perror("Error closing pipe");
        exit(EXIT_FAILURE);
    }
}

void hugepage_memory_free(void *hugepage_addr)
{
	if (munmap(hugepage_addr, HUGEPAGE_SIZE) == -1) {
	    printf("hugepage free failed\n");
	    exit(1);
	}
	else{
	    printf("hugepage free success\n");
	}
}

// Object allocation inside hugepage - helper functions


void object_allocation_A(void *hp_addr)
{
    for(int i=1; i<=TOTAL_ALLOCATION; i++)
    {

		if(i%2==0)
		{
		    continue;
		}

		    if(memset(hp_addr + ((i-1)* ALLOCATION_SIZE), 'A', ALLOCATION_SIZE) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
		//	printf("memset success\n");
		    }
    }
}

void object_allocation_B(void *hp_addr)
{
    for(int i=1; i<=TOTAL_ALLOCATION; i++){

		if(i%2==1)
		{
		    continue;
		}

		    if(memset(hp_addr + ((i-1)* ALLOCATION_SIZE), 'B', ALLOCATION_SIZE) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
		//	printf("memset success\n");
		    }
    }
}


void object_allocation_phy(void *addr,void *hp_addr_1, void *hp_addr_2)
{
    for(int i=0; i<TOTAL_ALLOCATION; i++){

		if(i%2==1)
		{
		    continue;
		}

		    if(memmove(addr+i*ALLOCATION_SIZE, hp_addr_1+ALLOCATION_SIZE*i, 32) == NULL)
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

		    if(memmove(addr+i*ALLOCATION_SIZE, hp_addr_2+ALLOCATION_SIZE*i, 32) == NULL)
		    {
			perror("memset failed");
			exit(EXIT_FAILURE);
		    }else{
			//printf("memset success B\n");
		    }
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

    //hugepage 1
    void *hugepage_addr1 = NULL;


    hugepage_addr1 = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_POPULATE, fd_1, 0);
    if (hugepage_addr1 == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd_1);
        exit(EXIT_FAILURE);
    }
    else{
	printf("hugepage allocation success\n");
	}


    //hugepage 2

    void *hugepage_addr2 = NULL;

    hugepage_addr2 = mmap(NULL,HUGEPAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE | MAP_HUGETLB , fd_1, 0);
    if (hugepage_addr2 == MAP_FAILED) {
        perror("Error mapping memory");
        close(fd_1);
        exit(EXIT_FAILURE);
    }
    else{
	printf("hugepage allocation success\n");
    }


    object_allocation_A(hugepage_addr1);
    object_allocation_B(hugepage_addr2);
print_memory_info();
    //hugepage 5
    //hugepage 6
    //hugepage 7
    //hugepage 8
    //hugepage 9
    //hugepage_10

    // experiment physical ram



    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }


    void *addr =mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , mem_fd, MEMORY_OFFSET);

    if (addr == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

    object_allocation_phy(addr,hugepage_addr1,hugepage_addr2);

     FILE *outputFile = fopen("/home/dante/test/output", "wb");

    // Check if the file is opened successfully
    if (outputFile == NULL) {
        fprintf(stderr, "Unable to open the file.\n");
	hugepage_memory_free(hugepage_addr1);
	hugepage_memory_free(hugepage_addr2);
	if (munmap(addr, MEMORY_SIZE) == -1) {
            perror("munmap");
        }
        return 1; // Return an error code
    }

    // Write data to the file
    int total_byte_written = fwrite(addr, 1, MEMORY_SIZE, outputFile);

    if (total_byte_written != MEMORY_SIZE) {
		fprintf(stderr, "Error writing to the file.\n");
		hugepage_memory_free(hugepage_addr1);
		hugepage_memory_free(hugepage_addr2);
		close(fd_1);
		if (munmap(addr, MEMORY_SIZE) == -1) {
		    perror("munmap");
		}

		close(mem_fd);
		fclose(outputFile);
    }
    // Close the file
    fclose(outputFile);
    getRSS();
//dealloc hugepages
    hugepage_memory_free(hugepage_addr1);
    hugepage_memory_free(hugepage_addr2);
    close(fd_1);
// unlink the hugepage file
    /*if (unlink("/dev/hugepages/hugepage_1") == -1) {
        perror("Error unlinking file");
        close(fd_1);
        exit(EXIT_FAILURE);
    }*/


    // dealloc physical page
	if (munmap(addr, MEMORY_SIZE) == -1) {
            perror("munmap");
        }

    close(mem_fd);

}

