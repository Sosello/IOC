#include <linux/module.h>
#include <linux/init.h>

#include "ledbp.h"

//Gestion du driver
//-----------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie, 2015");
MODULE_DESCRIPTION("Module, aussitot insere, aussitot efface");

static int major;

#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");

static int __init mon_module_init(void)
{
    int i;
    printk(KERN_DEBUG "Hello World ! dauvet chen\n");
    for (i=0; i < nbled; i++)
       printk(KERN_DEBUG "LED %d = %d\n", i, leds[i]);

    major = register_chrdev(0, DRV_NAME, &fops_led); // 0 est le numéro majeur qu'on laisse choisir par linux


    return 0;
}

static void __exit mon_module_cleanup(void)
{
   printk(KERN_DEBUG "Goodbye World!\n");
   unregister_chrdev(major, DRV_NAME);
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);

