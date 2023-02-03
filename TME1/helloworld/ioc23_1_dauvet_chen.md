# TP1 DAUVET CHEN

## Hello World! RaspberryPi 

### 1er connexion au Raspberry

La carte qui à été attribué à notre groupe est la *23*, nous avon donc effectué notre première connexion à cette dernière avec la commande suivante :

` ssh -p 62220 pi@peri `

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

## Manipukation des GPIOs

### Contrôle de GPIO en sortie

Après avoir copié le repertoir *lab1* pour la suite du tp avec `cp -rp /users/enseig/franck/IOC/lab1 ./` et éditer le Makefile, nous avons compiler le fichier blink0 et éxécuté sur le raspberry en modifiant le 1er argument pour faire clignoter la LED à des fréquences différentes.

Nous nous sommes alors penchés sur le code pour mieux comprendre son fonctionnement :

1. Il peut être dangereux de se tromper de broche car la valeur des tensions pour chaque broches sont différentes(par exemple si jamais on configure une branche en entrée et qu'on fait circuler la mauvais tension il peut y avoir une surtension)
2. *BCM2835_GPIO_BASE* est l'adresse physique à partir du quel les GPIO sont mappé, pour s'en convaincre, on voit que dans la fonction *mmap*, *BCM2835_GPIO_BASE* est passé en argument de l'offset, càd que dans l'ensemble des adresses mappé dans /dev/mem, les adresses des GPIO commencent à *BCM2835_GPIO_BASE*
3. La *struct gpio_s* correspond a une structure de donné permettant de faire un mappage virtuel des adresses  des GPIO pour la manipulation de leurs valeurs
4. l'adresse *gpio_regs_virt* pointe sur l'adresse virtuelle (en mémoire du processus) créé par mmap à partir du fd */dev/mem*, il permet d'associé les adresses mappé à la structure *struct gpio_s*
5. *reg* correspond à l'adresse du registre du pin passé en argument
6. Les deux adresses sont gpset[reg] et gpclr[reg], lorsque on souhaite allumé la LED soit faire passé du courant, on écrit à l'adresse du registre de gpset[reg] sinon on coupe le courant en sélectionnant l'adresse du registre gpclr[reg]
7. les flags dans la fonction *open()* appelé dans la fonction *gpio_mmap()* permettent d'ouvrir un fd avec certain paramètre : 
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

### suiv

modif du make, modif du code , explicztion thread et arg