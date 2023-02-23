# TP2 DAUVET CHEN

## Création et test d'un module noyau

### Code du module

Questions
1.  mon_module_init(void) est exécutée lorsqu'on insère le module dans le noyau
2.  mon_module_cleanup(void) est exécutée lorsqu'on enlève le module du noyau

### Compilation du module

Nous avons d'abord créer les fichier **test.c** et **module.c** et **Makefile** qui déjà existent dans le site.
Ensuite dans **Makefile** on change les variables **CARD_NUMB**, **LOGIN** et **LAB** par notre périphérique et répertoire correspondant.
Puis on compile et copie sur la raspberry pi par les commands **make** et **make upload**
On accède au répertoire de **lab2** pour tester les command:
```console
$ sudo insmod ./module.ko
$ lsmod
$ dmesg 
$ sudo rmmod module
$ lsmod
$ dmesg
```
 sudo insmod ./module.ko :insère dans raspberry pi le module.
 lsmod :affiche la liste de module
 ```
 Module                  Size  Used by
module                   797  0 
snd_bcm2835            21342  0 
snd_pcm                93100  1 snd_bcm2835
snd_seq                61097  0 
snd_seq_device          7209  1 snd_seq
snd_timer              23007  2 snd_pcm,snd_seq
snd                    67211  5 snd_bcm2835,snd_timer,snd_pcm,snd_seq,snd_seq_device
i2c_bcm2708             6200  0 
spi_bcm2708             6018  0 
uio_pdrv_genirq         3666  0 
uio                     9897  1 uio_pdrv_genirq

 ```
 dmesg : affiche et contrôle le tampon des messages du noyau
 On voit dans la console le printk du code de notre module.
 sudo rmmod module: supprime le module dans le raspberry.
 On peut vérifie que notre module est bien supprimé par les command **lsmod** et **dmesg**.
 
 ### Ajout des paramètres au module

On peut passé des parmètres au module, ca peut être utile par exemple pour indiqué le numéro de pin des LEDs ou de bouton poussoir.
Pour l'utiliser on appelle la fonction `module_param(var, type, permissions)`, on ajoute une description pour les paramètres avec `MODULE_PARM_DESC(var, string)`
On peut aussi demader une liste de paramètre avec la fonction `module_param_array(*vars,type,*nb_param,permissions)`, dans notre code on instancie plusieurs LEDs en utilisant ces fonctions puis on affiche la valeur de ces paramètres :

```console
sudo insmod ./module.ko LEDS=4,17,20
```

lorsqu'on affiche avec `dmesg`

```console
[664159.516325] Hello World ! dauvet chen
[664159.516366] LED 0 = 4
[664159.516378] LED 1 = 17
[664159.516392] LED 2 = 20
```
### Créaion d'un driver

Dans notre code du driver **led0_dauvet_chen** on va créé les fonctions associé au comportement d'ouverture, fermeture, écriture et lecture de celui-ci.
Ces fonctions sont assoscié grâce à la structure **file_operations**.
Pour indiquer à l'OS que notre module est un driver et enregistrer son numéro de major, on appelle la fonction `register_chrdev(maj, string, *struct_operations)` à l'initialisation, qui renverra le numéro de major de notre module et l'enregistrerat, et unregister_chrdev(major,string) pour décharger le module.
Le numéro de major est donc comme l'identifiant du driver.

Une fois le module chargé avec `insmod`, il faut récupérer le numéro de major qui lui à été assoscié, il se trouve dans **/proc/devives** :

```console
cat /proc/devices | grep led
245 led
```
Ensuite avec ce numéro de major, on va pouvoir crééer un nouvel **inode**, c'est celui-ci qui va nous permettre de manipuler le driver comme un fichier (en utilisant la structure file_operations)

