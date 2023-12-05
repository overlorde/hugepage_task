## Hugepage Allocation and mapping on physical memory
The folder mmap_devmem hosts the codes to allocate hugepages and mapping them on the physical memory with direct access to ram.
```
cd mmap_devmem
cd kernelmembypass
make
sudo insmod membypass.ko
cd ..
gcc hugepage_task.c -o hugepage_task
```
The folder without_mmap_devmem maps to the physicl memory using file operations

```
cd without_mmap_devmem
gcc hugepage_task.c -o hugepage_task
```
and filecalc.py is used to calculate the byte consumption.
