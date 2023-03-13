#include "esp32-hal-gpio.h"
#include "timer.h"
#include "flag.h"

#define BP_PIN 23

struct Button_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int val_prec;                                             // etats interne du bouton
  int val_nouv;
}; 

void setup_button(Button_s * ctx,int timer,int period)
{
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->val_prec = 1;
  ctx->val_nouv = 1;

  //initialisation du bouton en mode pullup
  pinMode(BP_PIN, INPUT_PULLUP);
}

void loop_button(Button_s * ctx,mailbox_s * mb)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente
  ctx->val_nouv = digitalRead(BP_PIN);
  // Serial.println(BP_PIN);
  if((ctx->val_prec != ctx->val_nouv) && mb->state == EMPTY)
  {
    if(ctx->val_nouv == 0)//appuyé
      mb->val |= 0x1;
    else //relaché
      mb->val |= 0x10;
    
    mb->state == FULL;
  }
  ctx->val_prec = ctx->val_nouv;
}