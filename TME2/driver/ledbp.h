#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <mach/platform.h>


char DRV_NAME[246] = "ledbp_dauvet_chen";

//Gestion GPIO
//-----------------------------------


#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

#define GPIO_LED0   4
#define GPIO_LED1   17
#define GPIO_BP     18

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1

struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3];
    uint32_t gpclr[3];
    uint32_t gplev[3];
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
}
volatile *gpio_regs_virt = (struct gpio_s *)__io_address(BCM2835_GPIO_BASE);


static void 
gpio_fsel(uint32_t pin, uint32_t fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs_virt->gpfsel[reg] = (gpio_regs_virt->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void 
gpio_write (uint32_t pin, uint32_t val)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    if (val == 1) 
        gpio_regs_virt->gpset[reg] = (1 << bit);
    else
        gpio_regs_virt->gpclr[reg] = (1 << bit);
}

static char
gpio_read (uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    //printk("bp value %d\n",((gpio_regs_virt->gplev[reg] & (0x1 << bit))));

    return ((gpio_regs_virt->gplev[reg] & (0x1 << bit)) == 0)?'1':'0';
}

//Primitive du driver
//-----------------------------------

static int 
open_led_dauvet_chen(struct inode *inode, struct file *file) {


    // Setup GPIO of LED0 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED0, GPIO_FSEL_OUTPUT);

    // Setup GPIO of LED1 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED1, GPIO_FSEL_OUTPUT);

    // Setup GPIO of BP to INPUT
    // ---------------------------------------------
    
    gpio_fsel(GPIO_BP, GPIO_FSEL_INPUT);
    
    printk(KERN_DEBUG "open_led_dauvet_chen()\n");
    return 0;
}

static ssize_t 
read_led_dauvet_chen(struct file *file, char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "read()\n");
    //on lis la valeur du bouton poussoir
    buf[0] = gpio_read(GPIO_BP);
    buf[1] = '\0';
    printk("read bp %d : %c\n",GPIO_BP,buf[0]);
    
    return 2;
}

static ssize_t 
write_led_dauvet_chen(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "write()\n");
    //dans buf 1 er octet LED 2eme octet valeur
    uint32_t led_no = (buf[0] == '0') ? GPIO_LED0 : GPIO_LED1;
    uint32_t val = (buf[1] == '1') ? 1 : 0;
    gpio_write(led_no,val);
    printk("write led %d : %d\n",led_no,val);

    return count;
}

static int 
release_led_dauvet_chen(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "release_led_dauvet_chen()\n");
    return 0;
}

struct file_operations fops_led =
{
    .open       = open_led_dauvet_chen,
    .read       = read_led_dauvet_chen,
    .write      = write_led_dauvet_chen,
    .release    = release_led_dauvet_chen 
};
