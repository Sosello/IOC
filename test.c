#include <stdio.h>
int main()
{
   char led, bp;
   int fdled0 = open("/dev/led0_XY", O_WR);
   int fdbp = open("/dev/bp_XY", O_RD);
   if ((fdled0 < 0)||(fdbp < 0)) {
      fprintf(stderr, "Erreur d'ouverture des pilotes LED ou Boutons\n");
      exit(1);
   }
   do { 
      led = (led == '0') ? '1' : '0';
      write( fd, &led, 1);
      sleep( 1);
      read( fd, &bp, 1);
   } while (bp == '0');
   return 0;
}