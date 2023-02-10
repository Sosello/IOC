# TP2 DAUVET CHEN

## Étape 1

### Code du module

Questions
1.  mon_module_init(void) est exécutée lorsqu'on insère dans le noyau
2.  exit mon_module_cleanup(void) est exécutée lorsqu'on enlève le module du noyau

### Compilation du module

### Travail à faire
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


expliqué module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "numéro du port du bouton"); module_param_array(leds, int, &nbled, 0);


dmesg

```console
[664159.516325] Hello World ! dauvet chen
[664159.516366] LED 0 = 420
[664159.516378] LED 1 = 69
[664159.516392] LED 2 = 666
```
### créaion d'un driver

insertion du module (l'os va enregistré avec n de major)

n de major dans /proc/device 
`cat /proc/devices` = `245 led`

créé node
(expliqué)
`sudo mknod /dev/led0_dauvet_chen c 245 0`
`sudo chmod a+rw /dev/led0_dauvet_chen`


ls -l /dev/ | grep led0_dauvet_chen
crw-rw-rw- 1 root root    245,   0 Jul 25 11:07 led0_dauvet_chen


echo "rien" > /dev/led0_XY && dmesg
[665959.247498] open()
[665959.247635] write()
[665959.247688] close()

dd bs=1 count=1 < /dev/led0_XY && dmesg
[665978.917570] open()
[665978.929617] read()
[665978.930456] close()

sudo rm -f /dev/led0_dauvet_chen

bash-4.2$ scp -P62223 insdev pi@peri:dauvet_chen/labdriver
insdev                                                                                                                                                                           100%  255    89.0KB/s   00:00    
bash-4.2$ scp -P62223 rmdev pi@peri:dauvet_chen/labdriver
rmdev                                                                                                                                                                            100%  106    35.3KB/s   00:00    
bash-4.2$ 












