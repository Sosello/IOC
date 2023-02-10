#include <linux/fs.h>

char DRV_NAME[246] = "ledbp_dauvet_chen";

//Gestion GPIO
//-----------------------------------


//Variable globale pour le bouton
char BP_ON = 0;   // mis à 1 si le bouton a été appuyé, mis à 0 quand la tâche qui attend l'appui a vu l'appui
char BP_OFF = 0;  // mis à 1 si le bouton a été relâché, mis à 0 quand la tâche qui attend le relâchement a vu le relâchement


#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

#define GPIO_LED0   4
#define GPIO_LED1   17
#define GPIO_BP     18

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1


static const int LED0 = 4;

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
volatile *gpio_regs = (struct gpio_s *)__io_address(GPIO_BASE);


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

static uint32_t
gpio_read (uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;

    return (gpio_regs_virt->gplev[reg] & (0x1 << bit));
}

//------------------------------------------------------------------------------
// Access to memory-mapped I/O
//------------------------------------------------------------------------------

#define RPI_PAGE_SIZE           4096
#define RPI_BLOCK_SIZE          4096

static int mmap_fd;

static int
gpio_mmap ( void ** ptr )
{
    void * mmap_result;

    mmap_fd = open ( DRV_NAME, O_RDWR | O_SYNC );

    if ( mmap_fd < 0 ) {
        return -1;
    }

    mmap_result = mmap (
        NULL
      , RPI_BLOCK_SIZE
      , PROT_READ | PROT_WRITE
      , MAP_SHARED
      , mmap_fd
      , BCM2835_GPIO_BASE );

    if ( mmap_result == MAP_FAILED ) {
        close ( mmap_fd );
        return -1;
    }

    *ptr = mmap_result;

    return 0;
}

void
gpio_munmap ( void * ptr )
{
    munmap ( ptr, RPI_BLOCK_SIZE );
}

    //détection du bouton pressé
void * bp_press(void* arg)
{
    printf ( "-- info: bp start.\n" );

    uint32_t val_prec = 1;
    uint32_t val_nouv = 1;

    uint32_t bp_pin = *((uint32_t *)arg);

    while(1)
    {
        delay(20);
        val_nouv = gpio_read(bp_pin);

        if(val_prec != val_nouv)//changement d'état
        {
            if(val_nouv == 0)//appuyé
                BP_ON = 1;
            else //relaché
                BP_OFF = 1;
        }
        val_prec = val_nouv;
    }
}

//Primitive du driver
//-----------------------------------

static int 
open_led_dauvet_chen(struct inode *inode, struct file *file) {


    if ( gpio_mmap ( (void **)&gpio_regs ) < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    // Setup GPIO of LED0 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED0, GPIO_FSEL_OUTPUT);

    // Setup GPIO of LED1 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED1, GPIO_FSEL_OUTPUT);

    // Setup GPIO of BP to INPUT
    // ---------------------------------------------
    
    gpio_fsel(GPIO_BP, GPIO_FSEL_INPUT);
    
    //printk(KERN_DEBUG "open()\n");
    return 0;
}

static ssize_t 
read_led_dauvet_chen(struct file *file, char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "read()\n");
    return gpio_read(count);
}

static ssize_t 
write_led_dauvet_chen(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "write()\n");
    gpio_write(((int)*buf),count);
    return count;
}

static int 
release_led_dauvet_chen(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    return 0;
}

struct file_operations fops_led =
{
    .open       = open_led_dauvet_chen,
    .read       = read_led_dauvet_chen,
    .write      = write_led_dauvet_chen,
    .release    = release_led_dauvet_chen 
};