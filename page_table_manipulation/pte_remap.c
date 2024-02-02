#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/highmem.h>
//#include <linux/memory.h>
#include <asm/tlbflush.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Farhan Saif");
MODULE_DESCRIPTION("pte remap");

static int *int_ptr_1 = NULL;
static int *int_ptr_2 = NULL;
#define OBJ_SIZE 10
#define NUM_OBJS 5

typedef struct {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long address;
    unsigned long physical_address;
} PageInfo;


pte_t *getPTE(unsigned long address)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(current->mm, address);
    p4d = p4d_offset(pgd, address);
    pud = pud_offset(p4d, address);
    pmd = pmd_offset(pud, address);
    pte = pte_offset_kernel(pmd, address);
    return pte;
}

unsigned long getPhysicalAddress(unsigned long address)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(current->mm, address);

    if (pgd_none(*pgd) || pgd_bad(*pgd))
        return -1;

    p4d = p4d_offset(pgd, address);

    if (p4d_none(*p4d) || p4d_bad(*p4d)){
	printk(KERN_INFO "return -1 p4d\n");
        return -1;
    }
    pud = pud_offset(p4d, address);
    if (pud_none(*pud) || pud_bad(*pud))
        return -1;

    pmd = pmd_offset(pud, address);

   if (pmd_none(*pmd) || pmd_bad(*pmd))
        return -1;

    pte = pte_offset_kernel(pmd, address);

    if (!pte)
        return -1;
    unsigned long physical_address = (pte_val(*pte) & PAGE_MASK) | (address & ~PAGE_MASK);

    printk(KERN_INFO "PGD %lx\n", pgd_val(*pgd));
    printk(KERN_INFO "PTE %lx\n", pte_val(*pte));

    return physical_address;
}

/*void getNewPage(void *page_addr)
{
    int i;

    struct page* my_page;

    my_page = alloc_pages(GFP_KERNEL, 0); // 0 as the order for a single page
    if (!my_page) {
	printk(KERN_ERR "Page allocation failed\n");
	//return -ENOMEM;
    }

    page_addr = page_address(my_page);

    char* objects[NUM_OBJS];
    for (int i = 0; i < NUM_OBJS; ++i) {
	objects[i] = (char*)page_addr + i * OBJ_SIZE;
	memset(objects[i], 'A', OBJ_SIZE);
    }
}
*/
static unsigned long page_addr_1;
static unsigned long page_addr_2;

static int __init pte_remap_init(void) {


    int i;
    struct page* my_page;


    struct page* page_1 =alloc_pages(GFP_KERNEL, 0);
    struct page* page_2 =alloc_pages(GFP_KERNEL, 0);


    page_addr_1 = page_address(page_1);
    page_addr_2 = page_address(page_2);

    if (!page_addr_1) {
	free_pages(page_addr_2, 0);
	printk(KERN_ERR "Page allocation failed\n");
	return -ENOMEM;
    }
    if (!page_addr_2) {
	free_pages(page_addr_1, 0);
	printk(KERN_ERR "Page allocation failed\n");
	return -ENOMEM;
    }



    unsigned long address_1 = page_addr_1;
    unsigned long address_2 = page_addr_2;

    printk(KERN_INFO "debug point three - address_1 assign\n");

    unsigned long physical_address_1 = getPhysicalAddress(address_1);
    unsigned long physical_address_2 = getPhysicalAddress(address_2);
    //unsigned long physical_address_2 = getPhysicalAddress(address_2, &page_info_2);

    printk(KERN_INFO "debug point four - physical_address assign %lx\n", physical_address_1);
    printk(KERN_INFO "debug point four - physical_address assign %lx\n", physical_address_2);

    if(physical_address_1 == -1 || physical_address_2 == -1){
	if(physical_address_1 == -1 && physical_address_2 > -1){
	    free_pages(page_addr_2, 0);
	}
	else if(physical_address_2 == -1 && physical_address_1 > -1){
	    free_pages(page_addr_1, 0);
	}
	printk(KERN_INFO "physical address is -1 page allocation failed for one\n");
	return -1;
    }

    printk(KERN_INFO "page allocation sucess - object allocating\n");
    char *object_1;
    char *object_2;
    object_1 = (char*)page_addr_1;
    object_2 = (char*)page_addr_2;

    for (int i = 0; i < 100; ++i) {
	object_1[i] = 'A';
	object_2[i] = 'B';
    }



    for (int i = 0; i < 4; ++i) {
    printk(KERN_INFO "%d Object A - %c\n",i, object_1[i]);
    printk(KERN_INFO "%d Object B - %c\n",i, object_2[i]);
    }
    // physical memory to kernel virtual memory

    printk(KERN_INFO "debug point five - kmap\n");

    // kernel virtual memory to physical memory

    void *vaddr_1 = kmap(page_1);
    void *vaddr_2 = kmap(page_2);

    if(vaddr_1 && vaddr_2 ){
	printk(KERN_INFO "debug point six - kmap sucess\n");
	memcpy(vaddr_2, vaddr_1, 3);
	kunmap(page_1);
	kunmap(page_2);
    }
    else{
	printk(KERN_INFO "kmap virtual address allocation failed \n");
	return -1;
    }

    printk(KERN_INFO "debug point five - kmap ends\n");

    /*
       page remapping
    */


    printk(KERN_INFO "remapping starts\n");
    pte_t new_pte;
    pte_t *pte = getPTE(address_1);
    pgprot_t pgprot = __pgprot(pte_flags(*pte));
    new_pte = pfn_pte(physical_address_2 >> PAGE_SHIFT, pgprot);
    set_pte_at(current->mm, address_1, pte, new_pte);

    printk(KERN_INFO "remapping ends\n");

    // flush_tlb_mm - implicit why

    __flush_tlb_all();

    printk(KERN_INFO "flush tlb done\n");
    for (int i = 0; i < 6; ++i) {
    printk(KERN_INFO "%d Object A - %c\n",i, object_1[i]);
    printk(KERN_INFO "%d Object B - %c\n",i, object_2[i]);
    }

    return 0;
}

static void __exit pte_remap_exit(void) {
    if(page_addr_1)
    {
	free_pages(page_addr_1, 0);
	printk(KERN_INFO "Page freed\n");
    }
    if(page_addr_2)
    {
	free_pages(page_addr_2, 0);
	printk(KERN_INFO "Page freed\n");
    }
    printk(KERN_INFO "Exiting Page Table Example Module\n");
}

module_init(pte_remap_init);
module_exit(pte_remap_exit);

