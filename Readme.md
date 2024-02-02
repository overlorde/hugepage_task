## Hugepage Allocation and mapping on physical memory
The folder page_table_manipulation hosts the codes to allocate (4kb)pages and mapping them on the physical memory .
```
cd page_table_manipulation
make
sudo insmod pte_remap.ko
sudo dmesg
```
It shows that, through pte entry modification, pages can be remapped


The folder without_mmap_devmem maps to the physicl memory using file operations

```
cd mmap_map_fixed
gcc hugepage_task.c -o hugepage_task
```
and filecalc.py is used to calculate the byte consumption.

it shows, hugepages can be allocated, virtual addresses can be remapped through map_fixed keeping the same virtual address.

/proc/sys/vm/nr_hugepages
