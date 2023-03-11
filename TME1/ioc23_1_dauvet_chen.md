# TP1 DAUVET CHEN

## Hello World! RaspberryPi 

### 1er connexion au Raspberry

La carte qui à été attribué à notre groupe est la *23*, nous avon donc effectué notre première connexion à cette dernière avec la commande suivante :

` ssh -p 62223 pi@peri `

avec le mot de passe raspberry

ensuite nous avons créé notre répertoire personnel avec la commande 

` mkdir dauvet_chen `

### Configuration de la clé ssh

Dans les TP avec le raspberry, nous allons devoir nous connecter et envoyer des fichiers fréquemment, pour cela nous allons créé une clé ssh RSA pour nous connecter sanns fournir de mot de passe

```console
ssh-keygen -t rsa
ssh-copy-id -i $HOME/.ssh/id_rsa.pub -p 62223 pi@peri

```
la clé est donc configuré, plus besoin de rentrer le mot de passe pour faire les commandes `ssh` et `scp`.

### Compilation croisée

Nous avons ensuite configuré la compilation croisé, tout d'abord nous avons ajouté le toolchain créé par notre proffesseur a bashrc avec la commande

`echo source /users/enseig/franck/IOC/export_rpi_toolchain.sh >> $HOME/.bashrc`

puis vérifié que ca a bien marché avec la commande `which bcm2708hardfp-gcc`

Nous avons ensuite créé un Makefile qui compile avec le compilateur gcc pour le SoC BCM2708 puis envoi notre fichier compilé dans le dossier *dauvet_chen/* avec `scp`.

### Ecriture du premier programme

Nous avons alors créé un simple programme `helloworld` qui affiche simplement la chaine `Hello World !` dans la sortie standard avant de se terminer.

il a suffit de faire `make` et le programme a été compilé et envoyé à notre raspberry dans notre dossier personnel.

## Manipulation des GPIOs

### Contrôle de GPIO en sortie

Après avoir copié le repertoir *lab1* pour la suite du tp avec `cp -rp /users/enseig/franck/IOC/lab1 ./` et éditer le Makefile, nous avons compiler le fichier blink0 et éxécuté sur le raspberry en modifiant le 1er argument pour faire clignoter la LED à des fréquences différentes.

Nous nous sommes alors penchés sur le code pour mieux comprendre son fonctionnement :

1. Il peut être dangereux de se tromper de broche car la valeur des tensions pour chaque broches sont différentes(par exemple si jamais on configure une branche en entrée et qu'on fait circuler la mauvais tension il peut y avoir une surtension)
2. **BCM2835_GPIO_BASE** est l'adresse physique à partir du quel les GPIO sont mappé, pour s'en convaincre, on voit que dans la fonction **mmap**, **BCM2835_GPIO_BASE** est passé en argument de l'offset, càd que dans l'ensemble des adresses mappé dans /dev/mem, les adresses des GPIO commencent à **BCM2835_GPIO_BASE**
3. La **struct gpio_s** correspond a une structure de donné permettant de faire un mappage virtuel des adresses  des GPIO pour la manipulation de leurs valeurs
4. l'adresse **gpio_regs_virt** pointe sur l'adresse virtuelle (en mémoire du processus) créé par mmap à partir du fd */dev/mem*, il permet d'associé les adresses mappé à la structure **struct gpio_s**
5. **reg** correspond à l'adresse du registre du pin passé en argument
6. Les deux adresses sont gpset[reg] et gpclr[reg], lorsque on souhaite allumé la LED soit faire passé du courant, on écrit à l'adresse du registre de gpset[reg] sinon on coupe le courant en sélectionnant l'adresse du registre gpclr[reg]
7. les flags dans la fonction **open()** appelé dans la fonction **gpio_mmap()** permettent d'ouvrir un fd avec certain paramètre : 
    * O_RDWR : ouverture en lecture et écriture
    * O_SYNC : permet de mettre à jour le contenu du fichier lorsque qu'on écrit sur le fd
8. >void *mmap(void *addr, size_t length, int prot, int flags,int fd, off_t offset);
    * addr : l'adresse existante sur lequel on veut mettre le mappage, si NULL c'est l'OS qui décide
    * length : la longeur de la projection de mémoire
    * prot : défini les indicateurs de protection de la nouvelle section de mémoire alloué
    * flags : défini les indicateurs de visibilité par les autres processus de la nouvelle section de mémoire alloué
    * fd : le déscripteur de fichier à mapper en mémoire
    * offset : à partir d'où je dois mapper la mémoire
9.  La fonction delay prend en argument un temps d'attente, elle va créer deux structures de temps *timespec* qui correspond un intervalle de temps durant lequel le processus va être mis en sommeil
10. On peut accéder aux adresses du bus mémoire uniquement si on est super-administrateur.

### Allumage de LEDs à l'aide de thread

