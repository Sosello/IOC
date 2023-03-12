#include "timer.h"
#include "flag.h"

#define INTERRUPT_PIN 2

//--------- définition de la tache isr

struct Isr_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  byte state;                                             //etat de la broche
}; 

void SerialEvent()
{
  Serial.println("Interrupt led");
}

void setup_isr(Isr_s * ctx,int timer,int period)
{
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->state = LOW;

  pinMode(INTERRUPT_PIN, OUTPUT); //défini la direction du pin d'interruption
  //décommenter cette ligne pour l'interruption en fonctionnement
  // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), SerialEvent, CHANGE); //place un handler d'interruption lorsqu'un changement est détécté sur le pin d'interruption
}

void loop_isr(Isr_s * ctx, mailbox_s * mb)
{
  if (!waitFor(ctx->timer,ctx->period)) return;
  if (mb->state != EMPTY)return;//une fois que l'interruption sera levé on éxécutera plus le code de cette fonction
  if(Serial.available())//si il y a un caractère
    if(Serial.read() == 's')//que c'est un "s"
    {
      ctx->state = !ctx->state;
      digitalWrite(INTERRUPT_PIN,ctx->state);//on inverse l'état du pin
      mb->state = FULL;
    }
      
}