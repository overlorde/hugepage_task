#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define MEMORY_OFFSET 0x10000000  // Adjust this to the physical address you want to access
#define MEMORY_SIZE   (2*1024*1024*10)        // Adjust this to the size of the memory region you want to access

int main() {
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    void *mem_ptr = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, MEMORY_OFFSET);

    if (mem_ptr == MAP_FAILED) {
        perror("mmap");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }

    memset(mem_ptr, 'A', 100);
    // Now you can access physical memory through mem_ptr
    sleep(30);
    // ...

    if (munmap(mem_ptr, MEMORY_SIZE) == -1) {
        perror("munmap");
    }

    close(mem_fd);

    return 0;
}