Nous allons maintenant utiliser la bibliothèque **pthread** pour attribuer la routine de faire clignoter la LED à des threads qui s'éxécutent indépendament de notre programme principale, grâce a ce mécanisme on pourra manipuler différent élement du bus simultanément.

D'abord on commence par modifier le **Makefile** en ajoutant une variable `LIBRARY=-lpthread`, qui correspond aux library utiliser par notre programme indiqué en paramètre de la compilation :  

```Makefile
%.x: %.c
	$(CROSS_COMPILE)gcc -o $@ $^ $(LDFLAGS) $(LIBRARY)
```
On ajoute evidemment a dep **blink0_pt.x** et **blink01_pt.x** pour que le Makefile les appelles.

Ensuite dans le code on va inclure `pthread.h`, on créé une fonction qui va être éxécuté par un thread :

```C
void * blink(void* arg)
{
    printf ( "-- info: start blinking.\n" );

    while (1) {
        gpio_write (*((int *)arg+2) , *((int *)arg+1) );
        delay ( *((int *)arg) );
        *((int *)arg+1) = 1 - *((int *)arg+1);
    }
}
```

cette fonction prend en argument une addresse sur une structure d'argument cette structure contient *{**half_period**, **begin_value**, **GPIO_LED**}*

où 

* **half_period** est la periode de clignotement de la LED
* **begin_value** si 0 la LED commence éteinte, si 1 commence allumé
* **GPIO_LED** adresse du GPIO à controlé

voici comment dans le thread principale on créé un nouveau thread pour faire clignoté une (ou plusieur) LED :

```C
// Setup GPIO of LED0 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED0, GPIO_FSEL_OUTPUT);

    // Setup GPIO of LED1 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED1, GPIO_FSEL_OUTPUT);
    

    // Blink led at frequency of 1Hz
    // ---------------------------------------------

    uint32_t val = 0;
    //création des deux threads

        //les arguments de chacun des threads {half_period, begin_value, GPIO_LED}
    void * arg0 = malloc(sizeof(int)*3);
    void * arg1 = malloc(sizeof(int)*3);

        //le 1er
    
    *((int *)arg0) = half_period;
    *((int *)arg0+1) = val;
    *((int *)arg0+2) = GPIO_LED0;

            //pthread_create créé un thread sur la fonctionblink avec les paramètres arg0
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
    if(pthread_create (&blink1_thread, NULL, blink, arg1) != 0)
    {
        printf("error in thread 1");
        exit(1);
    }

    //on attend la terminaison des deux threads avec pthread_join pour terminer le programme principale

    pthread_join(blink0_thread,NULL);
    pthread_join(blink1_thread,NULL);
```

### Contrôle de GPIO en entrée

On créé un fichier **read_bp.c** et on ajoute dans le Makefile **read_bp.x** à dep.
En plus de toutes les fonctions et structures déjà présente dans **blink.c**, on ajoute la fonction de lecture d'un pin :

```C
static uint32_t
gpio_read (uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;

    //retourne 262144 quand c'est pas pressé et 0 sinon donc :
    if ((gpio_regs_virt->gplev[reg] & (0x1 << bit)) == 0)
        return 1;
    else
        return 0;
}
```
si on initialise le GPIO en entrée et qu'on appelle cette fonction on a 0 si le bouton n'est pas pressé et 1 sinon.


Ensuite on effeectue une copie de **blink01_pt.c** dans **blink01_bp_pt.c**, le but est de faire fonctionner un thread qui détecte si le bouton à été appuyé, si c'est le cas une des deux LED s'arrête ou commence a clignoter (télérupteur)

le principe est simple : on échantillone la lecture de la valeur du bouton, si la valeur lu à changer entre deux lecture c'est que le bouton à été pressé :

```C
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

```

Le code du clignottement à été modifié pour détecté un appuie, les threads communiquent avec les variables globale BP_ON et BP_OFF :

```C
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
            BP_OFF = 0;
            blink_on = 1 - blink_on;
        }
    }
}
```

On créé un nouveau thread en passant 1 argument pour GPIO_PIN de la même façon que les deux threads.


### aller plus loin 

Dans le dossier **lab1+** ce trouve le programme **blink_exp.c** qui a une fonction éxpérimentale

simplement une des LEDs a 3 modes : clignotte en phase avec l'autre, clignotte en opposition de phase avec l'autre et ne clignotte pas.

La LED qui change en fonction de l'appui du bouton est simplement controlé par un switch, a chaque appui **blink_mode** change :

```C
blink_mode = (blink_mode +1)%3; 
            switch(blink_mode)
            {
                case 1:
                                blink_on = 1;
                                break;

                case 2:
                                blink_on = 1;
                                if(val = 1)
                                    val = 0;
                                else
                                    val = 1;
                                break;

                case 0: 
                                blink_on = 1 - blink_on;
                                break;
            }
```
