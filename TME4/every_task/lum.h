#include "timer.h"
#include "flag.h"

#define PHOTO_PIN 36
#define PHOTO_MAX 4095 //valeur max de la photo-résistance

//--------- définition de la tache Lum

struct Lum_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int val;                                             // etat interne de la photo resistance
}; 

void setup_lum(Lum_s * ctx,int timer, int period) {
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = 0;
}

void loop_lum(Lum_s * ctx,mailbox_s * mb_oled ,mailbox_s * mb_led,mailbox_s * mb_buzz) {
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  ctx->val = analogRead(PHOTO_PIN);

  if (mb_oled->state == EMPTY); // attend que la mailbox oled soit vide
  {
    mb_oled->val = map(ctx->val,0,PHOTO_MAX,0,100);
    mb_oled->state = FULL;
  }
  if (mb_led->state == EMPTY); // attend que la mailbox led soit vide
  {
    mb_led->val = map(ctx->val,0,PHOTO_MAX,1,500000); //inversement proportionnel à la luminosité
    mb_led->state = FULL;
  }
  if (mb_buzz->state == EMPTY); // attend que la mailbox led soit vide
  {
    mb_buzz->val = map(ctx->val,0,PHOTO_MAX,500000,500);
    if(ctx->val < 10)
      mb_buzz->val = 100000000;
    mb_buzz->state = FULL;
  }
  
}