#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

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
   return 0;
}