```console
sudo mknod /dev/led0_dauvet_chen c 245 0
sudo chmod a+rw /dev/led0_dauvet_chen
```
pour savoir si notre driver est bien présent, on le recherche dans le répertoire **/dev/**

```console
ls -l /dev/ | grep led0_dauvet_chen
crw-rw-rw- 1 root root    245,   0 Jul 25 11:07 led0_dauvet_chen
```
on peut alors tester les fontionnalités de notre driver avec les commandes suivantes :

une écriture se fait en ouvrant l'inode, écrivant et fermant l'inode
```console
echo "rien" > /dev/led0_dauvet_chen && dmesg
[665959.247498] open()
[665959.247635] write()
[665959.247688] close()
```

une lecture se fait en ouvrant l'inode, lisant et fermant l'inode
```console
dd bs=1 count=1 < /dev/led0_dauvet_chen && dmesg
[665978.917570] open()
[665978.929617] read()
[665978.930456] close()
```
Pour supprimer un driver il faut retirer le module avec rmmod (cf Compilation du module) mais aussi retirer l'inode des **devices** :
`sudo rm -f /dev/led0_dauvet_chen`

Pour que ce soit moin fastidieux, on créé 2 scripts bash insdev et rmdev qui respectivement installe et enlève un driver de l'OS, le script récupère le numéro de major automatiquement en utilisant la commande `awk` qui permet de traité le texte de manière avancé :
`awk "\$2==\$module\" {print \$1;exit}" /proc/devices`
récupère la 1er colonne de la sortie de **/proc/devices** si la valeur de la deuxième colonne est  le nom de notre module, on souhaite le faire qu'une seul fois.

on transmet nos script :
```console
bash-4.2$ scp -P62223 insdev pi@peri:dauvet_chen/labdriver
insdev                                                                                                                                                                           100%  255    89.0KB/s   00:00    
bash-4.2$ scp -P62223 rmdev pi@peri:dauvet_chen/labdriver
rmdev                                                                                                                                                                            100%  106    35.3KB/s   00:00    
```
 
## Accès au GPIO depuis les fonctions du pilote
Á présent on a tous les outils en main pour créé un driver de notre bouton poussoir et LEDs, pour notre driver on a plus besoin de mapper les broches virtuellement, la fonction **__io_address** le fait pour nous, le script de chargement est le même que précédemment.

on écrit alors le comportement de 
* l'ouverture : qui fait gpofsel sur nos LEDs et BP.
* l'écriture : le buffer passé en argument décide du comportement de la fonction, si c'est 0 on écrit sur le pin GPIO_LED0 et 1 sur le pin GPIO_LED1, le deuxième caractère indique la valeur à écrire.
* la lecture : dans le buffer on écrit la valeur lue par le pin GPIO_BP
* la fermeture : rien en particulié

Après avoir charger le driver, on peut le tester comme suit :
```console
 echo "10" > /dev/ledbp_dauvet_chen
pi@raspberrypi ~/dauvet_chen/labdriver $ echo "00" > /dev/ledbp_dauvet_chen
pi@raspberrypi ~/dauvet_chen/labdriver $ echo "01" > /dev/ledbp_dauvet_chen
pi@raspberrypi ~/dauvet_chen/labdriver $ echo "11" > /dev/ledbp_dauvet_chen
pi@raspberrypi ~/dauvet_chen/labdriver $ echo "10" > /dev/ledbp_dauvet_chen
```
les deux LEDs vont s'allumer et s'éteindre.

Finalement on peut écrire un code utilisateur **test.c**, qui permet de réaliser des opérations plus complexe avec notre driver :

```C
int main()
{
   char led[2], bp[2];
   led[0] = '1';//led gpio pin 17
   led[1] = '1';
   int fdled0 = open("/dev/ledbp_dauvet_chen", O_WRONLY);
   int fdbp = open("/dev/ledbp_dauvet_chen", O_RDONLY);
   if ((fdled0 < 0)||(fdbp < 0)) {
      fprintf(stderr, "Erreur d'ouverture des pilotes LED ou Boutons\n, %d, %d",fdled0,fdbp);
      exit(1);
   }
   //allumé au début clignotte tant qu'on appuie pas sur le bouton
   write( fdled0, led, 2);
   sleep( 1);
   do { 
      led[1] = (led[1] == '0') ? '1' : '0';
      write( fdled0, led, 2);
      sleep( 1);
      read( fdbp, bp, 2);
   } while (bp[0] == '0');
   
   close(fdled0);
   close(fdbp);
   return 0;
}
```

Ce programme ouvre deux files descriptors sur notre driver, en lecture pour le bouton et en écriture pour les LEDs. Les LEDs vont clignotter tant qu'on appuie pas sur le bouton.


















