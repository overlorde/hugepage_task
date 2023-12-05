#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Farhan Saif");
MODULE_DESCRIPTION("override dev mem /dev/mem .");
MODULE_VERSION("1.0");

static int bypass(struct kretprobe_instance *probe, struct pt_regs *regs)
{
    regs->ax = 1;

    return 0;
}

static struct kretprobe probe = {
    .handler = bypass,
    .maxactive = 20,
};

static int __init mem_bypass_init(void)
{
    probe.kp.symbol_name = "devmem_is_allowed";
    register_kretprobe(&probe);

    return 0;
}

static void __exit mem_bypass_exit(void)
{
    unregister_kretprobe(&probe);
}

module_init(mem_bypass_init);
module_exit(mem_bypass_exit);
