//------------------------------------------------------------------------------
// Headers that are required for printf and mmap
//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//pour l'utilisation des threads
#include <pthread.h>

//Variable globale pour le bouton
char BP_ON = 0;   // mis à 1 si le bouton a été appuyé, mis à 0 quand la tâche qui attend l'appui a vu l'appui
char BP_OFF = 0;  // mis à 1 si le bouton a été relâché, mis à 0 quand la tâche qui attend le relâchement a vu le relâchement

//------------------------------------------------------------------------------
// GPIO ACCES
//------------------------------------------------------------------------------

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
};

struct gpio_s *gpio_regs_virt; 


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

    mmap_fd = open ( "/dev/mem", O_RDWR | O_SYNC );

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

//------------------------------------------------------------------------------
// Main Programm
//------------------------------------------------------------------------------

void
delay ( unsigned int milisec )
{
    struct timespec ts, dummy;
    ts.tv_sec  = ( time_t ) milisec / 1000;
    ts.tv_nsec = ( long ) ( milisec % 1000 ) * 1000000;
    nanosleep ( &ts, &dummy );
}

//Handlers pour les threads

    //clignottement
void * blink(void* arg)
{
    printf ( "-- info: start blinking.\n" );

    while (1) {
        gpio_write (*((int *)arg+2) , *((int *)arg+1) );
        delay ( *((int *)arg) );
        *((int *)arg+1) = 1 - *((int *)arg+1);
    }
}

    //clignottement si bouton pressé
void * blink_bp(void* arg)
{
    printf ( "-- info: telerupted blink start.\n" );
    char blink_on = 0;

    while (1) {
        gpio_write (*((int *)arg+2) , *((int *)arg+1) );
        delay ( *((int *)arg) );
        *((int *)arg+1) = blink_on - *((int *)arg+1);

        if(BP_OFF)
        {
            printf("-- info: bouton release");
            BP_OFF = 0;
            blink_on = 1 - blink_on;
        }
    }
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

int
main ( int argc, char **argv )
{

    // Get args
    // ---------------------------------------------

    int period, half_period;

    period = 1000; /* default = 1Hz */
    if ( argc > 1 ) {
        period = atoi ( argv[1] );
    }
    half_period = period / 2;
    uint32_t volatile * gpio_base = 0;

    // map GPIO registers
    // ---------------------------------------------

    if ( gpio_mmap ( (void **)&gpio_regs_virt ) < 0 ) {
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
    

    // Blink led at frequency of 1Hz
    // ---------------------------------------------

    uint32_t val = 0;
    
    //création des trois threads

    //les arguments de chacun des threads
    void * arg0 = malloc(sizeof(int)*3);
    void * arg1 = malloc(sizeof(int)*3);
    void * argbp = malloc(sizeof(int));
    //{half_period,begin_value,GPIO_LED}
    //le 1er

    *((int *)arg0) = half_period;
    *((int *)arg0+1) = val;
    *((int *)arg0+2) = GPIO_LED0;

    pthread_t blink0_thread;
    if(pthread_create (&blink0_thread, NULL, blink, arg0) != 0)
    {
        printf("error in thread 0");
        exit(1);
    }
    //le 2ème

    *((int *)arg1) = half_period;
    *((int *)arg1+1) = val+1;
    *((int *)arg1+2) = GPIO_LED1;

    pthread_t blink1_thread;
    if(pthread_create (&blink1_thread, NULL, blink_bp, arg1) != 0)
    {
        printf("error in thread 1");
        exit(1);
    }

    //le 3ème (bouton poussoire)

    *((int *)argbp) = GPIO_BP;

    pthread_t bp_thread;
    if(pthread_create (&bp_thread, NULL, bp_press, argbp) != 0)
    {
        printf("error in thread bp");
        exit(1);
    }

    //attendre la fin des threads

    pthread_join(blink0_thread,NULL);
    pthread_join(blink1_thread,NULL);
    pthread_join(bp_thread,NULL);

    return 0;
}
